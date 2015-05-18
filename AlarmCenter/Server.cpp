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
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

#ifdef _DEBUG 
#define TIME_OUT		10			// in seconds
#else
#define TIME_OUT		60			// in seconds
#endif
#define THREAD_HANDLER_NO	4

namespace net {
namespace server {

IMPLEMENT_SINGLETON(CServer)
CServer::CServer() : m_bServerStarted(false)
{
	//m_pDataPacketMgr = NULL;
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
		, m_phThreadHandlers(NULL)
	{
		InitializeCriticalSection(&m_cs);
	}

	virtual ~CMyServerEventHandler() 
	{
		DeleteCriticalSection(&m_cs);
	}

	virtual void Start() {}

	virtual void Stop()	{}

	virtual DWORD OnRecv(CServerService *server, CClientData* client);

	virtual void OnConnectionEstablished(CServerService *server, CClientData *client)
	{
		UNREFERENCED_PARAMETER(server);
		//cout << "new connection from " << inet_ntoa(client->foreignAddIn.sin_addr) << ":" <<
		//	client->foreignAddIn.sin_port << endl;
		CLog::WriteLogA("new connection from %s:%d, conn_id %d\n",
						inet_ntoa(client->foreignAddIn.sin_addr),
						client->foreignAddIn.sin_port, client->conn_id);
		
	}

	virtual void OnConnectionLost(CServerService *server, CClientData *client)
	{
		UNREFERENCED_PARAMETER(server);
		//cout << "connection lost at " << inet_ntoa(client->foreignAddIn.sin_addr) << ":" <<
		//	client->foreignAddIn.sin_port << endl;
		CLog::WriteLogA("connection lost at %s:%d, conn_id %d\n",
						inet_ntoa(client->foreignAddIn.sin_addr),
						client->foreignAddIn.sin_port, client->conn_id);
		//wchar_t wacct[1024] = { 0 };
		//AnsiToUtf16Array(client->acct, wacct, sizeof(wacct));
		if (core::CAlarmMachineManager::GetInstance()->CheckIsValidMachine(client->ademco_id,
			client->acct, 0)) {
			core::CAlarmMachineManager::GetInstance()->MachineOnline(client->ademco_id, FALSE);
		}
	}
};

DWORD CMyServerEventHandler::OnRecv(CServerService *server, CClientData* client)
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
				strcpy_s(client->acct, packet._acct);
				char out[1024] = { 0 };
				_snprintf_s(out, 1024, "[#%04d| %04d %d %03d] %s %s\n",
							client->ademco_id, ademco_event, subzone,
							zone, ademco::GetAdemcoEventString(ademco_event),
							packet._timestamp._data);
				CLog::WriteLogA(out);

				//wchar_t wacct[1024] = { 0 };
				//AnsiToUtf16Array(client->acct, wacct, sizeof(wacct));

