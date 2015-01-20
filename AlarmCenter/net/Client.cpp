#include "stdafx.h"
#include "Client.h"
#include "../ademco_func.h"
using namespace AdemcoFunc;
#include "../HistoryRecord.h"

namespace net {
#define LINK_TEST_GAP 3000

CLock CClient::m_lock4Instance;

CClientService::CClientService()
	: m_buff()
	, m_socket(INVALID_SOCKET)
	, m_bConnectionEstablished(FALSE)
	, m_server_addr()
	, m_handler(NULL)
	, m_hEventShutdown(INVALID_HANDLE_VALUE)
	, m_hThreadRecv(INVALID_HANDLE_VALUE)
	, m_hThreadReconnectServer(INVALID_HANDLE_VALUE)
	, m_hThreadLinkTest(INVALID_HANDLE_VALUE)
	, m_server_port(0)
	, m_bShuttingDown(FALSE)
{
	memset(m_server_ip, 0, sizeof(m_server_ip));
}


CClientService::~CClientService()
{}


void CClientService::SetEventHandler(CClientEventHandler* handler)
{
	if (handler) {
		m_handler = handler;
	}
}


BOOL CClientService::Start(const char* server_ip, unsigned short server_port)
{
	if (m_bConnectionEstablished)
		return TRUE;
	do {
		strcpy_s(m_server_ip, server_ip);
		m_server_port = server_port;

		memset(&m_server_addr, 0, sizeof(m_server_addr));
		m_server_addr.sin_family = AF_INET;
		m_server_addr.sin_addr.S_un.S_addr = inet_addr(server_ip);
		m_server_addr.sin_port = htons(server_port);

		if ((m_socket = socket(AF_INET, SOCK_STREAM, 0)) == INVALID_SOCKET) {
			CLog::WriteLog(_T("socket failed\n"));
			CLog::WriteLog(FormatWSAError(WSAGetLastError()));
			break;
		}

		//// set the socket in non-blocking mode.
		//unsigned long non_blocking_mode = 1;
		//int result = ioctlsocket(m_socket, FIONBIO, &non_blocking_mode);
		//if (result != NO_ERROR) {
		//	CLog::WriteLog(_T("ioctlsocket failed : %d\n"), result);
		//	CLog::WriteLog(FormatWSAError(WSAGetLastError()));
		//	CLOSESOCKET(m_socket);
		//	break;
		//}

		if (connect(m_socket, (struct sockaddr *) &m_server_addr, sizeof(struct sockaddr)) != 0) {
			CLog::WriteLog(_T("connect failed\n"));
			CLog::WriteLog(FormatWSAError(WSAGetLastError()));
			CLOSESOCKET(m_socket);
			break;
		}

		//// set the socket in blocking mode.
		//non_blocking_mode = 0;
		//result = ioctlsocket(m_socket, FIONBIO, &non_blocking_mode);
		//if (result != NO_ERROR) {
		//	CLog::WriteLog(_T("ioctlsocket failed : %d\n"), result);
		//	CLog::WriteLog(FormatWSAError(WSAGetLastError()));
		//	CLOSESOCKET(m_socket);
		//	break;
		//}

		//fd_set fdWrite;
		//FD_ZERO(&fdWrite);
		//FD_SET(m_socket, &fdWrite);
		//timeval tv = { 1, 0 };

		//// check if the socket is ready
		//select(0, NULL, &fdWrite, NULL, &tv);
		//if (!FD_ISSET(m_socket, &fdWrite)) {
		//	CLog::WriteLog(_T("FD_ISSET failed\n"));
		//	CLog::WriteLog(FormatWSAError(WSAGetLastError()));
		//	CLOSESOCKET(m_socket);
		//	break;
		//}

		m_bConnectionEstablished = TRUE;
		m_buff.Clear();

		if (m_handler) {
			m_handler->OnConnectionEstablished(this);
		}

		if (INVALID_HANDLE_VALUE == m_hEventShutdown) {
			m_hEventShutdown = CreateEvent(NULL, TRUE, FALSE, NULL);
		}

		if (INVALID_HANDLE_VALUE == m_hThreadRecv) {
			m_hThreadRecv = CreateThread(NULL, 0, ThreadRecv, this, 0, NULL);
		}
		
		if (INVALID_HANDLE_VALUE == m_hThreadLinkTest) {
			m_hThreadLinkTest = CreateThread(NULL, 0, ThreadLinkTest, this, 0, NULL);
		}

		return TRUE;
	} while (0);

	Restart();

	return FALSE;
}


void CClientService::Restart()
{
	if (m_bShuttingDown)
		return;

	if (INVALID_HANDLE_VALUE == m_hEventShutdown) {
		m_hEventShutdown = CreateEvent(NULL, TRUE, FALSE, NULL);
	}
	ResetEvent(m_hEventShutdown);
	if (INVALID_HANDLE_VALUE == m_hThreadReconnectServer) {
		m_hThreadReconnectServer = CreateThread(NULL, 0, ThreadReconnectServer, this, 0, NULL);
	}
}

DWORD WINAPI CClientService::ThreadReconnectServer(LPVOID lp)
{
	CClientService* service = reinterpret_cast<CClientService*>(lp);
	for (;;) {
		if (WAIT_OBJECT_0 == WaitForSingleObject(service->m_hEventShutdown, 3000))
			break;
		if (service->Start(service->m_server_ip, service->m_server_port)) {
			break;
		} else {
			Sleep(100);
		}
	}
	CLOSEHANDLE(service->m_hThreadReconnectServer);
	return 0;
}


void CClientService::Stop()
{
	if (INVALID_HANDLE_VALUE != m_hEventShutdown) {
		m_bShuttingDown = TRUE;
		SetEvent(m_hEventShutdown);

		if (INVALID_HANDLE_VALUE != m_hThreadReconnectServer) {
			WaitForSingleObject(m_hThreadReconnectServer, INFINITE);
			CLOSEHANDLE(m_hThreadReconnectServer);
		}

		if (INVALID_HANDLE_VALUE != m_hThreadLinkTest) {
			WaitForSingleObject(m_hThreadLinkTest, INFINITE);
			CLOSEHANDLE(m_hThreadLinkTest);
		}
		
		if (INVALID_HANDLE_VALUE != m_hThreadRecv) {
			WaitForSingleObject(m_hThreadRecv, INFINITE);
			CLOSEHANDLE(m_hThreadRecv);
		}

		CLOSEHANDLE(m_hEventShutdown);
	}
	Release();
}

void CClientService::Release()
{
	if (m_bConnectionEstablished) {
		if (INVALID_HANDLE_VALUE != m_hEventShutdown) {
			SetEvent(m_hEventShutdown);
		}
		CLOSEHANDLE(m_hThreadLinkTest);
		CLOSEHANDLE(m_hThreadRecv);
		shutdown(m_socket, 2);
		closesocket(m_socket);
		m_socket = INVALID_SOCKET;
		if (m_handler) {
			m_handler->OnConnectionLost(this);
		}
		m_bConnectionEstablished = FALSE;
	}
}


int CClientService::Send(const char* buff, size_t buff_size)
{
	timeval tv = { 0, 10000 };
	fd_set fdWrite;
	
	int nRet = 0;

	do {
		FD_ZERO(&fdWrite);
		FD_SET(m_socket, &fdWrite);
		nRet = select(m_socket + 1, NULL, &fdWrite, NULL, &tv);
	} while (nRet <= 0 && !FD_ISSET(m_socket, &fdWrite));

	nRet = send(m_socket, buff, buff_size, 0);

	if (nRet <= 0) {
		CLog::WriteLog(_T("CClientService::Send ret <= 0, ret %d"), nRet);
		Release();
	}

	return nRet;
}


DWORD WINAPI CClientService::ThreadLinkTest(LPVOID lp)
{
	CClientService* service = reinterpret_cast<CClientService*>(lp);
	DWORD dwLastTimeSendLinkTest = 0;
	for (;;) {
		if (WAIT_OBJECT_0 == WaitForSingleObject(service->m_hEventShutdown, 100))
			break;
		if (service->m_handler && GetTickCount() - dwLastTimeSendLinkTest >= LINK_TEST_GAP) {
			char buff[4096] = { 0 };
			DWORD dwLen = service->m_handler->GenerateLinkTestPackage(buff, sizeof(buff));
			if (dwLen > 0 && dwLen <= sizeof(buff)) {
				int nLen = service->Send(buff, dwLen);
				if (nLen <= 0) {
					CLog::WriteLog(_T("ThreadLinkTest::Send ret <= 0, ret %d"), nLen);
					service->Release();
					break;
				}
				dwLastTimeSendLinkTest = GetTickCount();
#ifdef _DEBUG
				DWORD dwThreadID = GetCurrentThreadId();
				CLog::WriteLog(_T("CClientService::ThreadLinkTest id %d is running.\n"), dwThreadID);
#endif
				CLog::WriteLog(_T("Send link test to transmite server, len %d\n"), nLen);
			}
		}
	}
	return 0;
}


DWORD WINAPI CClientService::ThreadRecv(LPVOID lp)
{
	CClientService* service = reinterpret_cast<CClientService*>(lp);
	timeval tv = { 0, 10 };
	
	for (;;) {
		if (WAIT_OBJECT_0 == WaitForSingleObject(service->m_hEventShutdown, 1))
			break;
		int nRet = 0;
		fd_set fdRead;

		do {
			FD_ZERO(&fdRead);
			FD_SET(service->m_socket, &fdRead);
			nRet = select(service->m_socket + 1, &fdRead, NULL, NULL, &tv);
			if (WAIT_OBJECT_0 == WaitForSingleObject(service->m_hEventShutdown, 1))
				break;
		} while (nRet <= 0 && !FD_ISSET(service->m_socket, &fdRead));

		char* temp = service->m_buff.buff + service->m_buff.wpos;
		DWORD dwLenToRead = BUFF_SIZE - service->m_buff.wpos;
		nRet = recv(service->m_socket, temp, dwLenToRead, 0);

		if (nRet <= 0) {
			CLog::WriteLog(_T("ThreadRecv::recv ret <= 0, ret %d"), nRet);
			service->Release();
			break;
		} else if (service->m_handler) {
			service->m_buff.wpos += nRet;
			DWORD ret = ARV_OK; 
			ret = service->m_handler->OnRecv(service);

			while (1) {
				unsigned int bytes_not_commited = service->m_buff.wpos - service->m_buff.rpos;
				if (bytes_not_commited == 0) {
					if (service->m_buff.wpos == BUFF_SIZE) {
						service->m_buff.Clear();
					}
					break;
				}
				if (service->m_buff.wpos == BUFF_SIZE) {
					memmove_s(service->m_buff.buff, BUFF_SIZE,
							  service->m_buff.buff + service->m_buff.rpos,
							  bytes_not_commited);
					memset(service->m_buff.buff + bytes_not_commited,
						   0, BUFF_SIZE - bytes_not_commited);
					service->m_buff.wpos -= service->m_buff.rpos;
					service->m_buff.rpos = 0;
					ret = service->m_handler->OnRecv(service);
				} else {
					ret = service->m_handler->OnRecv(service);
				}
				if (ret == ARV_PACK_NOT_ENOUGH) {
					break;
				}
			}
		}
	}
	return 0;
}


CClient* CClient::GetInstance()
{
	m_lock4Instance.Lock();
	static CClient client;
	m_lock4Instance.UnLock();
	return &client;
}


class MyClientEventHandler : public CClientEventHandler
{
	enum DEAL_CMD_RET
	{
		DCR_NULL,
		DCR_ONLINE,
		DCR_ACK,
		DCR_NAK,
	};
public:
	MyClientEventHandler() : m_conn_id(-1), m_conn_id1(0), m_conn_id2(0), m_conn_id3(0) {}
	virtual ~MyClientEventHandler() {}
	virtual void OnConnectionEstablished(CClientService* service)
	{
		UNREFERENCED_PARAMETER(service);
		CLog::WriteLog(_T("connection established\n"));
	}

