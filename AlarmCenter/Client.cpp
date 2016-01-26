#include "stdafx.h"
#include "Client.h"
#include "ademco_func.h"
using namespace ademco;
#include "HistoryRecord.h"
#include "AlarmMachine.h"
#include "AlarmMachineManager.h"
#include "CsrInfo.h"
#include <memory>
#include <mutex>
#include "AppResource.h"
#include "ConfigHelper.h"

namespace net {
namespace client {
#ifdef _DEBUG
	static const int LINK_TEST_GAP = 30000;
#else
	static const int LINK_TEST_GAP = 5000;
#endif
CClient::CClient(bool main_client)
	: m_bClientServiceStarted(FALSE)
	, main_client_(main_client)
{}

CClientService::CClientService(bool main_client)
	: m_buff()
	, m_socket(INVALID_SOCKET)
	, m_bConnectionEstablished(FALSE)
	, m_server_addr()
	, m_handler(nullptr)
	, m_hEventShutdown(INVALID_HANDLE_VALUE)
	, m_hThreadRecv(INVALID_HANDLE_VALUE)
	//, m_hThreadReconnectServer(INVALID_HANDLE_VALUE)
	//, m_hThreadLinkTest(INVALID_HANDLE_VALUE)
	, m_server_ip()
	, m_server_port(0)
	, m_bShuttingDown(FALSE)
	, main_client_(main_client)
	, last_recv_time_()
{
	AUTO_LOG_FUNCTION;
}


CClientService::~CClientService()
{
	AUTO_LOG_FUNCTION;
}


void CClientService::SetEventHandler(std::shared_ptr<CClientEventHandler> handler)
{
	AUTO_LOG_FUNCTION;
	if (handler) {
		m_handler = handler;
	}
}


BOOL CClientService::Start(const std::string& server_ip, unsigned int server_port)
{
	AUTO_LOG_FUNCTION;
	m_server_ip = server_ip;
	m_server_port = server_port;
	//Restart();
	m_bShuttingDown = FALSE;
	if (INVALID_HANDLE_VALUE == m_hEventShutdown) {
		m_hEventShutdown = CreateEvent(nullptr, TRUE, FALSE, nullptr);
	}

	if (INVALID_HANDLE_VALUE == m_hThreadRecv) {
		m_hThreadRecv = CreateThread(nullptr, 0, ThreadWorker, this, 0, nullptr);
	}
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
		m_server_addr.sin_addr.S_un.S_addr = inet_addr(m_server_ip.c_str());
		m_server_addr.sin_port = htons(static_cast<u_short>(m_server_port));

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
			CLog::WriteLogA("connect to %s:%d failed\n", m_server_ip.c_str(), m_server_port);
			CLog::WriteLog(FormatWSAError(WSAGetLastError()));
			CLOSESOCKET(m_socket);
			break;
		}

		TIMEVAL tm;
		tm.tv_sec = 3;
		tm.tv_usec = 0;
		fd_set fdset;
		FD_ZERO(&fdset);
		FD_SET(m_socket, &fdset);
		if (select(m_socket + 1, nullptr, &fdset, nullptr, &tm) <= 0) {
			//CLog::WriteLogA("connect to %s:%d failed\n", m_server_ip.c_str(), m_server_port);
			//CLog::WriteLog(FormatWSAError(WSAGetLastError()));
			CLOSESOCKET(m_socket);
			break;
		}

		int error, len;
		len = sizeof(int);
		getsockopt(m_socket, SOL_SOCKET, SO_ERROR, (char*)&error, &len);
		if (error != NO_ERROR) {
			CLog::WriteLogA("connect to %s:%d failed\n", m_server_ip.c_str(), m_server_port);
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
		//select(0, nullptr, &fdWrite, nullptr, &tv);
		//if (!FD_ISSET(m_socket, &fdWrite)) {
		//	CLog::WriteLog(_T("FD_ISSET failed\n"));
		//	CLog::WriteLog(FormatWSAError(WSAGetLastError()));
		//	CLOSESOCKET(m_socket);
		//	break;
		//}

		m_bConnectionEstablished = TRUE;
		m_buff.Clear();
		last_recv_time_ = COleDateTime::GetTickCount();

		if (m_handler) {
			m_handler->OnConnectionEstablished(this);
		}

		/*if (INVALID_HANDLE_VALUE == m_hEventShutdown) {
			m_hEventShutdown = CreateEvent(nullptr, TRUE, FALSE, nullptr);
		}

		if (INVALID_HANDLE_VALUE == m_hThreadRecv) {
			m_hThreadRecv = CreateThread(nullptr, 0, ThreadWorker, this, 0, nullptr);
		}*/
		
		/*if (INVALID_HANDLE_VALUE == m_hThreadLinkTest) {
			m_hThreadLinkTest = CreateThread(nullptr, 0, ThreadLinkTest, this, 0, nullptr);
		}*/

		return TRUE;
	} while (0);

