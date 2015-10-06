#include "stdafx.h"
#include "ServerService.h"
#include <assert.h>
#include <WinSock2.h>
#pragma comment(lib, "ws2_32.lib")
#include "ademco_func.h"
//#define KICKOUT_CLIENT_IF_RECV_OR_SEND_RESULT_EQUALS_TO_0
//#define KICKOUT_CLIENT_IF_RECV_OR_SEND_RESULT_LESS_THAN_0


namespace net {
namespace server {

static const int NUM_BUFFERED_CLIENTS = 1000;


CServerService::CServerService()
{}


CServerService::~CServerService()
{
	Stop();
	::DeleteCriticalSection(&m_cs4liveingClients);
	::DeleteCriticalSection(&m_cs4outstandingClients);

	for (auto client : m_bufferedClients) {
		delete client;
	}

	for (auto iter : m_livingClients) {
		delete iter.second;
	}

	for (auto client : m_outstandingClients) {
		delete client;
	}
}


bool set_timeout(SOCKET s, int miliseconds) 
{
	AUTO_LOG_FUNCTION;
	// set recv/send timeout
	int timeout = 0;
	int optlen = sizeof(struct timeval);
	int ret = 0;

	LOGA("sys info:\n");
	ret = getsockopt(s, SOL_SOCKET, SO_RCVTIMEO, (char *)&timeout, &optlen);
	if (ret == 0) {
		LOGA("SO_RCVTIMEO: %d\n", timeout);
	} else {
		LOGA("get SO_RCVTIMEO failed\n");
		return false;
	}

	ret = getsockopt(s, SOL_SOCKET, SO_SNDTIMEO, (char *)&timeout, &optlen);
	if (ret == 0) {
		LOGA("SO_SNDTIMEO: %d\n", timeout);
	} else {
		LOGA("get SO_SNDTIMEO failed\n");
		return false;
	}

	timeout = miliseconds;
	LOGA("user set timeout to %dms:\n", timeout);
	optlen = sizeof(struct timeval);
	ret = setsockopt(s, SOL_SOCKET, SO_RCVTIMEO, (char *)&timeout, optlen);
	if (ret == 0) {
		LOGA("set SO_RCVTIMEO ok\n");
	} else {
		LOGA("set SO_RCVTIMEO failed\n");
		return false;
	}

	ret = setsockopt(s, SOL_SOCKET, SO_SNDTIMEO, (char *)&timeout, optlen);
	if (ret == 0) {
		LOGA("set SO_SNDTIMEO ok\n");
	} else {
		LOGA("set SO_SNDTIMEO failed\n");
		return false;
	}
	LOGA("set ok\n");

	LOGA("sys info:\n");
	ret = getsockopt(s, SOL_SOCKET, SO_RCVTIMEO, (char *)&timeout, &optlen);
	if (ret == 0) {
		LOGA("SO_RCVTIMEO: %d\n", timeout);
	} else {
		LOGA("get SO_RCVTIMEO failed\n");
		return false;
	}

	ret = getsockopt(s, SOL_SOCKET, SO_SNDTIMEO, (char *)&timeout, &optlen);
	if (ret == 0) {
		LOGA("SO_SNDTIMEO: %d\n", timeout);
	} else {
		LOGA("get SO_SNDTIMEO failed\n");
		return false;
	}

	return true;
}


CServerService::CServerService(unsigned short& nPort, unsigned int nMaxClients,
							   unsigned int nTimeoutVal,
							   bool blnCreateAsync, bool blnBindLocal)
							   : m_ServSock(INVALID_SOCKET)
							   , m_ShutdownEvent(INVALID_HANDLE_VALUE)
							   , m_phThreadAccept(nullptr)
							   , m_phThreadRecv(nullptr)
							   //, m_nLiveConnections(0)
							   , m_handler(nullptr)
							   , m_nMaxClients(nMaxClients)
							   , m_nTimeoutVal(nTimeoutVal)
{
	int nRet = -1;
	unsigned long lngMode = 0;
	struct sockaddr_in sAddrIn;
	memset(&sAddrIn, 0, sizeof(sAddrIn));

	// Create the server socket, set the necessary 
	// parameters for making it IOCP compatible.
	m_ServSock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);// , nullptr, 0, WSA_FLAG_OVERLAPPED);
	if (INVALID_SOCKET == this->m_ServSock) {
		//throw L"server socket creation failed.";
		int errnono = WSAGetLastError();
		wchar_t *err = FormatWSAError(errnono);
		throw err;
	}

