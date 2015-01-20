// Server.cpp: implementation of the CServer class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "Server.h"

#include "ServerService.h"
#define MAX_CONNECTIONS 10000
#ifdef _DEBUG 
#define TIME_OUT		120			// in seconds
#else
#define TIME_OUT		10			// in seconds
#endif
#define THREAD_HANDLER_NO	4
#include <list>

#include "../ademco_func.h"
using namespace AdemcoFunc;
//#include "AdemcoAndPrivateProtocalTcpDataPacket.h"
#include <time.h>
#include "../AlarmMachineManager.h"
#include "../HistoryRecord.h"
//#include "../DBOper.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif


namespace net {
CServer *CServer::m_pInst;
CLock CServer::m_Lock4GetInstance;

typedef struct _EVENT_DATA
{
	char acct[64];
	int ademco_id;
	int event;
	int zone;
	unsigned int conn_id;
	_EVENT_DATA() : ademco_id(0), event(0), zone(0), conn_id(0)
	{
		memset(acct, 0, sizeof(acct));
	}
}EVENT_DATA;


class CMyEventHandler : public CSockEventHandler
{
	
private:
	volatile unsigned int m_nSignaledEventCount;
	EVENT_DATA m_event_data[MAX_CONNECTIONS];
	CRITICAL_SECTION m_cs;
	HANDLE m_hEventShutdown;
	HANDLE *m_phThreadHandlers;
	std::list<unsigned int> m_idle_event_ids;
	std::list<unsigned int> m_signal_event_ids;
public:
	inline void AddEvent(char acct[64], int ademco_id, int event, int zone, unsigned int conn_id)
	{
		CLocalLock lock(&m_cs);
		if (!m_idle_event_ids.empty()) {
			unsigned int idle_event_id = m_idle_event_ids.front();
			m_idle_event_ids.pop_front();
			m_signal_event_ids.push_back(idle_event_id);

			InterlockedIncrement(&m_nSignaledEventCount);
			strcpy_s(m_event_data[idle_event_id].acct, acct);
			m_event_data[idle_event_id].ademco_id = ademco_id;
			m_event_data[idle_event_id].event = event;
			m_event_data[idle_event_id].zone = zone;
			m_event_data[idle_event_id].conn_id = conn_id;
		}
	}

	inline bool GetEvent(char acct[64], int &ademco_id, int &event, int &zone, unsigned int &conn_id)
	{
		CLocalLock lock(&m_cs);
		if (!m_signal_event_ids.empty()) {
			unsigned int idle_event_id = m_signal_event_ids.front();
			m_signal_event_ids.pop_front();
			m_idle_event_ids.push_back(idle_event_id);

			InterlockedDecrement(&m_nSignaledEventCount);
			strcpy_s(acct, 64, m_event_data[idle_event_id].acct);
			ademco_id = m_event_data[idle_event_id].ademco_id;
			event = m_event_data[idle_event_id].event;
			zone = m_event_data[idle_event_id].zone;
			conn_id = m_event_data[idle_event_id].conn_id;
			return true;
		}
		return false;
	}

	CMyEventHandler() 
		: m_nSignaledEventCount(0) 
		, m_hEventShutdown(INVALID_HANDLE_VALUE)
		, m_phThreadHandlers(NULL)
	{
		InitializeCriticalSection(&m_cs);
		
	}

	virtual ~CMyEventHandler() {
		DeleteCriticalSection(&m_cs);
	}

	virtual void Start()
	{
	}

	virtual void Stop()
	{
	}

	virtual DWORD OnRecv(CServerService *server, CLIENT* client);

	virtual void OnConnectionEstablished(CServerService *server, CLIENT *client)
	{
		UNREFERENCED_PARAMETER(server);
		//cout << "new connection from " << inet_ntoa(client->foreignAddIn.sin_addr) << ":" <<
		//	client->foreignAddIn.sin_port << endl;
		CLog::WriteLogA("new connection from %s:%d\n",
						inet_ntoa(client->foreignAddIn.sin_addr),
						client->foreignAddIn.sin_port);
	}

