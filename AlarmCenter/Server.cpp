// Server.cpp: implementation of the CServer class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "Server.h"
#include "ServerService.h"
#include <list>
#include <ctime>

#include "ademco_func.h"
using namespace ademco;

#include <time.h>
#include "AlarmMachineManager.h"
#include "HistoryRecord.h"
#include "AlarmCenter.h"
#include "congwin_fe100_mgr.h"

#ifdef _DEBUG 
#define TIME_OUT		120			// in seconds
#else
#define TIME_OUT		120			// in seconds
#endif
#define THREAD_HANDLER_NO	4

namespace net {
namespace server {

CServer::CServer() : m_bServerStarted(false)
{
}

class CMyServerEventHandler : public CServerEventHandler
{
	
private:
	volatile unsigned int m_nSignaledEventCount;
	HANDLE m_hEventShutdown;
	HANDLE *m_phThreadHandlers;

	std::map<int, time_t> invalid_client_conn_;

	void HandleInvalidClient(int ademco_id) {
		auto now = time(nullptr);
		auto iter = invalid_client_conn_.find(ademco_id);
		if (iter == invalid_client_conn_.end()) {
			invalid_client_conn_[ademco_id] = now;
		} else {
			double diff = std::difftime(now, iter->second);
			if (diff < 6)
				return;
			else
				invalid_client_conn_[ademco_id] = now;
		}

		CString fm, rec;
		fm = TR(IDS_STRING_FM_KICKOUT_INVALID);
		rec.Format(fm, ademco_id);
		core::history_record_manager::get_instance()->InsertRecord(ademco_id, 0, rec, now, core::RECORD_LEVEL_STATUS);
		JLOG(rec);
		JLOG(_T("Check acct-aid failed, pass.\n"));
	}

public:
	CMyServerEventHandler() 
		: m_nSignaledEventCount(0) 
		, m_hEventShutdown(INVALID_HANDLE_VALUE)
		, m_phThreadHandlers(nullptr)
	{
	}

	virtual ~CMyServerEventHandler() 
	{
	}

	virtual void Start() {}

	virtual void Stop()	{}

	virtual DWORD OnRecv(CServerService *server, const net::server::CClientDataPtr& client, BOOL& resolved);

	virtual void OnConnectionEstablished(CServerService *server, const net::server::CClientDataPtr& client)
	{
		UNREFERENCED_PARAMETER(server);
		JLOGA("new connection from %s:%d\n",
						inet_ntoa(client->foreignAddIn.sin_addr),
						client->foreignAddIn.sin_port);
		
	}