	//Restart();

	return FALSE;
}

//
//void CClientService::Restart()
//{
//	AUTO_LOG_FUNCTION;
//	if (m_bShuttingDown)
//		return;
//
//	if (INVALID_HANDLE_VALUE == m_hEventShutdown) {
//		m_hEventShutdown = CreateEvent(nullptr, TRUE, FALSE, nullptr);
//	}
//	ResetEvent(m_hEventShutdown);
//	if (INVALID_HANDLE_VALUE == m_hThreadReconnectServer) {
//		m_hThreadReconnectServer = CreateThread(nullptr, 0, ThreadReconnectServer, this, 0, nullptr);
//	}
//}
//
//DWORD WINAPI CClientService::ThreadReconnectServer(LPVOID lp)
//{
//	AUTO_LOG_FUNCTION;
//	CClientService* service = reinterpret_cast<CClientService*>(lp);
//	for (;;) {
//		if (WAIT_OBJECT_0 == WaitForSingleObject(service->m_hEventShutdown, 0))
//			break;
//		if (service->Connect()) {
//			break;
//		} 
//		if (WAIT_OBJECT_0 == WaitForSingleObject(service->m_hEventShutdown, 10000))
//			break;
//	}
//	CLOSEHANDLE(service->m_hThreadReconnectServer);
//	return 0;
//}
//

void CClientService::Stop()
{
	AUTO_LOG_FUNCTION;
	m_bShuttingDown = TRUE;

	if (INVALID_HANDLE_VALUE != m_hEventShutdown) {
		SetEvent(m_hEventShutdown);
	}

	/*if (INVALID_HANDLE_VALUE != m_hThreadReconnectServer) {
		WaitForSingleObject(m_hThreadReconnectServer, INFINITE);
		CLOSEHANDLE(m_hThreadReconnectServer);
	}

	if (INVALID_HANDLE_VALUE != m_hThreadLinkTest) {
		WaitForSingleObject(m_hThreadLinkTest, INFINITE);
		CLOSEHANDLE(m_hThreadLinkTest);
	}
		*/
	if (INVALID_HANDLE_VALUE != m_hThreadRecv) {
		WaitForSingleObject(m_hThreadRecv, INFINITE);
		CLOSEHANDLE(m_hThreadRecv);
	}

	CLOSEHANDLE(m_hEventShutdown);
	
	Disconnect();
}

void CClientService::Disconnect()
{
	AUTO_LOG_FUNCTION;
	if (m_bConnectionEstablished) {
		//if (INVALID_HANDLE_VALUE != m_hEventShutdown) {
		//	SetEvent(m_hEventShutdown);
		//}
		//CLOSEHANDLE(m_hThreadLinkTest);
		//CLOSEHANDLE(m_hThreadRecv);
		shutdown(m_socket, 2);
		closesocket(m_socket);
		m_socket = INVALID_SOCKET;
		last_recv_time_.SetStatus(COleDateTime::error);
		m_bConnectionEstablished = FALSE;
		
		if (m_handler) {
			m_handler->OnConnectionLost(this);
		}
	}
}


void CClientService::PrepairToSend(int /*ademco_id*/, const char* buff, size_t buff_size)
{
	std::lock_guard<std::mutex> lock(buffer_lock_);
	std::vector<char> buffer;
	buffer.reserve(buff_size);
	buffer.assign(buff, buff + buff_size);
	buffer_.push_back(buffer);
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
		nRet = select(m_socket + 1, nullptr, &fdWrite, nullptr, &tv);
		if (WAIT_OBJECT_0 == WaitForSingleObject(m_hEventShutdown, 0))
			break;
	} while (nRet <= 0 && !FD_ISSET(m_socket, &fdWrite));

