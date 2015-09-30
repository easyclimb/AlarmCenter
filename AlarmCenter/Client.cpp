#include "stdafx.h"
#include "Client.h"
#include "ademco_func.h"
using namespace ademco;
#include "HistoryRecord.h"
#include "AlarmMachine.h"
#include "AlarmMachineManager.h"
#include "resource.h"
#include "CsrInfo.h"

namespace net {
namespace client {
#define LINK_TEST_GAP 30000

IMPLEMENT_SINGLETON(CClient)
CClient::CClient() : m_bClientServiceStarted(FALSE) 
{}

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
	AUTO_LOG_FUNCTION;
	memset(m_server_ip, 0, sizeof(m_server_ip));
}


CClientService::~CClientService()
{
	AUTO_LOG_FUNCTION;
}


void CClientService::SetEventHandler(CClientEventHandler* handler)
{
	AUTO_LOG_FUNCTION;
	if (handler) {
		m_handler = handler;
	}
}


BOOL CClientService::Start(const char* server_ip, unsigned short server_port)
{
	AUTO_LOG_FUNCTION;
	strcpy_s(m_server_ip, server_ip);
	m_server_port = server_port;
	Restart();
	return TRUE;
}

BOOL CClientService::Connect()
{
	AUTO_LOG_FUNCTION;
	if (m_bConnectionEstablished)
		return TRUE;
	do {
		memset(&m_server_addr, 0, sizeof(m_server_addr));
		m_server_addr.sin_family = AF_INET;
		m_server_addr.sin_addr.S_un.S_addr = inet_addr(m_server_ip);
		m_server_addr.sin_port = htons(m_server_port);

		if ((m_socket = socket(AF_INET, SOCK_STREAM, 0)) == INVALID_SOCKET) {
			CLog::WriteLog(_T("socket failed\n"));
			CLog::WriteLog(FormatWSAError(WSAGetLastError()));
			break;
		}

		// set the socket in non-blocking mode.
		unsigned long non_blocking_mode = 1;
		int result = ioctlsocket(m_socket, FIONBIO, &non_blocking_mode);
		if (result != NO_ERROR) {
			CLog::WriteLog(_T("ioctlsocket failed : %d\n"), result);
			CLog::WriteLog(FormatWSAError(WSAGetLastError()));
			CLOSESOCKET(m_socket);
			break;
		}

		int ret = connect(m_socket, (struct sockaddr *) &m_server_addr, 
						  sizeof(struct sockaddr));
		
		if (ret != -1) {
			CLog::WriteLogA("connect to %s:%d failed\n", m_server_ip, m_server_port);
			CLog::WriteLog(FormatWSAError(WSAGetLastError()));
			CLOSESOCKET(m_socket);
			break;
		}

		TIMEVAL tm;
		tm.tv_sec = 1;
		tm.tv_usec = 0;
		fd_set fdset;
		FD_ZERO(&fdset);
		FD_SET(m_socket, &fdset);
		if (select(m_socket + 1, NULL, &fdset, NULL, &tm) <= 0) {
			CLog::WriteLogA("connect to %s:%d failed\n", m_server_ip, m_server_port);
			CLog::WriteLog(FormatWSAError(WSAGetLastError()));
			CLOSESOCKET(m_socket);
			break;
		}

		int error, len;
		len = sizeof(int);
		getsockopt(m_socket, SOL_SOCKET, SO_ERROR, (char*)&error, &len);
		if (error != NO_ERROR) {
			CLog::WriteLogA("connect to %s:%d failed\n", m_server_ip, m_server_port);
			CLog::WriteLog(FormatWSAError(WSAGetLastError()));
			CLOSESOCKET(m_socket);
			break;
		}

		// set the socket in blocking mode.
		non_blocking_mode = 0;
		result = ioctlsocket(m_socket, FIONBIO, &non_blocking_mode);
		if (result != NO_ERROR) {
			CLog::WriteLog(_T("ioctlsocket failed : %d\n"), result);
			CLog::WriteLog(FormatWSAError(WSAGetLastError()));
			CLOSESOCKET(m_socket);
			break;
		}

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
	AUTO_LOG_FUNCTION;
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
	AUTO_LOG_FUNCTION;
	CClientService* service = reinterpret_cast<CClientService*>(lp);
	for (;;) {
		if (WAIT_OBJECT_0 == WaitForSingleObject(service->m_hEventShutdown, 10000))
			break;
		if (service->Connect()) {
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
	AUTO_LOG_FUNCTION;
	m_bShuttingDown = TRUE;

	if (INVALID_HANDLE_VALUE != m_hEventShutdown) {
		SetEvent(m_hEventShutdown);
	}

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
	
	Release();
}

void CClientService::Release()
{
	AUTO_LOG_FUNCTION;
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
	AUTO_LOG_FUNCTION;
	timeval tv = { 0, 10000 };
	fd_set fdWrite;
	
	int nRet = 0;

	do {
		FD_ZERO(&fdWrite);
		FD_SET(m_socket, &fdWrite);
		nRet = select(m_socket + 1, NULL, &fdWrite, NULL, &tv);
		if (WAIT_OBJECT_0 == WaitForSingleObject(m_hEventShutdown, 0))
			break;
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
	AUTO_LOG_FUNCTION;
	CClientService* service = reinterpret_cast<CClientService*>(lp);
	DWORD dwLastTimeSendLinkTest = 0;
	for (;;) {
		if (WAIT_OBJECT_0 == WaitForSingleObject(service->m_hEventShutdown, 1000))
			break;
		if (service->m_handler && service->m_bConnectionEstablished && GetTickCount() - dwLastTimeSendLinkTest >= LINK_TEST_GAP) {
			dwLastTimeSendLinkTest = GetTickCount();
			char buff[4096] = { 0 };
			DWORD dwLen = service->m_handler->GenerateLinkTestPackage(buff, sizeof(buff));
			if (dwLen > 0 && dwLen <= sizeof(buff)) {
				int nLen = service->Send(buff, dwLen);
				if (nLen <= 0) {
					CLog::WriteLog(_T("ThreadLinkTest::Send ret <= 0, ret %d"), nLen);
					service->Release();
					break;
				}
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
	AUTO_LOG_FUNCTION;
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
		if (WAIT_OBJECT_0 == WaitForSingleObject(service->m_hEventShutdown, 0))
			break;

		char* temp = service->m_buff.buff + service->m_buff.wpos;
		DWORD dwLenToRead = BUFF_SIZE - service->m_buff.wpos;
		nRet = recv(service->m_socket, temp, dwLenToRead, 0);

		if (nRet <= 0) {
			CLog::WriteLog(_T("ThreadRecv::recv ret <= 0, ret %d"), nRet);
			service->Release();
			break;
		} else if (service->m_handler) {
			service->m_buff.wpos += nRet;
			DWORD ret = RESULT_OK; 
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
				if (ret == RESULT_NOT_ENOUGH) {
					break;
				}
			}
		}
	}
	return 0;
}


class CMyClientEventHandler : public CClientEventHandler
{
	enum DEAL_CMD_RET
	{
		DCR_NULL,
		DCR_ONLINE,
		DCR_ACK,
		DCR_NAK,
	};
public:
	CMyClientEventHandler() : m_conn_id(-1){}
	virtual ~CMyClientEventHandler() {}
	virtual void OnConnectionEstablished(CClientService* service)
	{
		AUTO_LOG_FUNCTION;
		UNREFERENCED_PARAMETER(service);
		CWinApp* app = AfxGetApp();
		if (app) {
			CWnd* wnd = app->GetMainWnd();
			if (wnd) {
				wnd->PostMessageW(WM_NETWORKSTARTUPOK, 1);
			}
		}
	}

	virtual void OnConnectionLost(CClientService* service)
	{
		AUTO_LOG_FUNCTION;
		service->Restart();
		CWinApp* app = AfxGetApp();
		if (app) {
			CWnd* wnd = app->GetMainWnd();
			if (wnd) {
				wnd->PostMessageW(WM_NETWORKSTARTUPOK, 0);
			}
		}
	}

	virtual DWORD OnRecv(CClientService* service);
	virtual DWORD GenerateLinkTestPackage(char* buff, size_t buff_len);
	DEAL_CMD_RET DealCmd(AdemcoPacket& packet1, PrivatePacket& packet2);
	inline int GetConnID() const
	{
		return m_conn_id;
	}
private:
	int m_conn_id;
	typedef struct _CLIENT_DATA
	{
		bool online;
		//int conn_id;
		int ademco_id;
		_CLIENT_DATA() : online(false), /*conn_id(-1), */ademco_id(-1) {}
	}CLIENT_DATA;

	CLIENT_DATA m_clients[core::MAX_MACHINE];
};

CClientService* g_client_service = NULL;
CMyClientEventHandler *g_client_event_handler = NULL;

BOOL CClient::Start(const char* server_ip, unsigned short server_port)
{
	AUTO_LOG_FUNCTION;
	if (m_bClientServiceStarted)
		return TRUE;

	try {
		if (NULL == g_client_service) {
			g_client_service = new CClientService();
		}

		if (NULL == g_client_event_handler) {
			g_client_event_handler = new CMyClientEventHandler();
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
	AUTO_LOG_FUNCTION;
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


int CClient::SendToTransmitServer(int ademco_id, ADEMCO_EVENT ademco_event, int gg, 
								  int zone, const char* xdata, int xdata_len)
{
	AUTO_LOG_FUNCTION;
	if (g_client_service) {
		char data[BUFF_SIZE] = { 0 };
		core::CAlarmMachine* machine = NULL;
		if (core::CAlarmMachineManager::GetInstance()->GetMachine(ademco_id, machine)) {
			AdemcoPacket packet;
			const PrivatePacket* privatePacket = machine->GetPrivatePacket();
			if (!privatePacket)
				return 0;
			DWORD dwSize = 0;
			dwSize = packet.Make(data, sizeof(data), AID_HB, 0,
								 //machine->GetDeviceIDA(),
								 //privatePacket->_acct_machine,
								 //NULL,
								 ademco::HexCharArrayToStr(privatePacket->_acct_machine, 9),
								 ademco_id, ademco_event,
								 gg, zone, xdata, xdata_len);
			//ConnID conn_id = g_client_event_handler->GetConnID();
			PrivateCmd cmd;
			cmd.AppendConnID(privatePacket->_cmd.GetConnID());
			PrivatePacket packet2;
			dwSize += packet2.Make(data + dwSize, sizeof(data)-dwSize, 0x0c, 0x00, cmd,
								   privatePacket->_acct_machine,
								   privatePacket->_passwd_machine,
								   privatePacket->_acct,
								   privatePacket->_level);
			return g_client_service->Send(data, dwSize);
		}		
	}
	return 0;
}

DWORD CMyClientEventHandler::GenerateLinkTestPackage(char* buff, size_t buff_len)
{
	AUTO_LOG_FUNCTION;
	if (m_conn_id == -1)
		return 0;
	static int seq = 1;
	if (seq >= 9999)
		seq = 1;
	AdemcoPacket packet;
	DWORD dwLen = packet.Make(buff, buff_len, AID_NULL, seq++, /*ACCOUNT, */NULL, 
							  0, 0, 0, 0, NULL, 0);
	PrivatePacket packet2;
	ConnID conn_id = m_conn_id;
	PrivateCmd cmd;
	cmd.AppendConnID(conn_id);
	dwLen += packet2.Make(buff + dwLen, buff_len - dwLen, 0x06, 0x00, cmd, NULL, NULL, NULL, 0);
	return dwLen;
}

DWORD CMyClientEventHandler::OnRecv(CClientService* service)
{
	AUTO_LOG_FUNCTION;
	AdemcoPacket packet1;
	size_t dwBytesCmted = 0;
	ParseResult result1 = packet1.Parse(service->m_buff.buff + service->m_buff.rpos,
										service->m_buff.wpos - service->m_buff.rpos,
										dwBytesCmted);


	core::CHistoryRecord* hr = core::CHistoryRecord::GetInstance(); ASSERT(hr);
	if (result1 == RESULT_OK) {
		service->m_buff.rpos = (service->m_buff.rpos + dwBytesCmted);
		PrivatePacket packet2;
		ParseResult result2 = packet2.Parse(service->m_buff.buff + service->m_buff.rpos,
											service->m_buff.wpos - service->m_buff.rpos,
											dwBytesCmted);
		if (RESULT_DATA_ERROR == result2) {
			service->m_buff.Clear();
			ASSERT(0);
			return RESULT_OK;
		} else if (RESULT_NOT_ENOUGH == result2) {
			return RESULT_NOT_ENOUGH;
		} else {
			service->m_buff.rpos = (service->m_buff.rpos + dwBytesCmted);
			char buff[1024] = { 0 };
			DEAL_CMD_RET dcr = DealCmd(packet1, packet2);
			if (strcmp(ademco::AID_NAK, packet1._id) == 0) {
				CString record = _T("");
				record.LoadStringW(IDS_STRING_ILLEGAL_OP);
				hr->InsertRecord(packet1._data._ademco_id, 0, record, packet1._timestamp._time, core::RECORD_LEVEL_ONOFFLINE);
			}

			int seq = ademco::NumStr2Dec(packet1._seq, 4);
			if (seq > 9999) seq = 1;
			//const char* acct = NULL;
			////int acct_len = 0;
			//if (strlen(packet1._acct) > 0) {
			//	acct = packet1._acct;
			//	//acct_len = strlen(packet1._acct);
			//} else if (strlen(packet2._acct_machine) > 0) {
			//	acct = packet2._acct_machine;
			//	//acct_len = strlen(packet2._acct_machine);
			//} else {
			//	acct = "123456789";
			//	//acct_len = 9;
			//}

			if (dcr == DCR_ONLINE) {
				const char* csr_acct = core::CCsrInfo::GetInstance()->get_acctA();
				//const char* csr_acct = core::CAlarmMachineManager::GetInstance()->GetCsrAcctA();
				if (csr_acct/* && strlen(csr_acct) == 32*/) {
					//USES_CONVERSION;
					//const char* csr_acct = W2A(csr_acctW);
					size_t len = packet1.Make(buff, sizeof(buff), AID_HB, 0, 
											  /*csr_acct, */NULL, 
											  packet1._data._ademco_id, 0, 0, 0, NULL, 0);
					PrivateCmd cmd;
					cmd.AppendConnID(ConnID(m_conn_id));
					char temp[9] = { 0 };
					NumStr2HexCharArray_N(csr_acct, temp, 9);
					cmd.Append(temp, 9);
					len += packet2.Make(buff + len, sizeof(buff)-len, 0x06, 0x01, cmd,
										packet2._acct_machine,
										packet2._passwd_machine,
										packet2._acct, 
										packet2._level);
					service->Send(buff, len);
				}
			} else if (dcr == DCR_ACK) {
				size_t len = packet1.Make(buff, sizeof(buff), AID_ACK, seq,
										  /*acct, packet2._acct_machine, */
										  ademco::HexCharArrayToStr(packet2._acct_machine, 9),
										  packet1._data._ademco_id, 0, 0, 0, NULL, 0);
				PrivateCmd cmd;
				cmd.AppendConnID(packet2._cmd.GetConnID());
				len += packet2.Make(buff + len, sizeof(buff)-len, 0x0c, 0x01, cmd,
									packet2._acct_machine,
									packet2._passwd_machine,
									packet2._acct,
									packet2._level);
				service->Send(buff, len);
			} else if (dcr == DCR_NAK) {
				size_t len = packet1.Make(buff, sizeof(buff), AID_NAK, seq,
										  /*acct, packet2._acct_machine, */
										  ademco::HexCharArrayToStr(packet2._acct_machine, 9),
										  packet1._data._ademco_id, 0, 0, 0, NULL, 0);
				PrivateCmd cmd;
				cmd.AppendConnID(packet2._cmd.GetConnID());
				len += packet2.Make(buff + len, sizeof(buff)-len, 0x0c, 0x01, cmd,
									packet2._acct_machine,
									packet2._passwd_machine,
									packet2._acct,
									packet2._level);
				service->Send(buff, len);
			}
		}
	} else if (result1 == RESULT_NOT_ENOUGH) {
		return RESULT_NOT_ENOUGH;
	} else {
		ASSERT(0);
		service->m_buff.Clear();
	}
	return RESULT_OK;
}

CMyClientEventHandler::DEAL_CMD_RET CMyClientEventHandler::DealCmd(AdemcoPacket& packet1, PrivatePacket& packet2)
{
	AUTO_LOG_FUNCTION;
	const PrivateCmd* private_cmd = &packet2._cmd;
	//int private_cmd_len = private_cmd->_size;

	//if (private_cmd_len != 3 && private_cmd_len != 32)
	//	return DCR_NULL;

	DWORD conn_id = private_cmd->GetConnID().ToInt();
	core::CAlarmMachineManager* mgr = core::CAlarmMachineManager::GetInstance(); ASSERT(mgr);
	if (packet2._big_type == 0x07) {			// from Transmit server
		switch (packet2._lit_type) {
			case 0x00:	// link test responce
				CLog::WriteLog(_T("Transmite server link test responce\n"));
				break;
			case 0x01:	// conn_id
				m_conn_id = conn_id;
				CLog::WriteLog(_T("Transmite server responce my conn_id %d\n"), m_conn_id);
				return DCR_ONLINE;
				break;
			case 0x02:	// alarm machine connection lost
				if (conn_id < 0 || conn_id >= core::MAX_MACHINE) {
					return DCR_NULL;
				}
				if (m_clients[conn_id].online) {
					mgr->MachineOnline(ES_TCP_SERVER, m_clients[conn_id].ademco_id, FALSE);
					m_clients[conn_id].online = false;
					core::CAlarmMachine* machine = NULL;
					if (mgr->GetMachine(m_clients[conn_id].ademco_id, machine) && machine) {
						machine->SetPrivatePacket(NULL);
					}
					m_clients[conn_id].ademco_id = -1;
				}
				break;
			default:
				break;
		}
	} else if (packet2._big_type == 0x0d) {	// from Alarm Machine
		if (packet2._lit_type == 0x00) {	// Alarm machine on/off line, event report.
			int ademco_id = packet1._data._ademco_id;
			ADEMCO_EVENT ademco_event= packet1._data._ademco_event;
			int zone = packet1._data._zone;
			int subzone = packet1._data._gg;
			/*if (packet1._xdata) {
				subzone = atoi(packet1._xdata);
			}*/

			CLog::WriteLogA("alarm machine EVENT:0d 00 aid %04d event %04d zone %03d %s\n",
							ademco_id, ademco_event, zone, packet1._timestamp._data);

			BOOL ok = TRUE;
			do {
				if (!m_clients[conn_id].online) {
					char acct[64] = { 0 };
					strcpy_s(acct, packet1._acct);
					CLog::WriteLogA("alarm machine ONLINE:0d 00 aid %04d acct %s online.\n",
									ademco_id, acct);
					if (!mgr->CheckIsValidMachine(ademco_id, /*acct, */zone)) {
						ok = FALSE; break;
					}

					core::CAlarmMachine* machine = NULL;
					if (mgr->GetMachine(ademco_id, machine) && machine) {
						machine->SetPrivatePacket(&packet2);
					}

					m_clients[conn_id].online = true;
					m_clients[conn_id].ademco_id = ademco_id;
					mgr->MachineOnline(ES_TCP_SERVER, ademco_id);
					mgr->MachineEventHandler(ES_TCP_SERVER, ademco_id, ademco_event, zone,
											 subzone, packet1._timestamp._time, time(NULL),
											 packet1._xdata, packet1._xdata_len);
				} else {
					mgr->MachineEventHandler(ES_TCP_SERVER, ademco_id, ademco_event, zone,
											 subzone, packet1._timestamp._time, time(NULL),
											 packet1._xdata, packet1._xdata_len);
				}
			} while (0);

			return ok ? DCR_ACK : DCR_NAK;
		} else if (packet2._lit_type == 0x01) { // todo
			// 2014Äê11ÔÂ26ÈÕ 17:02:23 add
			LOG(L"0x0d 0x01 todo................................\n");
		}
	} 

	return DCR_NULL;
}

NAMESPACE_END
NAMESPACE_END
