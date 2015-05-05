#include "stdafx.h"
#include "server.h"

// Server.cpp: implementation of the CServer class.
//
//////////////////////////////////////////////////////////////////////


#include "../AlarmCenter/ServerService.h"
#include "resource.h"
#include <list>

#include "../AlarmCenter/ademco_func.h"
using namespace ademco;

#include <time.h>



#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

#ifdef _DEBUG 
#define TIME_OUT		10			// in seconds
#else
#define TIME_OUT		120			// in seconds
#endif
#define THREAD_HANDLER_NO	4

namespace net {
namespace server {

IMPLEMENT_SINGLETON(CServer)
CServer::CServer() : m_bServerStarted(false), m_cb(NULL), m_udata(NULL)
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
	OnAdemcoEvent m_cb;
	void* m_udata;
	CMyServerEventHandler() 
		: m_nSignaledEventCount(0) 
		, m_hEventShutdown(INVALID_HANDLE_VALUE)
		, m_phThreadHandlers(NULL)
		, m_cb(NULL)
		, m_udata(NULL)
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
		CLog::WriteLogA("new connection from %s:%d\n",
						inet_ntoa(client->foreignAddIn.sin_addr),
						client->foreignAddIn.sin_port);
		
	}

	virtual void OnConnectionLost(CServerService *server, CClientData *client)
	{
		UNREFERENCED_PARAMETER(server);
		//cout << "connection lost at " << inet_ntoa(client->foreignAddIn.sin_addr) << ":" <<
		//	client->foreignAddIn.sin_port << endl;
		CLog::WriteLogA("connection lost at %s:%d\n",
						inet_ntoa(client->foreignAddIn.sin_addr),
						client->foreignAddIn.sin_port);
		//wchar_t wacct[1024] = { 0 };
		//AnsiToUtf16Array(client->acct, wacct, sizeof(wacct));
		//if (core::CAlarmMachineManager::GetInstance()->CheckIsValidMachine(client->ademco_id,
		//	client->acct, 0)) {
		//	core::CAlarmMachineManager::GetInstance()->MachineOnline(client->ademco_id, FALSE);
		//}
	}
};

DWORD CMyServerEventHandler::OnRecv(CServerService *server, CClientData* client)
{
	USES_CONVERSION;
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
				/*if (packet._xdata) {+		packet._acct	0x0193fa47 "ee71f17244ba5c95e3bf4990df79a1ac"	char[64]

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
				if (m_cb && m_udata) {
					m_cb(m_udata, ademco_event);
				}

				if (!client->online) {
					client->online = true;
					server->ReferenceClient(client->ademco_id, client);
				}
				
			} else {
				bFaild = TRUE;
			}
		} else if (strcmp(packet._id, AID_NAK) == 0) {
			LOG(L"NAK\n");
		} else if (strcmp(packet._id, AID_ACK) == 0) {
			CLog::WriteLog(L"ACK");
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

		if (bFaild) {
			client->buff.Clear();
			DWORD dwSize = packet.Make(buff, BUFF_SIZE, AID_NAK, 0,
									   acct, 0, 0, 0, 0, NULL, 0);
			server->SendToClient(client, buff, dwSize);
		} else {
			client->buff.rpos = (client->buff.rpos + dwBytesCommited);
			if (bNeed2ReplyAck) {
				DWORD dwSize = packet.Make(buff, BUFF_SIZE, AID_ACK, 0, acct, 
										   client->ademco_id, 0, 0, 0, NULL, 0);
				server->SendToClient(client, buff, dwSize);
			}
		}
	}

	return result;
}

CMyServerEventHandler *g_event_handler = NULL;
CServerService *g_select_server = NULL;


BOOL CServer::Start(WORD port)
{
	if (m_bServerStarted)
		return TRUE;
	
	try {
		int	nRet;
		WSAData	wsData;

		nRet = WSAStartup(MAKEWORD(2, 2), &wsData);
		if (nRet < 0) {
			CLog::WriteLog(L"Can't load winsock.dll.\n");
			return FALSE;
		}
		g_event_handler = new CMyServerEventHandler();
		g_event_handler->m_cb = m_cb;
		g_event_handler->m_udata = m_udata;
		g_select_server = new CServerService(port, 100, TIME_OUT, true, false);
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
	WSACleanup();
}


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