	nRet = send(m_socket, buff, buff_size, 0);

	if (nRet <= 0) {
		CLog::WriteLog(_T("CClientService::Send ret <= 0, ret %d"), nRet);
		Disconnect();
	}

	return nRet;
}

//
//DWORD WINAPI CClientService::ThreadLinkTest(LPVOID lp)
//{
//	AUTO_LOG_FUNCTION;
//	CClientService* service = reinterpret_cast<CClientService*>(lp);
//	DWORD dwLastTimeSendLinkTest = 0;
//	for (;;) {
//		if (WAIT_OBJECT_0 == WaitForSingleObject(service->m_hEventShutdown, 1000))
//			break;
//		if (service->m_handler && service->m_bConnectionEstablished && GetTickCount() - dwLastTimeSendLinkTest >= LINK_TEST_GAP) {
//			dwLastTimeSendLinkTest = GetTickCount();
//			char buff[4096] = { 0 };
//			DWORD dwLen = service->m_handler->GenerateLinkTestPackage(buff, sizeof(buff));
//			if (dwLen > 0 && dwLen <= sizeof(buff)) {
//				int nLen = service->Send(buff, dwLen);
//				if (nLen <= 0) {
//					CLog::WriteLog(_T("ThreadLinkTest::Send ret <= 0, ret %d"), nLen);
//					service->Release();
//					break;
//				}
//#ifdef _DEBUG
//				DWORD dwThreadID = GetCurrentThreadId();
//				CLog::WriteLog(_T("CClientService::ThreadLinkTest id %d is running.\n"), dwThreadID);
//#endif
//				CLog::WriteLog(_T("Send link test to transmite server, len %d\n"), nLen);
//			}
//		}
//	}
//	return 0;
//}