	set_timeout(m_ServSock, 5000);

	// Make server socket Synchronous (blocking) 
	// or Asynchronous (non-blocking) 
	if (blnCreateAsync) {
		nRet = ioctlsocket(this->m_ServSock, FIONBIO, (u_long FAR*) &lngMode);
		if (nRet < 0) {
			closesocket(this->m_ServSock);
			throw "server socket creation failed.";
		}
	}

	// Fill the structure for binding operation
	sAddrIn.sin_family = AF_INET;
	sAddrIn.sin_port = htons(nPort);

	// Bind to the localhost ("127.0.0.1") to accept connections only from
	// localhost or 
	// "0.0.0.0" (INADDR_ANY) to accept connections from any IP address.
	if (blnBindLocal) sAddrIn.sin_addr.s_addr = inet_addr("127.0.0.1");
	else sAddrIn.sin_addr.s_addr = htonl(INADDR_ANY);

	// Bind the structure to the created server socket.
	while (bind(this->m_ServSock, (struct sockaddr *) &sAddrIn, sizeof(sAddrIn)) < 0) {
		//closesocket(this->m_ServSock);
		LOG(L"server socket failed to bind on port %d, now try port %d.", nPort, nPort+1);
		sAddrIn.sin_port = htons(++nPort);
	}

	// Set server socket in listen mode and set the listen queue to 20.
	nRet = listen(this->m_ServSock, SOMAXCONN);
	if (nRet < 0) {
		closesocket(this->m_ServSock);
		throw "server scket failed to listen.";
	}

	::InitializeCriticalSection(&m_cs4liveingClients);
	::InitializeCriticalSection(&m_cs4outstandingClients);