	virtual void OnConnectionLost(CServerService *server, CLIENT *client)
	{
		UNREFERENCED_PARAMETER(server);
		//cout << "connection lost at " << inet_ntoa(client->foreignAddIn.sin_addr) << ":" <<
		//	client->foreignAddIn.sin_port << endl;
		CLog::WriteLogA("connection lost at %s:%d\n",
						inet_ntoa(client->foreignAddIn.sin_addr),
						client->foreignAddIn.sin_port);
		wchar_t wacct[1024] = { 0 };
		AnsiToUtf16Array(client->acct, wacct, sizeof(wacct));
		if (CNetHostInfo::GetInstance()->CheckMachine(client->ademco_id,
			wacct, 0)) {
			CNetHostInfo::GetInstance()->MachineOnline(client->ademco_id, FALSE);
		}
	}
protected:
	static DWORD WINAPI ThreadHandler(LPVOID lp);
};

DWORD CMyEventHandler::OnRecv(CServerService *server, CLIENT* client)
{
	CHistoryRecord *hr = CHistoryRecord::GetInstance(); ASSERT(hr);
	DWORD dwBytesCommited = 0;
	AdemcoPrivateProtocal app;
	AttachmentReturnValue arv = CAdemcoFunc::ParsePacket(client->buff.buff + client->buff.rpos,
											client->buff.wpos - client->buff.rpos,
											app, &dwBytesCommited);
	BOOL bFaild = FALSE;
	BOOL bAck = FALSE;
	do {
		if (ARV_PACK_DATA_ERROR == arv) {
			arv = ARV_OK;
			ASSERT(0);
			client->buff.Clear();
			bFaild = TRUE;
		} else if (ARV_PACK_NOT_ENOUGH == arv) {
		} else {
			if (strncmp(app.id, AID_NULL, app.id_len) == 0) {
			} else if (strncmp(app.id, AID_HB, app.id_len) == 0) {
				if (app.admcid.len > 0) {
					client->ademco_id = app.admcid.acct;
					strncpy_s(client->acct, app.acct, app.acct_len);
					char out[1024] = { 0 };
					_snprintf_s(out, 1024, "[#%04d| %04d %03d] %s\n",
								client->ademco_id, app.admcid.event,
								app.admcid.zone,
								AdemcoFunc::CAdemcoFunc::GetAdemcoEventString(app.admcid.event));
					CLog::WriteLogA(out);
					wchar_t wacct[1024] = { 0 };
					AnsiToUtf16Array(client->acct, wacct, sizeof(wacct));
					if (CNetHostInfo::GetInstance()->CheckMachine(client->ademco_id,
						wacct, app.admcid.zone)) {
						CLog::WriteLogA("CheckMachine succeeded aid %04d, acct %s",
										client->ademco_id, client->acct);
						if (AdemcoFunc::CAdemcoFunc::IsStatusEvent(app.admcid.event)) {
							CLog::WriteLog(L"IsStatusEvent true event %d", app.admcid.event);
							//server->KillOtherClients(client->conn_id, client->ademco_id);
						}
						CNetHostInfo::GetInstance()->MachineOnline(client->ademco_id);
						CNetHostInfo::GetInstance()->MachineEventHandler(client->ademco_id,
																		 app.admcid.event,
																		 app.admcid.zone);
					} else {
						CString fm, rec;
						fm.LoadStringW(IDS_STRING_FM_KICKOUT_INVALID);
						rec.Format(fm, client->ademco_id, wacct);
						CHistoryRecord::GetInstance()->InsertRecord(EX_LEVEL_0, rec);
						CLog::WriteLog(rec);
						CLog::WriteLog(_T("Check acct-aid failed, pass.\n"));
						server->Release(client);
						goto EXIT_ON_RECV;
					}
#endif
				} else {
					bFaild = TRUE;
				}
			} else if (strncmp(app.id, AID_NAK, app.id_len) == 0) {
				CString record = _T("");
				record.LoadStringW(IDS_STRING_ILLEGAL_OP);
				hr->InsertRecord(0, record);
			} else if (strncmp(app.id, AID_MODULE_REG, app.id_len) == 0) {
				bFaild = TRUE;
				do {
					if (app.xdata_len != 32)
						break;
					char* module_id = new char[64];
					ZeroMemory(module_id, 64);
					strncpy_s(module_id, 64, app.xdata, 32);
					if (strlen(module_id) != 32) {
						break;
					}
					USES_CONVERSION;
					CString module_acct = _T("");
					module_acct = A2W(module_id);
					SAFEDELETEARR(module_id);
					if (module_acct.GetLength() != 32) {
						break;
					}

					if (!CDBOper::GetInstance()->CheckIfNetMachineAcctUnique(module_acct)) {
						//MyErrorMsgBox(GlobalGetSafeHWnd(), IDS_STRING_ACCT_NOT_UNIQUE);
						CString record = _T("");
						record.LoadStringW(IDS_STRING_ACCT_NOT_UNIQUE);
						CLog::WriteLog(record);
						hr->InsertRecord(0, record);
						break;
					}

					int ademco_id = CDBOper::GetInstance()->DistributeID(DB_TABLE_MAP_NET);
					if (ademco_id < 0 || ademco_id >= MAX_NET_MACHINE) {
						//MyErrorMsgBox(GlobalGetSafeHWnd(), IDS_STRING_NO_MORE_MACHINE);
						CString record = _T("");
						record.LoadStringW(IDS_STRING_NO_MORE_MACHINE);
						CLog::WriteLog(record);
						hr->InsertRecord(0, record);
						goto EXIT_ON_RECV;
					}

					NET_MAP_INFO nmi;
					nmi.id = ademco_id;
					nmi.ademco_id = ademco_id;
					nmi.acct = module_acct;
					//nmi.alias = m_alias;
					nmi.has_map = 0;
					nmi.map_id = -1;

					if (!CDBOper::GetInstance()->Add(&nmi, DB_TABLE_MAP_NET))
						break;

					CNetHostInfo::GetInstance()->AddMachine(nmi);

					char buff[1024] = { 0 };
					int bytes = CAdemcoFunc::GenerateRegRspPackage(buff, sizeof(buff), ademco_id);
					if (server->SendToClient(client, buff, bytes)) {
						bFaild = FALSE;
						CString record = _T("");
						record.Format(_T("%s %s, ademco_id %d, acct %s"),
									  GetStringTable(IDS_STRING_NEW),
									  GetStringTable(IDS_STRING_NETMACHINE),
									  ademco_id,
									  module_acct);
						CLog::WriteLog(record);
						hr->InsertRecord(0, record);
					}

				} while (0);
			} else if (strncmp(app.id, AID_PWW, app.id_len) == 0) {
				CLog::WriteLog(L"��������ٴ�����");
				CNetHostInfo::GetInstance()->PasswdWrong(client->ademco_id);
			} else if (strncmp(app.id, AID_ACK, app.id_len) == 0) {
				CLog::WriteLog(L"remote: ACK");
				bAck = TRUE;
				//break;
			} else {
				bFaild = TRUE;
			}
			char buff[BUFF_SIZE] = { 0 };
			const char* acct = NULL;
			int acct_len = 0;
			if (app.acct_len > 0) {
				acct = app.acct;
				acct_len = app.acct_len;
			} else if (strlen(client->acct) > 0) {
				acct = client->acct;
				acct_len = strlen(client->acct);
			} else {
				acct = "0000";
				acct_len = 4;
			}

			if (bFaild) {
				client->buff.Clear();
				
				DWORD dwSize = CAdemcoFunc::GenerateAckOrNakEvent(FALSE, -1, buff, 
																  BUFF_SIZE, 
																  acct,
																  acct_len);
				server->SendToClient(client, buff, dwSize);
			} else {
				client->buff.rpos = (client->buff.rpos + dwBytesCommited);
				if (!bAck) {
					DWORD dwSize = CAdemcoFunc::GenerateAckOrNakEvent(TRUE, -1, buff,
																	  BUFF_SIZE,
																	  acct,
																	  acct_len);
					server->SendToClient(client, buff, dwSize);
				}
			}
		}
	}while (0);

EXIT_ON_RECV:
	return arv;
}

CMyEventHandler *g_event_handler = NULL;
CServerService *g_select_server = NULL;


BOOL CServer::Start(WORD port)
{
	if (m_bServerStarted)
		return TRUE;

	

	try {
		Overlapped::Init( MAX_CONNECTIONS );
		g_mSockHndl = new MyISockEventHandler();
		g_sService = new MyServerService((MyISockEvent *) g_mSockHndl, PORT, 
			MAX_CONNECTIONS, NO_THREADS, TIME_OUT, false);
		g_sService->start();
	}
	catch (const char *err) {
		wchar_t* werr = AnsiToUtf16(err);
		CLog::WriteLog(werr);
		SAFEDELETEARR(werr);
	}
	catch (const wchar_t *err) {
		CLog::WriteLog(err);
	}
	
	m_bServerStarted = true;
	
	return TRUE;
}

void CServer::Stop()
{
	if (g_select_server)
		g_select_server->Stop();
	SAFEDELETEP(g_select_server);
	SAFEDELETEP(g_event_handler);	
	m_bServerStarted = false;
}

void CServer::Release()
{
	m_Lock4GetInstance.Lock();
	SAFEDELETEP(m_pInst);
	m_Lock4GetInstance.UnLock();
}

BOOL CServer::SendToClient(int ademco_id, int event, const char* psw)
{
	if(!m_bServerStarted)
		return FALSE;
	MyCSocket *pSocket = NULL;
	if(CLiveClientMap::FindLiveClient(ademco_id, pSocket)){
		pSocket->GetAttachment()->send_buff.clear();
		char *temp = pSocket->GetAttachment()->send_buff.buff;
		if (temp == NULL) {
			temp = new char[BUFF_SIZE];
			memset(temp, 0, BUFF_SIZE);
			pSocket->GetAttachment()->send_buff.buff = temp;
		}
		DWORD dwSize = GenerateEventPacket(temp, BUFF_SIZE, ademco_id,
			pSocket->GetAttachment()->acct, event, 0);
		pSocket->GetAttachment()->send_buff.dwSendingSize = dwSize;
		int nRet = pSocket->WriteToSocket( temp, dwSize );
		if ( nRet == SOCKET_ERROR ) 
			pSocket->CloseSocket();
	}
	return FALSE;
}

NAMESPACE_END