DWORD WINAPI CClientService::ThreadWorker(LPVOID lp)
{
	AUTO_LOG_FUNCTION;
	CClientService* service = reinterpret_cast<CClientService*>(lp);
	timeval tv = { 0, 10 };
	DWORD dwLastTimeSendLinkTest = 0;
	DWORD dwCount = 0;
	for (;;) {
		dwCount++;
		if (WAIT_OBJECT_0 == WaitForSingleObject(service->m_hEventShutdown, 1))
			break;

		if (!service->m_bConnectionEstablished) {
			service->Connect();
		}

		if (!service->m_bConnectionEstablished) {
			continue;
		}

		if (service->m_bShuttingDown) {
			break;
		}

		int nRet = 0;
		fd_set fdRead, fdWrite;

		FD_ZERO(&fdRead);
		FD_ZERO(&fdWrite);
		FD_SET(service->m_socket, &fdRead);
		FD_SET(service->m_socket, &fdWrite);
		nRet = select(service->m_socket + 1, &fdRead, &fdWrite, nullptr, &tv);
		if (WAIT_OBJECT_0 == WaitForSingleObject(service->m_hEventShutdown, 1))
			continue;
		
		BOOL bRead = FD_ISSET(service->m_socket, &fdRead);
		BOOL bWrite = FD_ISSET(service->m_socket, &fdWrite);
		if (WAIT_OBJECT_0 == WaitForSingleObject(service->m_hEventShutdown, 0))
			break;

		if (bRead) {
			char* temp = service->m_buff.buff + service->m_buff.wpos;
			DWORD dwLenToRead = BUFF_SIZE - service->m_buff.wpos;
			nRet = recv(service->m_socket, temp, dwLenToRead, 0);

			if (nRet <= 0) {
				CLog::WriteLog(_T("ThreadRecv::recv ret <= 0, ret %d"), nRet);
				service->Disconnect();
				continue;
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
				service->last_recv_time_ = COleDateTime::GetTickCount();
			}
		}

		if (bWrite) {
			if (service->m_handler && service->m_bConnectionEstablished && GetTickCount() - dwLastTimeSendLinkTest >= LINK_TEST_GAP) {
				dwLastTimeSendLinkTest = GetTickCount();
				char buff[4096] = { 0 };
				DWORD dwLen = service->m_handler->GenerateLinkTestPackage(buff, sizeof(buff));
				if (dwLen > 0 && dwLen <= sizeof(buff)) {
					int nLen = service->Send(buff, dwLen);
					if (nLen <= 0) {
						CLog::WriteLog(_T("ThreadLinkTest::Send ret <= 0, ret %d"), nLen);
						service->Disconnect();
						continue;
					}
#ifdef _DEBUG
					DWORD dwThreadID = GetCurrentThreadId();
					CLog::WriteLog(_T("CClientService::ThreadLinkTest id %d is running.\n"), dwThreadID);
#endif
					CLog::WriteLog(_T("Send link test to transmite server, len %d\n"), nLen);
				}
			}

			// send data
			//auto mgr = core::CAlarmMachineManager::GetInstance();
			std::lock_guard<std::mutex> lock(service->buffer_lock_);
			for (auto buffer : service->buffer_) {
				/*int ademco_id = iter.first;
				bool pass = true;
				if (ademco_id != -1) {
					auto machine = mgr->GetMachine(ademco_id);
					if (machine && !machine->get_sms_mode()) {
						pass = false;
					}
				}
				if (pass) {*/
					//auto buffer = iter;
					if (service->Send(&buffer[0], buffer.size()) < 0) break;
				//}
			}
			service->buffer_.clear();
		}

		// check timeup
		if (dwCount % 1000 == 0) {
			dwCount = 0;
			if ((COleDateTime::GetTickCount() - service->last_recv_time_).GetTotalSeconds() * 1000 > LINK_TEST_GAP * 3) {
				service->Disconnect();
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
	CMyClientEventHandler() : m_conn_id(0xFFFFFFFF){}
	virtual ~CMyClientEventHandler() {}
	virtual void OnConnectionEstablished(CClientService* service)
	{
		AUTO_LOG_FUNCTION;
		//std::lock_guard<std::mutex> lock(_mutex);
		CWinApp* app = AfxGetApp();
		if (app) {
			CWnd* wnd = app->GetMainWnd();
			if (wnd) {
				wnd->PostMessageW(WM_NETWORKSTARTUPOK, 1, service->main_client());
			}
		}
	}

	virtual void OnConnectionLost(CClientService* service)
	{
		AUTO_LOG_FUNCTION;
		//std::lock_guard<std::mutex> lock(_mutex);
		int conn_id = 0;
		for (auto client : m_clients) {
			if (client.online) {
				HandleOffline(conn_id);
			}
			conn_id++;
		}
		//service->Restart();
		CWinApp* app = AfxGetApp();
		if (app) {
			CWnd* wnd = app->GetMainWnd();
			if (wnd) {
				wnd->PostMessageW(WM_NETWORKSTARTUPOK, 0, service->main_client());
			}
		}
		m_conn_id = 0xFFFFFFFF;
	}

	virtual DWORD OnRecv(CClientService* service);
	virtual DWORD GenerateLinkTestPackage(char* buff, size_t buff_len);
	DEAL_CMD_RET DealCmd();
	inline int GetConnID() const
	{
		return m_conn_id;
	}

protected:
	// 2015-12-26 16:57:34 修复分段处理时假如剩余未处理的刚好为私有包，
	// 则优先以ademco包处理出错，导致该数据包被清空，从而导致漏掉事件的bug
	DWORD OnRecv2(CClientService* service);
	void HandleOffline(int conn_id) {
		core::CAlarmMachineManager* mgr = core::CAlarmMachineManager::GetInstance();
		if (m_clients[conn_id].online) {
			mgr->MachineOnline(ES_TCP_SERVER, m_clients[conn_id].ademco_id, FALSE);
			m_clients[conn_id].online = false;
			core::CAlarmMachinePtr machine = mgr->GetMachine(m_clients[conn_id].ademco_id);
			if (machine) {
				machine->SetPrivatePacket(nullptr);
			}
			m_clients[conn_id].ademco_id = -1;
		}
	}
private:
	DWORD m_conn_id;
	typedef struct _CLIENT_DATA
	{
		bool online;
		//int conn_id;
		int ademco_id;
		_CLIENT_DATA() : online(false), /*conn_id(-1), */ademco_id(-1) {}
	}CLIENT_DATA;

	CLIENT_DATA m_clients[core::MAX_MACHINE];

	AdemcoPacket m_packet1;
	PrivatePacket m_packet2;
	//std::mutex _mutex;
};



BOOL CClient::Start(const std::string& server_ip, unsigned int server_port)
{
	AUTO_LOG_FUNCTION;
	if (m_bClientServiceStarted)
		return TRUE;

	try {
		if (nullptr == _client_service) {
			_client_service = std::make_shared<CClientService>(main_client_);
		}
		
		if (nullptr == _client_event_handler) {
			_client_event_handler = std::make_shared<CMyClientEventHandler>();
		}

		_client_service->SetEventHandler(_client_event_handler);
		_client_service->Start(server_ip, server_port);

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
	if (nullptr != _client_service) {
		_client_service->Stop();
		_client_service = nullptr;
	}

	if (nullptr != _client_event_handler) {
		_client_event_handler = nullptr;
	}

	m_bClientServiceStarted = FALSE;
}


int CClient::SendToTransmitServer(int ademco_id, ADEMCO_EVENT ademco_event, int gg, 
								  int zone, const ademco::char_array_ptr& xdata)
{
	AUTO_LOG_FUNCTION;
	if (_client_service) {
		char data[BUFF_SIZE] = { 0 };
		core::CAlarmMachinePtr machine = core::CAlarmMachineManager::GetInstance()->GetMachine(ademco_id);
		if (machine) {
			static AdemcoPacket packet;
			const PrivatePacketPtr privatePacket = machine->GetPrivatePacket();
			if (!privatePacket)
				return 0;
			DWORD dwSize = 0;
			dwSize = packet.Make(data, sizeof(data), AID_HB, 0,
								 //machine->GetDeviceIDA(),
								 //privatePacket->_acct_machine,
								 //nullptr,
								 ademco::HexCharArrayToStr(privatePacket->_acct_machine, 9),
								 ademco_id, ademco_event,
								 gg, zone, xdata);
			//ConnID conn_id = g_client_event_handler->GetConnID();
			char_array cmd;
			//cmd.AppendConnID(privatePacket->_cmd.GetConnID());
			AppendConnIdToCharArray(cmd, GetConnIdFromCharArray(privatePacket->_cmd));
			static PrivatePacket packet2;
			dwSize += packet2.Make(data + dwSize, sizeof(data)-dwSize, 0x0c, 0x00, cmd,
								   privatePacket->_acct_machine,
								   privatePacket->_passwd_machine,
								   privatePacket->_acct,
								   privatePacket->_level);
			_client_service->PrepairToSend(ademco_id, data, dwSize);
			return 1;
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
	DWORD dwLen = m_packet1.Make(buff, buff_len, AID_NULL, seq++, nullptr, 0, 0, 0, 0);
	ConnID conn_id = m_conn_id;
	char_array cmd;
	//cmd.AppendConnID(privatePacket->_cmd.GetConnID());
	AppendConnIdToCharArray(cmd, conn_id);
	static PrivatePacket packet2;
	char acct[9] = { 0 };
	NumStr2HexCharArray_N(util::CConfigHelper::GetInstance()->get_csr_acct().c_str(), acct);
	dwLen += m_packet2.Make(buff + dwLen, buff_len - dwLen, 0x06, 0x00, cmd, nullptr, nullptr, 
							acct, 0);
	return dwLen;
}

DWORD CMyClientEventHandler::OnRecv(CClientService* service)
{
	AUTO_LOG_FUNCTION;
	//std::lock_guard<std::mutex> lock(_mutex);
	size_t dwBytesCmted = 0;
	ParseResult result1 = m_packet1.Parse(service->m_buff.buff + service->m_buff.rpos,
										  service->m_buff.wpos - service->m_buff.rpos,
										  dwBytesCmted);

	if (result1 == RESULT_OK) {
		service->m_buff.rpos = (service->m_buff.rpos + dwBytesCmted);
		return OnRecv2(service);
	} else if (result1 == RESULT_NOT_ENOUGH) {
		return RESULT_NOT_ENOUGH;
	} else if (result1 == RESULT_DATA_ERROR) {
		// 2015-12-26 17:00:02 这个时候有可能是缓冲区满，移动了内存，
		// 缓冲区起始位置刚好为私有数据包，不能丢弃，要处理
		DWORD result2 =  OnRecv2(service);
		if (result2 != RESULT_OK) {
			service->m_buff.Clear();
			return RESULT_OK;
		}
	} else {
		ASSERT(0);
		service->m_buff.Clear();
	}
	return RESULT_OK;
}


DWORD CMyClientEventHandler::OnRecv2(CClientService* service)
{
	size_t dwBytesCmted = 0;
	ParseResult result2 = m_packet2.Parse(service->m_buff.buff + service->m_buff.rpos,
										  service->m_buff.wpos - service->m_buff.rpos,
										  dwBytesCmted);
	if (RESULT_DATA_ERROR == result2) {
		ASSERT(0);
		service->m_buff.Clear();
		return RESULT_OK;
	} else if (RESULT_NOT_ENOUGH == result2) {
		return RESULT_NOT_ENOUGH;
	} else {
		service->m_buff.rpos = (service->m_buff.rpos + dwBytesCmted);
		
		char buff[1024] = { 0 };
		DEAL_CMD_RET dcr = DealCmd();
		
		if (ademco::is_same_id(m_packet1._id, ademco::AID_NAK)) {
			CString record = _T("");
			record = GetStringFromAppResource(IDS_STRING_ILLEGAL_OP);
			core::CHistoryRecord::GetInstance()->InsertRecord(m_packet1._ademco_data._ademco_id, 0, record,
															  m_packet1._timestamp._time, core::RECORD_LEVEL_STATUS);
		}

		char _seq[4];
		std::copy(m_packet1._seq.begin(), m_packet1._seq.end(), _seq);
		int seq = ademco::NumStr2Dec(_seq, 4);
		if (seq > 9999) seq = 1;
		
		if (dcr == DCR_ONLINE) {
			auto csr_acct = util::CConfigHelper::GetInstance()->get_csr_acct();
			if (!csr_acct.empty() && csr_acct.length() <= 18) {
				size_t len = m_packet1.Make(buff, sizeof(buff), AID_HB, 0, nullptr,
											m_packet1._ademco_data._ademco_id, 0, 0, 0);
				char_array cmd;
				AppendConnIdToCharArray(cmd, ConnID(m_conn_id));
				char temp[9] = { 0 };
				NumStr2HexCharArray_N(csr_acct.c_str(), temp, 9);
				for (auto t : temp) { cmd.push_back(t); }
				len += m_packet2.Make(buff + len, sizeof(buff) - len, 0x06, 0x01, cmd,
									  m_packet2._acct_machine,
									  m_packet2._passwd_machine,
									  m_packet2._acct,
									  m_packet2._level);
				service->PrepairToSend(-1, buff, len);
			}
		} else if (dcr == DCR_ACK) {
			size_t len = m_packet1.Make(buff, sizeof(buff), AID_ACK, seq,
									    /*acct, packet2._acct_machine, */
									    ademco::HexCharArrayToStr(m_packet2._acct_machine, 9),
										m_packet1._ademco_data._ademco_id, 0, 0, 0);
			char_array cmd;
			AppendConnIdToCharArray(cmd, GetConnIdFromCharArray(m_packet2._cmd));
			char csr_acct[9] = { 0 };
			auto acct = util::CConfigHelper::GetInstance()->get_csr_acct();
			ademco::NumStr2HexCharArray_N(acct.c_str(), csr_acct, 9);
			len += m_packet2.Make(buff + len, sizeof(buff) - len, 0x0c, 0x01, cmd,
								  m_packet2._acct_machine,
								  m_packet2._passwd_machine,
								  csr_acct,
								  m_packet2._level);
			service->PrepairToSend(m_packet1._ademco_data._ademco_id, buff, len);
		} else if (dcr == DCR_NAK) {
			size_t len = m_packet1.Make(buff, sizeof(buff), AID_NAK, seq,
									  /*acct, packet2._acct_machine, */
									  ademco::HexCharArrayToStr(m_packet2._acct_machine, 9),
										m_packet1._ademco_data._ademco_id, 0, 0, 0);
			char_array cmd;
			AppendConnIdToCharArray(cmd, GetConnIdFromCharArray(m_packet2._cmd));
			len += m_packet2.Make(buff + len, sizeof(buff) - len, 0x0c, 0x01, cmd,
								  m_packet2._acct_machine,
								  m_packet2._passwd_machine,
								  m_packet2._acct,
								  m_packet2._level);
			service->PrepairToSend(m_packet1._ademco_data._ademco_id, buff, len);
		}
		return RESULT_OK;
	}
}


CMyClientEventHandler::DEAL_CMD_RET CMyClientEventHandler::DealCmd()
{
	AUTO_LOG_FUNCTION;
	//const PrivateCmd* private_cmd = &m_packet2._cmd;
	//int private_cmd_len = private_cmd->_size;
	//return DCR_NULL;
	//if (private_cmd_len != 3 && private_cmd_len != 32)
	//	return DCR_NULL;

	DWORD conn_id = GetConnIdFromCharArray(m_packet2._cmd).ToInt();
	core::CAlarmMachineManager* mgr = core::CAlarmMachineManager::GetInstance(); ASSERT(mgr);
	if (m_packet2._big_type == 0x02) {
		if (m_packet2._lit_type == 0x06) {
			int ademco_id = m_packet1._ademco_data._ademco_id;
			if (m_packet2._cmd.size() >= 4) {
				bool sms_mode = m_packet2._cmd[3] == 0 ? false : true;
				auto machine = mgr->GetMachine(ademco_id);
				if (machine && sms_mode != machine->get_sms_mode()) {
					machine->set_sms_mode(sms_mode);
					CString txt;
					txt.Format(L"%s(%06d,%s) ", GetStringFromAppResource(IDS_STRING_MACHINE), ademco_id, machine->get_alias());
					if (sms_mode) {
						txt += GetStringFromAppResource(IDS_STRING_ENTER_SMS_MODE);
					} else {
						txt += GetStringFromAppResource(IDS_STRING_LEAVE_SMS_MODE);
					}
					core::CHistoryRecord::GetInstance()->InsertRecord(ademco_id, 0, txt, time(nullptr), core::RECORD_LEVEL_STATUS);
				}
			}
		}
	} else if (m_packet2._big_type == 0x05) { // from machine
		if (m_packet2._lit_type == 0x00) {
			try {
				int ademco_id = m_packet1._ademco_data._ademco_id;
				int zone = m_packet1._ademco_data._zone;
				int subzone = m_packet1._ademco_data._gg;
				ADEMCO_EVENT ademco_event = m_packet1._ademco_data._ademco_event;
				BOOL ok = TRUE;
				do {
					if (!m_clients[conn_id].online) {
						char acct[64] = { 0 };
						std::copy(m_packet1._acct.begin(), m_packet1._acct.end(), acct);
						CLog::WriteLogA("alarm machine: 0d 00 aid %04d acct %s online.\n",
										ademco_id, acct);
						if (!mgr->CheckIsValidMachine(ademco_id, /*acct, */zone)) {
							ok = FALSE; break;
						}

						core::CAlarmMachinePtr machine = mgr->GetMachine(ademco_id);
						if (machine) {
							machine->SetPrivatePacket(&m_packet2);
						}

						m_clients[conn_id].online = true;
						m_clients[conn_id].ademco_id = ademco_id;
						mgr->MachineOnline(ES_TCP_SERVER, ademco_id);

					}

					mgr->MachineEventHandler(ES_TCP_SERVER, ademco_id, ademco_event, zone,
											 subzone, m_packet1._timestamp._time, time(nullptr),
											 m_packet1._xdata);
				} while (0);

				if (!ok) {
					CString fm, rec;
					fm = GetStringFromAppResource(IDS_STRING_FM_KICKOUT_INVALID);
					rec.Format(fm, ademco_id/*, A2W(client->acct)*/);
					core::CHistoryRecord* hr = core::CHistoryRecord::GetInstance();
					hr->InsertRecord(ademco_id, zone, rec, time(nullptr), core::RECORD_LEVEL_STATUS);
					JLOG(rec);
					CLog::WriteLog(_T("Check acct-aid failed, pass.\n"));
				}

				return ok ? DCR_ACK : DCR_NAK;
				
			} catch (...) {
				return DCR_NAK;
			}
		}
	} else if (m_packet2._big_type == 0x07) {			// from Transmit server
		//return DCR_NULL;
		switch (m_packet2._lit_type) {
			case 0x00:	// link test responce
				CLog::WriteLog(_T("Transmite server link test responce\n"));
				break;
			case 0x01:	// conn_id
				m_conn_id = conn_id; 
				CLog::WriteLog(_T("Transmite server responce my conn_id %d\n"), conn_id);
				return DCR_ONLINE;
				break;
			case 0x02:	// alarm machine connection lost
				if (conn_id < 0 || conn_id >= core::MAX_MACHINE) {
					return DCR_NULL;
				}
				if (m_clients[conn_id].online) {
					HandleOffline(conn_id);
				}
				break;
			default:
				break;
		}
	} else if (m_packet2._big_type == 0x0d) {	// from Alarm Machine
		int ademco_id = m_packet1._ademco_data._ademco_id;		
		int zone = m_packet1._ademco_data._zone;
		int subzone = m_packet1._ademco_data._gg;
		ADEMCO_EVENT ademco_event = m_packet1._ademco_data._ademco_event;

		if (m_packet2._lit_type == 0x00) {	// Alarm machine on/off line, event report.
			JLOGA("alarm machine EVENT:0d 00 aid %04d event %04d zone %03d %s\n",
				  ademco_id, ademco_event, zone, m_packet1._timestamp._data);

			BOOL ok = TRUE;
			do {
				if (!m_clients[conn_id].online) {
					char acct[64] = { 0 };
					std::copy(m_packet1._acct.begin(), m_packet1._acct.end(), acct);
					CLog::WriteLogA("alarm machine ONLINE:0d 00 aid %04d acct %s online.\n",
									ademco_id, acct);
					if (!mgr->CheckIsValidMachine(ademco_id, /*acct, */zone)) {
						ok = FALSE; break;
					}
					 
					core::CAlarmMachinePtr machine = mgr->GetMachine(ademco_id);
					if (machine) {
						machine->SetPrivatePacket(&m_packet2);
					}

					m_clients[conn_id].online = true;
					m_clients[conn_id].ademco_id = ademco_id;
					mgr->MachineOnline(ES_TCP_SERVER, ademco_id);
					
				}
				
				mgr->MachineEventHandler(ES_TCP_SERVER, ademco_id, ademco_event, zone,
										 subzone, m_packet1._timestamp._time, time(nullptr),
										 m_packet1._xdata);
			} while (0);

			if (!ok) {
				CString fm, rec;
				fm = GetStringFromAppResource(IDS_STRING_FM_KICKOUT_INVALID);
				rec.Format(fm, ademco_id/*, A2W(client->acct)*/);
				core::CHistoryRecord* hr = core::CHistoryRecord::GetInstance();
				hr->InsertRecord(ademco_id, zone, rec, time(nullptr), core::RECORD_LEVEL_STATUS);
				JLOG(rec);
				CLog::WriteLog(_T("Check acct-aid failed, pass.\n"));
			}

			return ok ? DCR_ACK : DCR_NAK;
		} else if (m_packet2._lit_type == 0x01) {
			// 2014年11月26日 17:02:23 
			
			try {
				if (m_clients[conn_id].online) {
					ademco_id = m_clients[conn_id].ademco_id;
					JLOGA("alarm machine EVENT:0d 01 aid %04d event %04d zone %03d %s\n",
						  ademco_id, ademco_event, zone, m_packet1._timestamp._data);
					auto cmd = m_packet2._cmd;
					static ADEMCO_EVENT cStatus[] = { EVENT_ARM, EVENT_HALFARM, EVENT_DISARM, EVENT_DISARM };
					char machine_status = cmd[6];
					//char phone_num = cmd[7];
					//char alarming_num = cmd[8 + 3 * phone_num];
					if (machine_status < 4) {
						mgr->MachineEventHandler(ES_TCP_SERVER, ademco_id, cStatus[machine_status], zone,
												 subzone, m_packet1._timestamp._time, time(nullptr),
												 m_packet1._xdata);
					} 
					return DCR_ACK;
				} else {
					return DCR_NAK;
				}
			} catch (...) {
				return DCR_NAK;
			}
			
		}
	} 

	return DCR_NULL;
}

NAMESPACE_END
NAMESPACE_END