				if (!client->online) {
					if (mgr->CheckIsValidMachine(ademco_id, client->acct, zone)) {
						CLog::WriteLogA("CheckIsValidMachine succeeded aid %04d, acct %s",
										client->ademco_id, client->acct);
						client->online = true;
						server->ReferenceClient(client->ademco_id, client);
						mgr->MachineOnline(client->ademco_id, TRUE, 
										   inet_ntoa(client->foreignAddIn.sin_addr), 
										   client, client->OnConnHangup);
						mgr->MachineEventHandler(ademco_id, ademco_event, zone, 
												 subzone, packet._timestamp._time,
												 packet._xdata, packet._xdata_len);
					} else {
						CString fm, rec;
						fm.LoadStringW(IDS_STRING_FM_KICKOUT_INVALID);
						rec.Format(fm, client->ademco_id, A2W(client->acct));
						hr->InsertRecord(client->ademco_id, zone, rec, packet._timestamp._time, core::RECORD_LEVEL_ONOFFLINE);
						CLog::WriteLog(rec);
						CLog::WriteLog(_T("Check acct-aid failed, pass.\n"));
						server->Release(client);
						goto EXIT_ON_RECV;
					}
				} else {
					mgr->MachineEventHandler(ademco_id, ademco_event, zone, 
											 subzone, packet._timestamp._time,
											 packet._xdata, packet._xdata_len);
				}
			} else {
				bFaild = TRUE;
			}
		} else if (strcmp(packet._id, AID_NAK) == 0) {
			CString record = _T("");
			record.LoadStringW(IDS_STRING_ILLEGAL_OP);
			hr->InsertRecord(client->ademco_id, 0, record, packet._timestamp._time, core::RECORD_LEVEL_ONOFFLINE);
		} 
		//else if (strcmp(packet._id, AID_MODULE_REG) == 0) {
		//	bFaild = TRUE;
		//	do {
		//		if (strlen(packet._xdata) != 32)
		//			break;
		//		char device_id[64] = { 0 };
		//		ZeroMemory(device_id, 64);
		//		strncpy_s(device_id, 64, packet._xdata, 32);
		//		if (strlen(device_id) != 32) {
		//			break;
		//		}
		//
		//		if (mgr->CheckIfMachineAcctAlreadyInuse(device_id)) {
		//			CString record = _T("");
		//			record.LoadStringW(IDS_STRING_ACCT_NOT_UNIQUE);
		//			CLog::WriteLog(record);
		//			hr->InsertRecord(-1, -1, record, packet._timestamp._time, core::RECORD_LEVEL_ONOFFLINE);
		//			break;
		//		}
		//
		//		int ademco_id = -1;
		//		if (!mgr->DistributeAdemcoID(ademco_id)) {
		//			//MyErrorMsgBox(GlobalGetSafeHWnd(), IDS_STRING_NO_MORE_MACHINE);
		//			CString record = _T("");
		//			record.LoadStringW(IDS_STRING_NO_MORE_MACHINE);
		//			CLog::WriteLog(record);
		//			hr->InsertRecord(-1, -1, record, packet._timestamp._time, core::RECORD_LEVEL_ONOFFLINE);
		//			goto EXIT_ON_RECV;
		//		}
		//
		//
		//		if (!mgr->AddMachine(ademco_id, device_id, NULL))
		//			break;
		//
		//		char buff[1024] = { 0 };
		//		char xdata[5] = { 0 };
		//		sprintf_s(xdata, "%04d", ademco_id);
		//		int bytes = packet.Make(buff, sizeof(buff), AID_REG_RSP, 0, 
		//								ACCOUNT, ademco_id, 0, 0, 0, xdata);
		//		if (server->SendToClient(client, buff, bytes)) {
		//			bFaild = FALSE;
		//			CString record = _T(""), temp = _T("");
		//			temp.LoadStringW(IDS_STRING_NEW_MACHINE);
		//			record.Format(_T("%s, ademco_id %d, device_id %s"),
		//						  temp,
		//						  ademco_id,
		//						  A2W(device_id));
		//			CLog::WriteLog(record);
		//			hr->InsertRecord(-1, -1, record, packet._timestamp._time, core::RECORD_LEVEL_ONOFFLINE);
		//		}
		//	} while (0);
		//} 
		/*else if (strcmp(packet._id, AID_PWW) == 0) {
			if (client->online) {
				CLog::WriteLog(L"ÃÜÂë´íÎó£¬ÔÙ´ÎÊäÈë ademco_id %d");
				mgr->DisarmPasswdWrong(client->ademco_id);
			}
			bNeed2ReplyAck = FALSE;
		} */
		else if (strcmp(packet._id, AID_ACK) == 0) {
			CLog::WriteLog(L"remote: ACK");
			bNeed2ReplyAck = FALSE;
		} else {
			bFaild = TRUE;
		}

		char buff[BUFF_SIZE] = { 0 };
		const char* acct = NULL;
		int acct_len = 0;
		if (strlen(packet._acct) > 0) {
			acct = packet._acct;
			acct_len = strlen(packet._acct);
		} else if (strlen(client->acct) > 0) {
			acct = client->acct;
			acct_len = strlen(client->acct);
		} else {
			acct = "0000";
			acct_len = 4;
		}

		int seq = ademco::NumStr2Dec(packet._seq, 4);
		if (seq > 9999)
			seq = 0;

		if (bFaild) {
			client->buff.Clear();
			DWORD dwSize = packet.Make(buff, BUFF_SIZE, AID_NAK, seq,
									   acct, 0, 0, 0, 0, NULL, 0);
			server->SendToClient(client, buff, dwSize);
		} else {
			client->buff.rpos = (client->buff.rpos + dwBytesCommited);
			if (bNeed2ReplyAck) {
				
				DWORD dwSize = packet.Make(buff, BUFF_SIZE, AID_ACK, seq, acct,
										   client->ademco_id, 0, 0, 0, NULL, 0);
				server->SendToClient(client, buff, dwSize);
			}
		}
	}

EXIT_ON_RECV:
	return result;
}

CMyServerEventHandler *g_event_handler = NULL;
CServerService *g_select_server = NULL;


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
	LOG(L"ademco_id %04d, ademco_event %04d, gg %02d, zone %03d, xdata %p, len %d\n",
		ademco_id, ademco_event, gg, zone, xdata, xdata_len);
	if(!m_bServerStarted)
		return FALSE;
	if (g_select_server) {
		CClientData *client = NULL;
		if (g_select_server->FindClient(ademco_id, &client) && client) {
			char data[BUFF_SIZE] = { 0 };
			const char* acct = client->acct;
			AdemcoPacket packet;
			DWORD dwSize = packet.Make(data, BUFF_SIZE, AID_HB, 0, acct, 
									   ademco_id, ademco_event, gg, zone, 
									   xdata, xdata_len);
			LOG(L"find client success\n");
			return g_select_server->SendToClient(client->conn_id, data, dwSize);
		}
	}
	LOG(L"find client failed\n");
	return FALSE;
}





NAMESPACE_END
NAMESPACE_END