	for (size_t i = 0; i < NUM_BUFFERED_CLIENTS; i++) {
		CClientData* data = new CClientData();
		m_bufferedClients.push_back(data);
	}
}


void CServerService::Start()
{
	if (INVALID_HANDLE_VALUE == m_ShutdownEvent) {
		m_ShutdownEvent = CreateEvent(nullptr, TRUE, FALSE, nullptr);
	}

	if (nullptr == m_phThreadAccept) {
		m_phThreadAccept = new HANDLE[THREAD_ACCEPT_NO];
		for (int i = 0; i < THREAD_ACCEPT_NO; i++) {
			m_phThreadAccept[i] = CreateThread(nullptr, 0, ThreadAccept, this, 0, nullptr);
		}
	}

	if (nullptr == m_phThreadRecv) {
		m_phThreadRecv = new HANDLE[THREAD_RECV_NO];
		for (int i = 0; i < THREAD_RECV_NO; i++) {
			THREAD_PARAM* param = new THREAD_PARAM();
			param->service = this;
			param->thread_no = i;
			m_phThreadRecv[i] = CreateThread(nullptr, 0, ThreadRecv, param, CREATE_SUSPENDED, nullptr);
			SetThreadPriority(m_phThreadRecv[i], THREAD_PRIORITY_ABOVE_NORMAL);
			ResumeThread(m_phThreadRecv[i]);
		}
	}

	if (m_handler) {
		m_handler->Start();
	}

	CLog::WriteLog(L"ServerService started successfully");
}


void CServerService::Stop()
{
	AUTO_LOG_FUNCTION;
	if (m_handler) {
		m_handler->Stop();
	}

	if (INVALID_HANDLE_VALUE != m_ShutdownEvent) {
		SetEvent(m_ShutdownEvent);

		if (nullptr != m_phThreadAccept) {
			WaitForMultipleObjects(THREAD_ACCEPT_NO, m_phThreadAccept, TRUE, INFINITE);
			for (int i = 0; i < THREAD_ACCEPT_NO; i++) {
				CloseHandle(m_phThreadAccept[i]);
			}
			delete[] m_phThreadAccept;
			m_phThreadAccept = nullptr;
		}

		if (nullptr != m_phThreadRecv) {
			WaitForMultipleObjects(THREAD_RECV_NO, m_phThreadRecv, TRUE, INFINITE);
			for (int i = 0; i < THREAD_RECV_NO; i++) {
				CloseHandle(m_phThreadRecv[i]);
			}
			delete[] m_phThreadRecv;
			m_phThreadRecv = nullptr;
		}

		CloseHandle(m_ShutdownEvent);
		m_ShutdownEvent = INVALID_HANDLE_VALUE;
	}
}


DWORD WINAPI CServerService::ThreadAccept(LPVOID lParam)
{
	AUTO_LOG_FUNCTION;
	CServerService *server = static_cast<CServerService*>(lParam);
	timeval timeout = { 1, 0 };
	
	while (1) {
		if (WAIT_OBJECT_0 == WaitForSingleObject(server->m_ShutdownEvent, 1))
			break;

		struct sockaddr_in sForeignAddIn;
		int nLength = sizeof(struct sockaddr_in);
		fd_set rfd;
		int nfds;
		bool ok = false;
		
		while (1) {
			if (WAIT_OBJECT_0 == WaitForSingleObject(server->m_ShutdownEvent, 0))
				break;
			FD_ZERO(&rfd);
			FD_SET(server->m_ServSock, &rfd);
			nfds = select(1, &rfd, (fd_set*)0, (fd_set*)0, &timeout);
			if (nfds == 0)
				continue;
			else if (nfds > 0){
				FD_CLR(server->m_ServSock, &rfd);
				ok = true;
				break;
			} else
				break;
		}

		if (WAIT_OBJECT_0 == WaitForSingleObject(server->m_ShutdownEvent, 0))
			break;
		if (!ok)
			continue;

		SOCKET client = accept(server->m_ServSock, (struct sockaddr*) &sForeignAddIn, &nLength);
		if (client == INVALID_SOCKET)
			continue;
		LOG(L"got a new connection!++++++++++++++++++++++++++++++++++++++++++++++++++\n");
		if (server->m_livingClients.size() >= server->m_nMaxClients) {
			shutdown(client, 2);
			closesocket(client);
			LOG(L"LiveConnections %d ***************************\n", server->m_livingClients.size());
			LOG(L"LiveConnections >= m_nMaxClients %d.\n", server->m_nMaxClients);
			continue;
		}

		CLocalLock lock(&server->m_cs4outstandingClients);
		CClientData* data = server->AllocateClient();
		data->socket = client;
		data->ResetTime(false);
		memcpy(&data->foreignAddIn, &sForeignAddIn, sizeof(struct sockaddr_in));
		server->m_outstandingClients.push_back(data);
	}

	return 0;
}

DWORD WINAPI CServerService::ThreadRecv(LPVOID lParam)
{
	AUTO_LOG_FUNCTION;
	THREAD_PARAM* param = reinterpret_cast<THREAD_PARAM*>(lParam);
	CServerService *server = param->service; delete param;
	
	for (;;) {
		if (WAIT_OBJECT_0 == WaitForSingleObject(server->m_ShutdownEvent, 1))
			break;

		// handle outstanding clients
		{
			CLocalLock lock(&server->m_cs4outstandingClients);
			for (auto client : server->m_outstandingClients) {
				if (WAIT_OBJECT_0 == WaitForSingleObject(server->m_ShutdownEvent, 0))
					break;
				int ret = server->HandleClientEvents(client);
				if (ret == RESULT_CONTINUE) {
					continue;
				} else if (ret == RESULT_BREAK) {
					break;
				} else if (ret == RESULT_RECYCLE_AND_BREAK) {
					server->RecycleOutstandingClient(client);
					break;
				} else {
					assert(0); break;
				}
			}
		}

		if (WAIT_OBJECT_0 == WaitForSingleObject(server->m_ShutdownEvent, 0))
			break;

		// handle living clients
		{
			CLocalLock lock(&server->m_cs4liveingClients);
			for (auto iter : server->m_livingClients) {
				if (WAIT_OBJECT_0 == WaitForSingleObject(server->m_ShutdownEvent, 0))
					break;
				int ret = server->HandleClientEvents(iter.second);
				if (ret == RESULT_CONTINUE) {
					continue;
				} else if (ret == RESULT_BREAK) {
					break;
				} else if (ret == RESULT_RECYCLE_AND_BREAK) {
					server->RecycleLiveClient(iter.second, TRUE);
					break;
				} else {
					assert(0); break;
				}
			}
		}
	}

	return 0;
}


CServerService::HANDLE_EVENT_RESULT CServerService::HandleClientEvents(CClientData* client)
{
	timeval tv = { 0, 0 };
	fd_set fd_read, fd_write;
	if (!client->hangup) {
		long long lngTimeElapsed = client->GetTimeElapsed();
		if (0 < lngTimeElapsed && static_cast<long long>(m_nTimeoutVal) < lngTimeElapsed) {
			char buff[32] = { 0 };
			time_t last = client->tmLastActionTime;
			struct tm tmtm;
			localtime_s(&tmtm, &last);
			strftime(buff, 32, "%Y-%m-%d %H:%M:%S", &tmtm);
			CLog::WriteLogA("last action time %s", buff);
			time_t now = time(nullptr);
			localtime_s(&tmtm, &now);
			strftime(buff, 32, "%Y-%m-%d %H:%M:%S", &tmtm);
			CLog::WriteLogA("now %s", buff);
			CLog::WriteLog(L"lngTimeElapsed %ld, timeout %d",
							lngTimeElapsed, m_nTimeoutVal);
			CLog::WriteLog(L"client timeout, kick out. ademco_id %04d",
							client->ademco_id);
			return RESULT_RECYCLE_AND_BREAK;
		}
	}

	if (client->wait_to_kill) {
		LOG(L"wait_to_kill true.");
		return RESULT_RECYCLE_AND_BREAK;
	}

	if (client->disconnectd) {
		LOG(L"disconnectd true.");
		return RESULT_RECYCLE_AND_BREAK;
	}

	FD_ZERO(&fd_read);
	FD_ZERO(&fd_write);
	FD_SET(client->socket, &fd_read);
	FD_SET(client->socket, &fd_write);
	int ret = select(0, &fd_read, &fd_write, nullptr, &tv);
	if (ret <= 0)
		return RESULT_CONTINUE;
	BOOL bRead = FD_ISSET(client->socket, &fd_read);
	BOOL bWrite = FD_ISSET(client->socket, &fd_write);

	//handle recv
	if (bRead) {
		char* temp = client->buff.buff + client->buff.wpos;
		DWORD dwLenToRead = BUFF_SIZE - client->buff.wpos;
		int bytes_transfered = recv(client->socket, temp, dwLenToRead, 0);
#ifdef KICKOUT_CLIENT_IF_RECV_OR_SEND_RESULT_EQUALS_TO_0
		if (bytes_transfered == 0) {
			LOG(FormatWSAError(WSAGetLastError()));
			CLog::WriteLog(L"dwLenToRead %d recv %d bytes, kick out %04d, conn_id %d, continue",
							dwLenToRead,
							bytes_transfered,
							m_clients[i].ademco_id,
							m_clients[i].conn_id);
			Release(&m_clients[i]);
		}
		if (bytes_transfered < 0) {
#else
		if (bytes_transfered <= 0) {
#endif 
#ifdef KICKOUT_CLIENT_IF_RECV_OR_SEND_RESULT_LESS_THAN_0
			m_clients[i].disconnectd = true;
			LOG(FormatWSAError(WSAGetLastError()));
			CLog::WriteLog(L"dwLenToRead %d recv %d bytes, kick out %04d, conn_id %d, continue",
							dwLenToRead,
							bytes_transfered,
							m_clients[i].ademco_id,
							m_clients[i].conn_id);
			Release(&m_clients[i]);
#else
			client->disconnectd = true;
			LOG(FormatWSAError(WSAGetLastError()));
			CLog::WriteLog(L"dwLenToRead %d recv %d bytes, no kick out %04d, continue",
							dwLenToRead, bytes_transfered, client->ademco_id);
			return RESULT_CONTINUE;
#endif
		} else if (m_handler) {
			client->ResetTime(false);
			client->buff.wpos += bytes_transfered;
			DWORD ret = ademco::RESULT_OK;
			BOOL resolved = FALSE;
			ret = m_handler->OnRecv(this, client, resolved);
			while (1) {
				if (WAIT_OBJECT_0 == WaitForSingleObject(m_ShutdownEvent, 0)) break;
				unsigned int bytes_not_commited = client->buff.wpos - client->buff.rpos;
				if (bytes_not_commited == 0) { if (client->buff.wpos == BUFF_SIZE) { client->buff.Clear(); } break; }
				if (client->buff.wpos == BUFF_SIZE) {
					memmove_s(client->buff.buff, BUFF_SIZE, client->buff.buff + client->buff.rpos, bytes_not_commited);
					memset(client->buff.buff + bytes_not_commited, 0, BUFF_SIZE - bytes_not_commited);
					client->buff.wpos -= client->buff.rpos; client->buff.rpos = 0;
					ret = m_handler->OnRecv(this, client, resolved);
				} else { ret = m_handler->OnRecv(this, client, resolved); }
				if (ret == ademco::RESULT_NOT_ENOUGH) { break; }
			}
			if (resolved) { return RESULT_BREAK; }
		}
	}// handle recv

	// handle send
	if (bWrite) {
		Task* task = client->GetFirstTask();
		if (task) {
			bool bNeedSend = false;
			if (task->_last_send_time.m_dt == 0.0) {
				bNeedSend = true;
			}
			if (!bNeedSend) {
				COleDateTime now = COleDateTime::GetCurrentTime();
				COleDateTimeSpan span = now - task->_last_send_time;
				if (span.GetTotalSeconds() > 5) {
					bNeedSend = true;
				}
			}
			if (bNeedSend) {
				if (task->_retry_times > 10) {
					bNeedSend = false;
					client->RemoveFirstTask();
				}
			}
			if (bNeedSend) {
				LOG(L"++++++++++++++task list size %d, cur task seq %d, retry_times %d, ademco_id %d, event %d, gg %d, zone %d, xdata_len %d\n",
					client->taskList.size(), task->_seq, task->_retry_times, task->_ademco_id,
					task->_ademco_event, task->_gg, task->_zone, task->_xdata_len);
				if (task->_last_send_time.GetStatus() == COleDateTime::valid)
					task->_retry_times++;
				task->_last_send_time = COleDateTime::GetCurrentTime();
				ademco::AdemcoPacket packet;
				char data[1024] = { 0 };
				size_t data_len = packet.Make(data, 1024,
												ademco::AID_HB,
												task->_seq,
												/*m_clients[i].acct, */nullptr,
												task->_ademco_id,
												task->_ademco_event,
												task->_gg,
												task->_zone,
												task->_xdata,
												task->_xdata_len);
				SendToClient(client, data, data_len);
#ifndef ENABLE_SEQ_CONFIRM
				m_clients[i].RemoveFirstTask();
#endif
			}
		}
	}// handle send

	return RESULT_CONTINUE;
}


bool CServerService::SendToClient(CClientData* client, const char* data, size_t data_len)
{
	do {
		assert(client);
		if (client->socket == INVALID_SOCKET)
			break;
		int nRet = 0;
		nRet = send(client->socket, data, data_len, 0);
#ifdef KICKOUT_CLIENT_IF_RECV_OR_SEND_RESULT_EQUALS_TO_0
		if (nRet == 0){
			CLog::WriteLog(L"send %d bytes, kick out %04d, conn_id %d",
				nRet, client->ademco_id, client->conn_id);
			Release(client);
			break;
		} else if (nRet < 0) {
#else
		if (nRet <= 0) {
#endif
#ifdef KICKOUT_CLIENT_IF_RECV_OR_SEND_RESULT_LESS_THAN_0
			CLog::WriteLog(L"send %d bytes, kick out %04d, conn_id %d", 
						   nRet, client->ademco_id, client->conn_id);
			Release(client);
			break;
#else
			CLog::WriteLog(L"send %d bytes, no kick out %04d",
						   nRet, client->ademco_id);
			break;
#endif
		} else {
			client->ResetTime(false);
		}
		return true;
	} while (0);
	LOG(L"CServerService::SendToClient failed..++++++++++++++++\n");
	return false;
}


bool CServerService::SendToClient(int ademco_id, int ademco_event, int gg,
								  int zone, const char* xdata, int xdata_len)
{
	do {
		CClientData* client = nullptr;
		if (!FindClient(ademco_id, &client))
			break;
		if (client == nullptr)
			break;
		client->AddTask(new Task(ademco_id, ademco_event, gg, zone, xdata, xdata_len));
		return true;
	} while (0);
	return false;
}

bool CServerService::FindClient(int ademco_id, CClientData** client)
{
	AUTO_LOG_FUNCTION;
	CLocalLock lock(&m_cs4liveingClients);
	auto iter = m_livingClients.find(ademco_id);
	if (iter == m_livingClients.end()) {
		return false;
	} else {
		*client = iter->second;
		return true;
	}
}


void CServerService::ResolveOutstandingClient(CClientData* client, BOOL& bTheSameIpPortClientReconnect)
{
	AUTO_LOG_FUNCTION;
	int ademco_id = client->ademco_id;
	auto iter = m_livingClients.find(ademco_id);
	if (iter != m_livingClients.end()) {
		LOG(L"same client, offline-reconnect, donot show its offline info to user. ademco_id %04d\n", client->ademco_id);
		bTheSameIpPortClientReconnect = TRUE;
		iter->second->MoveTaskListToNewObj(client);
		RecycleLiveClient(iter->second, FALSE);
	}
	m_livingClients[ademco_id] = client;
	m_outstandingClients.remove(client);
	LOG(L"Outstanding Connections %d -----------------------------------\n", m_outstandingClients.size());
	LOG(L"Live Connections %d -----------------------------------\n", m_livingClients.size());
}


CClientData* CServerService::AllocateClient()
{
	CClientData* data = nullptr;
	if (m_bufferedClients.size() > 0) {
		data = m_bufferedClients.front();
		m_bufferedClients.pop_front();
		if (m_bufferedClients.size() < NUM_BUFFERED_CLIENTS / 2) {
			size_t num = NUM_BUFFERED_CLIENTS - m_bufferedClients.size();
			for (size_t i = 0; i < num; i++) {
				CClientData* newData = new CClientData();
				m_bufferedClients.push_back(newData);
			}
		}
	} else {
		data = new CClientData();
	}
	return data;
}


void CServerService::RecycleOutstandingClient(CClientData* client)
{
	AUTO_LOG_FUNCTION;
	m_outstandingClients.remove(client);
	RecycleClient(client);
	LOG(L"Outstanding Connections %d -----------------------------------\n", m_outstandingClients.size());
}


void CServerService::RecycleLiveClient(CClientData* client, BOOL bShowOfflineInfo)
{
	AUTO_LOG_FUNCTION;
	if (bShowOfflineInfo && m_handler) {
		m_handler->OnConnectionLost(this, client);
	}
	m_livingClients.erase(client->ademco_id);
	RecycleClient(client);
	LOG(L"Live Connections %d -----------------------------------\n", m_livingClients.size());
}


void CServerService::RecycleClient(CClientData* client)
{
	AUTO_LOG_FUNCTION;
	assert(client);
	shutdown(client->socket, 2);
	closesocket(client->socket);
	client->Clear();
	client->ResetTime(true);
	
	m_bufferedClients.push_back(client);
	while (m_bufferedClients.size() > NUM_BUFFERED_CLIENTS * 2) {
		auto toBeDeletedClient = m_bufferedClients.front();
		m_bufferedClients.pop_front();
		delete toBeDeletedClient;
	}
	LOG(L"Buffered clients %d -----------------------------------\n", m_bufferedClients.size());
}


NAMESPACE_END
NAMESPACE_END