	virtual void OnConnectionLost(CServerService *server, const net::server::CClientDataPtr& client)
	{
		UNREFERENCED_PARAMETER(server);
		JLOGA("connection lost at %s:%d, ademco_id %d\n",
						inet_ntoa(client->foreignAddIn.sin_addr),
						client->foreignAddIn.sin_port, client->ademco_id);
		if (core::alarm_machine_manager::get_instance()->CheckIsValidMachine(client->ademco_id, 0)) {
			core::alarm_machine_manager::get_instance()->MachineOnline(ES_TCP_CLIENT, client->ademco_id, FALSE);
		}
	}
};


DWORD CMyServerEventHandler::OnRecv(CServerService *server, const net::server::CClientDataPtr& client, BOOL& resolved)
{
	auto mgr = core::alarm_machine_manager::get_instance(); ASSERT(mgr);
	size_t dwBytesCommited = 0;
	static AdemcoPacket packet;
	ParseResult result = packet.Parse(client->buff.buff + client->buff.rpos,
								   client->buff.wpos - client->buff.rpos,
								   dwBytesCommited);
	BOOL bFaild = FALSE;
	BOOL bNeed2ReplyAck = TRUE;
	if (RESULT_DATA_ERROR == result) {
		result = RESULT_OK;
		ASSERT(0);
		client->buff.Clear();
		bFaild = TRUE;
	} else if (RESULT_NOT_ENOUGH == result) {
	} else {
		int seq = ademco::NumStr2Dec(&packet._seq[0], packet._seq.size());
		if (seq > 9999)
			seq = 1;

		ademco::ademco_protocal protocal = ademco::heng_bo;
		auto t = time(nullptr);

		if (ademco::is_same_id(packet._id, AID_NULL)) {
			// reply ACK
			char out[1024] = { 0 };
			char ts[64] = { 0 };
			memcpy(ts, packet._timestamp._data, packet._timestamp._len);
			_snprintf_s(out, 1024, "#%06d NULL %s\n",
						client->ademco_id, ts);
			JLOGA(out);
			seq = 0;
			mgr->MachineEventHandler(ES_TCP_CLIENT, client->ademco_id, EVENT_LINK_TEST, 0, 0, t, t);
		} else if (ademco::is_valid_ademco_protocal(packet._id, protocal)) {
			
			JLOGA("remote protocal %s. seq %d, ademco_id %04d\n",packet._id, seq, packet._ademco_data._ademco_id);
			client->protocal_ = protocal;

			TaskPtr task = client->GetFirstTask();
			if (task && task->_seq == seq) {
				client->RemoveFirstTask();
			}

			if (packet._ademco_data._len > 2) {
				int ademco_id = packet._ademco_data._ademco_id;
				auto ademco_event = packet._ademco_data._ademco_event;
				int zone = packet._ademco_data._zone;
				int subzone = packet._ademco_data._gg;
				if (client->ademco_id != ademco_id) {
					mgr->MachineOnline(ES_TCP_CLIENT, client->ademco_id, FALSE);
					client->ademco_id = ademco_id;
				}

				if (!client->online) {
					if (mgr->CheckIsValidMachine(ademco_id, zone)) {
						JLOGA("CheckIsValidMachine succeeded aid %04d", client->ademco_id);
						client->online = true;
						resolved = true;
						BOOL bTheSameIpPortClientReconnect = FALSE;
						server->ResolveOutstandingClient(client, bTheSameIpPortClientReconnect);
						if (!bTheSameIpPortClientReconnect) {
							mgr->MachineOnline(ES_TCP_CLIENT, client->ademco_id, TRUE,
											   inet_ntoa(client->foreignAddIn.sin_addr),
											   client, client->OnConnHangup);
						}
						mgr->MachineEventHandler(ES_TCP_CLIENT, ademco_id, ademco_event, zone, subzone, t, t, packet._xdata);
					} else {
						HandleInvalidClient(ademco_id);
						server->RecycleOutstandingClient(client);
						resolved = true;
						goto EXIT_ON_RECV;
					}
				} else {
					mgr->MachineEventHandler(ES_TCP_CLIENT, ademco_id, ademco_event, zone, subzone, t, t, packet._xdata);
				}
			} else {
				bFaild = TRUE;
			}
		} else if (ademco::is_same_id(packet._id, AID_DUH)) {
			CString record = _T("");
			record = TR(IDS_STRING_ILLEGAL_OP);
			JLOG(record);
#ifdef _DEBUG
			core::history_record_manager::get_instance()->InsertRecord(client->ademco_id, 0, record, t, core::RECORD_LEVEL_STATUS);
#endif
		} else if (ademco::is_same_id(packet._id, AID_ACK)) {
			JLOG(L"remote: ACK. seq %d, ademco_id %04d\n", seq, packet._ademco_data._ademco_id);
			bNeed2ReplyAck = FALSE;
			TaskPtr task = client->GetFirstTask();
			if (task && task->send_once_ && task->_seq == seq) {
				client->RemoveFirstTask();
			}
		} else {
			bFaild = TRUE;
		}

		char buff[BUFF_SIZE] = { 0 };

		if (bFaild) {
			client->buff.Clear();
			auto dwSize = packet.Make(buff, BUFF_SIZE, AID_DUH, seq, nullptr, client->ademco_id, ademco::EVENT_INVALID_EVENT, 0, 0);
			server->RealSendToClient(client, buff, dwSize);
		} else {
			client->buff.rpos = (client->buff.rpos + dwBytesCommited);
			if (bNeed2ReplyAck) {
				auto dwSize = packet.Make(buff, BUFF_SIZE, AID_ACK, seq, nullptr, client->ademco_id, ademco::EVENT_INVALID_EVENT, 0, 0);
				server->RealSendToClient(client, buff, dwSize);
			}
		}
	}

EXIT_ON_RECV:
	return result;
}

std::shared_ptr<CMyServerEventHandler> g_event_handler = nullptr;
std::shared_ptr<CServerService> g_select_server = nullptr;


BOOL CServer::Start(unsigned int& port)
{
	if (m_bServerStarted)
		return TRUE;
	
	try {
		g_event_handler = std::make_shared<CMyServerEventHandler>();
		g_select_server = std::make_shared<CServerService>(port, core::MAX_MACHINE, TIME_OUT, true, false);
		g_select_server->SetEventHander(g_event_handler);
		g_select_server->Start();
	}
	catch (const char *err) {
		JLOGA(err);
	}
	catch (const wchar_t *err) {
		JLOG(err);
	}
	
	m_bServerStarted = true;
	
	return TRUE;
}

void CServer::Stop()
{
	AUTO_LOG_FUNCTION;
	if (g_select_server)
		g_select_server->Stop();
	g_select_server = nullptr;
	g_event_handler = nullptr;
	m_bServerStarted = false;
}


BOOL CServer::SendToClient(int ademco_id, ademco::ADEMCO_EVENT ademco_event, int gg,
						   int zone, const ademco::char_array_ptr& xdata)
{
	AUTO_LOG_FUNCTION;
	JLOG(L"ademco_id %04d, ademco_event %04d, gg %02d, zone %03d\n",
		ademco_id, ademco_event, gg, zone);
	if(!m_bServerStarted)
		return FALSE;
	if (g_select_server) {
		return g_select_server->SendToClient(ademco_id, ademco_event, gg, zone, xdata);
	}
	JLOG(L"find client failed\n");
	return FALSE;
}





};};
