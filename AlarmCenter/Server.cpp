// Server.cpp: implementation of the CServer class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "Server.h"
#include "ServerService.h"
#include "resource.h"
#include <list>

#include "ademco_func.h"
using namespace ademco;

#include <time.h>
#include "AlarmMachineManager.h"
#include "HistoryRecord.h"

#ifdef _DEBUG 
#define TIME_OUT		120			// in seconds
#else
#define TIME_OUT		60			// in seconds
#endif
#define THREAD_HANDLER_NO	4

namespace net {
namespace server {

IMPLEMENT_SINGLETON(CServer)
CServer::CServer() : m_bServerStarted(false)
{
	//m_pDataPacketMgr = nullptr;
}

class CMyServerEventHandler : public CServerEventHandler
{
	
private:
	volatile unsigned int m_nSignaledEventCount;
	CRITICAL_SECTION m_cs;
	HANDLE m_hEventShutdown;
	HANDLE *m_phThreadHandlers;
public:
	CMyServerEventHandler() 
		: m_nSignaledEventCount(0) 
		, m_hEventShutdown(INVALID_HANDLE_VALUE)
		, m_phThreadHandlers(nullptr)
	{
		InitializeCriticalSection(&m_cs);
	}

	virtual ~CMyServerEventHandler() 
	{
		DeleteCriticalSection(&m_cs);
	}

	virtual void Start() {}

	virtual void Stop()	{}

	virtual DWORD OnRecv(CServerService *server, CClientData* client, BOOL& resolved);

	virtual void OnConnectionEstablished(CServerService *server, CClientData *client)
	{
		UNREFERENCED_PARAMETER(server);
		CLog::WriteLogA("new connection from %s:%d\n",
						inet_ntoa(client->foreignAddIn.sin_addr),
						client->foreignAddIn.sin_port);
		
	}