	virtual void OnConnectionLost(CClientService* service)
	{
		CLog::WriteLog(_T("connection lost\n"));
		service->Restart();
	}

	virtual DWORD OnRecv(CClientService* service);
	virtual DWORD GenerateLinkTestPackage(char* buff, size_t buff_len);
	DEAL_CMD_RET DealCmd(AdemcoFunc::AdemcoPrivateProtocal& app);
	inline int GetConnID() const
	{
		return m_conn_id;
	}
private:
	int m_conn_id;
	BYTE m_conn_id1;
	BYTE m_conn_id2;
	BYTE m_conn_id3;
	typedef struct _CLIENT_DATA
	{
		bool online;
		//int conn_id;
		int ademco_id;
		_CLIENT_DATA() : online(false), /*conn_id(-1), */ademco_id(-1) {}
	}CLIENT_DATA;

	CLIENT_DATA m_clients[MAX_NET_MACHINE];
};

CClientService* g_client_service = NULL;
MyClientEventHandler *g_client_event_handler = NULL;

BOOL CClient::Start(const char* server_ip, unsigned short server_port)
{
	if (m_bClientServiceStarted)
		return TRUE;

	try {
		if (NULL == g_client_service) {
			g_client_service = new CClientService();
		}

		if (NULL == g_client_event_handler) {
			g_client_event_handler = new MyClientEventHandler();
		}

		g_client_service->SetEventHandler(g_client_event_handler);
		g_client_service->Start(server_ip, server_port);

		m_bClientServiceStarted = TRUE;
	} catch (const char* err) {
		OutputDebugStringA(err);
	} catch (const wchar_t* err) {
		OutputDebugStringW(err);
	}

	return m_bClientServiceStarted;
}


void CClient::Stop()
{
	if (NULL != g_client_service) {
		g_client_service->Stop();
		delete g_client_service;
		g_client_service = NULL;
	}

	if (NULL != g_client_event_handler) {
		delete g_client_event_handler;
		g_client_event_handler = NULL;
	}
}


int CClient::SendToTransmitServer(int ademco_id, int event, const char* psw)
{
	if (g_client_service) {
		char data[BUFF_SIZE] = { 0 };
		PNET_MACHINE_INFO pNmi = NULL;
		if (CNetHostInfo::GetInstance()->GetHostInfo(ademco_id, pNmi)) {
			char acct[1024] = { 0 };
			Utf16ToAnsiUseCharArray(pNmi->acct, acct, 1024);
			DWORD dwSize = CAdemcoFunc::GenerateEventPacket(data, BUFF_SIZE, ademco_id,
											   acct, event, 0, psw, TRUE, 
											   g_client_event_handler->GetConnID());
			return g_client_service->Send(data, dwSize);
		}		
	}
	return 0;
}

DWORD MyClientEventHandler::GenerateLinkTestPackage(char* buff, size_t buff_len)
{
	if (m_conn_id == -1)
		return 0;
	DWORD dwLen = CAdemcoFunc::GenerateConnTestPacket(m_conn_id, buff, buff_len, FALSE, TRUE);
	return dwLen;
}

DWORD MyClientEventHandler::OnRecv(CClientService* service)
{
	AdemcoPrivateProtocal app;
	DWORD dwBytesCmted = 0;
	AttachmentReturnValue arv = CAdemcoFunc::ParsePacket(service->m_buff.buff + service->m_buff.rpos,
														 service->m_buff.wpos - service->m_buff.rpos,
														 app, &dwBytesCmted, TRUE);

	if (arv == ARV_OK) {
		service->m_buff.rpos = (service->m_buff.rpos + dwBytesCmted);
		char buff[1024] = { 0 };
		DEAL_CMD_RET dcr = DealCmd(app);
		BYTE conn_id1 = static_cast<BYTE>(app.private_cmd[2]);
		BYTE conn_id2 = static_cast<BYTE>(app.private_cmd[3]);
		BYTE conn_id3 = static_cast<BYTE>(app.private_cmd[4]);
		DWORD conn_id = MAKELONG(MAKEWORD(conn_id3, conn_id2), MAKEWORD(conn_id1, 0));

		if (strncmp(AdemcoFunc::AID_NAK, app.id, app.id_len) == 0) {
			CString record = _T("");
			record.LoadStringW(IDS_STRING_ILLEGAL_OP);
			CHistoryRecord::GetInstance()->InsertRecord(0, record);
		}

		if (dcr == DCR_ONLINE) {
			CAlarmCenterApp* papp = reinterpret_cast<CAlarmCenterApp*>(AfxGetApp());	ASSERT(papp);
			while (strlen(papp->m_csr_acct) != 32) {
				Sleep(1000);
			}

			int len = CAdemcoFunc::GenerateOnlinePackage(buff, sizeof(buff), m_conn_id,
														 papp->m_csr_acct, strlen(papp->m_csr_acct));
			service->Send(buff, len);
		} else if (dcr == DCR_ACK) {
			int len = CAdemcoFunc::GenerateAckOrNakEvent(FALSE, conn_id,
														 buff, sizeof(buff),
														 app.acct, app.acct_len,
														 TRUE);
			service->Send(buff, len);
		} else if (dcr == DCR_NAK) {
			int len = CAdemcoFunc::GenerateAckOrNakEvent(TRUE, conn_id,
														 buff, sizeof(buff),
														 app.acct, app.acct_len,
														 TRUE);
			service->Send(buff, len);
		}

	} else if (arv == ARV_PACK_NOT_ENOUGH) {
		return ARV_PACK_NOT_ENOUGH;
	} else {
		ASSERT(0);
		service->m_buff.Clear();
	}
	return ARV_OK;
}

MyClientEventHandler::DEAL_CMD_RET MyClientEventHandler::DealCmd(AdemcoPrivateProtocal& app)
{
	const char* private_cmd = app.private_cmd;
	int private_cmd_len = app.private_cmd_len;

	if (private_cmd_len != 5 && private_cmd_len != 34)
		return DCR_NULL;

	BYTE bigType = static_cast<BYTE>(private_cmd[0]);
	BYTE litType = static_cast<BYTE>(private_cmd[1]);
	BYTE conn_id1 = static_cast<BYTE>(private_cmd[2]);
	BYTE conn_id2 = static_cast<BYTE>(private_cmd[3]);
	BYTE conn_id3 = static_cast<BYTE>(private_cmd[4]);
	//DWORD conn_id = MAKELONG(MAKEWORD(conn_id3, conn_id2), MAKEWORD(conn_id1, 0));
	DWORD conn_id = CAdemcoFunc::MakeConnID(conn_id1, conn_id2, conn_id3);

	if (bigType == 0x07) {			// from Transmit server
		switch (litType) {
			case 0x00:	// link test responce
				CLog::WriteLog(_T("Transmite server link test responce\n"));
				break;
			case 0x01:	// conn_id
				m_conn_id = conn_id;
				m_conn_id1 = conn_id1;
				m_conn_id2 = conn_id2;
				m_conn_id3 = conn_id3;
				CLog::WriteLog(_T("Transmite server responce my conn_id %d\n"), m_conn_id);
				return DCR_ONLINE;
				break;
			case 0x02:	// alarm machine connection lost
				if (conn_id < 0 || conn_id >= MAX_NET_MACHINE) {
					return DCR_NULL;
				}
				if (m_clients[conn_id].online) {
					CNetHostInfo::GetInstance()->MachineOnline(m_clients[conn_id].ademco_id, FALSE);
					m_clients[conn_id].online = false;
					m_clients[conn_id].ademco_id = -1;
				}
				break;
			default:
				break;
		}
	} else if (bigType == 0x0d) {	// from Alarm Machine
		if (litType == 0x00) {	// Alarm machine on/off line, event report.
			int ademco_id = app.admcid.acct;
			int event = app.admcid.event;
			int zone = app.admcid.zone;

			CLog::WriteLogA("alarm machine EVENT:0d 00 aid %04d event %04d zone %03d.\n",
				  ademco_id, event, zone);

			BOOL ok = TRUE;
			do {
				if (!CNetHostInfo::GetInstance()->IsValidAdemcoID(ademco_id)
					|| !CNetHostInfo::GetInstance()->IsValidAreaID(zone)) {
					//return DCR_NULL;
					ok = FALSE;
					break;
				}

				if (!m_clients[conn_id].online) {
					char acct[64] = { 0 };
					strncpy_s(acct, app.acct, app.acct_len);
					CLog::WriteLogA("alarm machine ONLINE:0d 00 aid %04d acct %s online.\n",
									ademco_id, acct);
					wchar_t wacct[1024] = { 0 };
					AnsiToUtf16Array(acct, wacct, sizeof(wacct));
					if (!CNetHostInfo::GetInstance()->CheckMachine(ademco_id, wacct, zone)) {
						//return DCR_NULL;
						ok = FALSE;
						break;
					}

					m_clients[conn_id].online = true;
					m_clients[conn_id].ademco_id = ademco_id;
					CNetHostInfo::GetInstance()->MachineOnline(ademco_id);
					CNetHostInfo::GetInstance()->MachineEventHandler(ademco_id, event, zone);
				} else {
					CNetHostInfo::GetInstance()->MachineEventHandler(ademco_id, event, zone);
				}
			} while (0);

			return ok ? DCR_ACK : DCR_NAK;
		} else if (litType == 0x01) { // todo
			// 2014Äê11ÔÂ26ÈÕ 17:02:23 add
			
		}
	} else {
		return DCR_NULL;
	}
	return DCR_NULL;
}

NAMESPACE_END