	virtual void OnConnectionLost(CServerService *server, CClientData *client)
	{
		UNREFERENCED_PARAMETER(server);
		CLog::WriteLogA("connection lost at %s:%d, ademco_id %d\n",
						inet_ntoa(client->foreignAddIn.sin_addr),
						client->foreignAddIn.sin_port, client->ademco_id);
		if (core::CAlarmMachineManager::GetInstance()->CheckIsValidMachine(client->ademco_id, 0)) {
			core::CAlarmMachineManager::GetInstance()->MachineOnline(ES_TCP_CLIENT, client->ademco_id, FALSE);
		}
	}
};


DWORD CMyServerEventHandler::OnRecv(CServerService *server, CClientData* client, BOOL& resolved)
{
	USES_CONVERSION;
	core::CHistoryRecord *hr = core::CHistoryRecord::GetInstance(); ASSERT(hr);
	core::CAlarmMachineManager* mgr = core::CAlarmMachineManager::GetInstance(); ASSERT(mgr);
	size_t dwBytesCommited = 0;
	AdemcoPacket packet;
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
		if (strcmp(packet._id, AID_NULL) == 0) {
			// reply ACK
			char out[1024] = { 0 };
			_snprintf_s(out, 1024, "#%04d nullptr %s\n",
						client->ademco_id, packet._timestamp._data);
			CLog::WriteLogA(out);
		} else if (strcmp(packet._id, AID_HB) == 0) {
			if (packet._data._len > 2) {
				int ademco_id = packet._data._ademco_id;
				int ademco_event = packet._data._ademco_event;
				int zone = packet._data._zone;
				int subzone = packet._data._gg;
				/*if (packet._xdata) {
					subzone = atoi(packet._xdata);
				}*/
				client->ademco_id = ademco_id;
				//strcpy_s(client->acct, packet._acct);
				char out[1024] = { 0 };
				_snprintf_s(out, 1024, "[#%04d| %04d %d %03d] %s %s\n",
							client->ademco_id, ademco_event, subzone,
							zone, W2A(ademco::GetAdemcoEventStringChinese(ademco_event).c_str()),
							packet._timestamp._data);
				CLog::WriteLogA(out);

				//wchar_t wacct[1024] = { 0 };
				//AnsiToUtf16Array(client->acct, wacct, sizeof(wacct));

				if (!client->online) {
					if (mgr->CheckIsValidMachine(ademco_id, /*client->acct, */zone)) {
						CLog::WriteLogA("CheckIsValidMachine succeeded aid %04d",
										client->ademco_id/*, client->acct*/);
						client->online = true;
						resolved = true;
						BOOL bTheSameIpPortClientReconnect = FALSE;
						server->ResolveOutstandingClient(client, bTheSameIpPortClientReconnect);
						if (!bTheSameIpPortClientReconnect) {
							mgr->MachineOnline(ES_TCP_CLIENT, client->ademco_id, TRUE,
											   inet_ntoa(client->foreignAddIn.sin_addr),
											   client, client->OnConnHangup);
						}
						mgr->MachineEventHandler(ES_TCP_CLIENT, ademco_id, ademco_event, zone,
												 subzone, packet._timestamp._time, time(nullptr), 
												 packet._xdata, packet._xdata_len);
					} else {
						CString fm, rec;
						fm.LoadStringW(IDS_STRING_FM_KICKOUT_INVALID);
						rec.Format(fm, client->ademco_id/*, A2W(client->acct)*/);
						hr->InsertRecord(client->ademco_id, zone, rec, time(nullptr), core::RECORD_LEVEL_ONOFFLINE);
						CLog::WriteLog(rec);
						CLog::WriteLog(_T("Check acct-aid failed, pass.\n"));
						server->RecycleOutstandingClient(client);
						resolved = true;
						goto EXIT_ON_RECV;
					}
				} else {
					mgr->MachineEventHandler(ES_TCP_CLIENT, ademco_id, ademco_event, zone,
											 subzone, packet._timestamp._time, time(nullptr), 
											 packet._xdata, packet._xdata_len);
				}
			} else {
				bFaild = TRUE;
			}
		} else if (strcmp(packet._id, AID_NAK) == 0) {
			CString record = _T("");
			record.LoadStringW(IDS_STRING_ILLEGAL_OP);
			hr->InsertRecord(client->ademco_id, 0, record, packet._timestamp._time, core::RECORD_LEVEL_ONOFFLINE);
		} else if (strcmp(packet._id, AID_ACK) == 0) {
			CLog::WriteLog(L"remote: ACK. seq %d, ademco_id %04d\n", 
						   ademco::NumStr2Dec(packet._seq, 4), 
						   packet._data._ademco_id);
			bNeed2ReplyAck = FALSE;
			const Task* task = client->GetFirstTask();
			if (task && task->_seq == ademco::NumStr2Dec(packet._seq, 4)) {
				client->RemoveFirstTask();
			}
		} else {
			bFaild = TRUE;
		}

		char buff[BUFF_SIZE] = { 0 };
		int seq = ademco::NumStr2Dec(packet._seq, 4);
		if (seq > 9999)
			seq = 1;

		if (bFaild) {
			client->buff.Clear();
			seq = 1;
			DWORD dwSize = packet.Make(buff, BUFF_SIZE, AID_NAK, seq,
									   /*acct, */nullptr, client->ademco_id, 0, 0, 0, nullptr, 0);
			server->SendToClient(client, buff, dwSize);
		} else {
			client->buff.rpos = (client->buff.rpos + dwBytesCommited);
			if (bNeed2ReplyAck) {
				DWORD dwSize = packet.Make(buff, BUFF_SIZE, AID_ACK, seq, /*acct,*/nullptr,
										   client->ademco_id, 0, 0, 0, nullptr, 0);
				server->SendToClient(client, buff, dwSize);
			}
		}
	}

EXIT_ON_RECV:
	return result;
}

CMyServerEventHandler *g_event_handler = nullptr;
CServerService *g_select_server = nullptr;


BOOL CServer::Start(WORD& port)
{
	if (m_bServerStarted)
		return TRUE;
	
	try {
		g_event_handler = new CMyServerEventHandler();
		g_select_server = new CServerService(port, core::MAX_MACHINE, TIME_OUT, true, false);
		g_select_server->SetEventHander(g_event_handler);
		g_select_server->Start();
	}
	catch (const char *err) {
		CLog::WriteLogA(err);
	}
	catch (const wchar_t *err) {
		CLog::WriteLog(err);
	}
	
	m_bServerStarted = true;
	
	return TRUE;
}

void CServer::Stop()
{
	AUTO_LOG_FUNCTION;
	if (g_select_server)
		g_select_server->Stop();
	SAFEDELETEP(g_select_server);
	SAFEDELETEP(g_event_handler);	
	m_bServerStarted = false;
}

//void CServer::Release()
//{
//	m_Lock4GetInstance.Lock();
//	SAFEDELETEP(m_pInst);
//	m_Lock4GetInstance.UnLock();
//}

BOOL CServer::SendToClient(int ademco_id, int ademco_event, int gg, 
						   int zone, const char* xdata, int xdata_len)
{
	AUTO_LOG_FUNCTION;
	JLOG(L"ademco_id %04d, ademco_event %04d, gg %02d, zone %03d, xdata %p, len %d\n",
		ademco_id, ademco_event, gg, zone, xdata, xdata_len);
	if(!m_bServerStarted)
		return FALSE;
	if (g_select_server) {
		/*CClientData *client = nullptr;
		if (g_select_server->FindClient(ademco_id, &client) && client) {
			JLOG(L"find client success\n");
			client->AddTask(new Task(ademco_id, ademco_event, gg, zone, xdata, xdata_len));
			return TRUE;
		}*/
		return g_select_server->SendToClient(ademco_id, ademco_event, gg, zone, xdata, xdata_len);
	}
	JLOG(L"find client failed\n");
	return FALSE;
}





NAMESPACE_END
NAMESPACE_END
