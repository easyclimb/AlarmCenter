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

	m_bufferedClients.clear();
	m_livingClients.clear();
	m_outstandingClients.clear();

	shutdown(m_ServSock, 2);
	closesocket(m_ServSock);
}


bool set_timeout(SOCKET s, int miliseconds) 
{
	AUTO_LOG_FUNCTION;
	// set recv/send timeout
	int timeout = 0;
	int optlen = sizeof(struct timeval);
	int ret = 0;

	JLOGA("sys info:\n");
	ret = getsockopt(s, SOL_SOCKET, SO_RCVTIMEO, (char *)&timeout, &optlen);
	if (ret == 0) {
		JLOGA("SO_RCVTIMEO: %d\n", timeout);
	} else {
		JLOGA("get SO_RCVTIMEO failed\n");
		return false;
	}

	ret = getsockopt(s, SOL_SOCKET, SO_SNDTIMEO, (char *)&timeout, &optlen);
	if (ret == 0) {
		JLOGA("SO_SNDTIMEO: %d\n", timeout);
	} else {
		JLOGA("get SO_SNDTIMEO failed\n");
		return false;
	}

	timeout = miliseconds;
	JLOGA("user set timeout to %dms:\n", timeout);
	optlen = sizeof(struct timeval);
	ret = setsockopt(s, SOL_SOCKET, SO_RCVTIMEO, (char *)&timeout, optlen);
	if (ret == 0) {
		JLOGA("set SO_RCVTIMEO ok\n");
	} else {
		JLOGA("set SO_RCVTIMEO failed\n");
		return false;
	}

	ret = setsockopt(s, SOL_SOCKET, SO_SNDTIMEO, (char *)&timeout, optlen);
	if (ret == 0) {
		JLOGA("set SO_SNDTIMEO ok\n");
	} else {
		JLOGA("set SO_SNDTIMEO failed\n");
		return false;
	}
	JLOGA("set ok\n");

	JLOGA("sys info:\n");
	ret = getsockopt(s, SOL_SOCKET, SO_RCVTIMEO, (char *)&timeout, &optlen);
	if (ret == 0) {
		JLOGA("SO_RCVTIMEO: %d\n", timeout);
	} else {
		JLOGA("get SO_RCVTIMEO failed\n");
		return false;
	}

	ret = getsockopt(s, SOL_SOCKET, SO_SNDTIMEO, (char *)&timeout, &optlen);
	if (ret == 0) {
		JLOGA("SO_SNDTIMEO: %d\n", timeout);
	} else {
		JLOGA("get SO_SNDTIMEO failed\n");
		return false;
	}

	return true;
}


CServerService::CServerService(unsigned int& nPort, 
							   unsigned int nMaxClients,
							   unsigned int nTimeoutVal,
							   bool blnCreateAsync, 
							   bool blnBindLocal)
	: m_ServSock(INVALID_SOCKET)
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
		auto err = FormatWSAError(errnono);
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
	sAddrIn.sin_port = htons(static_cast<u_short>(nPort));

	// Bind to the localhost ("127.0.0.1") to accept connections only from
	// localhost or 
	// "0.0.0.0" (INADDR_ANY) to accept connections from any IP address.
	if (blnBindLocal) sAddrIn.sin_addr.s_addr = inet_addr("127.0.0.1");
	else sAddrIn.sin_addr.S_un.S_addr = ADDR_ANY;

	// Bind the structure to the created server socket.
	nRet = bind(this->m_ServSock, (struct sockaddr *) &sAddrIn, sizeof(sAddrIn));
	while (nRet < 0) {
		//closesocket(this->m_ServSock);
		JLOG(FormatWSAError(WSAGetLastError()));
		JLOG(L"server socket failed to bind on port %d, now try port %d.", nPort, nPort+1);
		sAddrIn.sin_port = htons(static_cast<u_short>(++nPort));
		nRet = bind(this->m_ServSock, (struct sockaddr *) &sAddrIn, sizeof(sAddrIn));
	}

	// Set server socket in listen mode and set the listen queue to 20.
	nRet = listen(this->m_ServSock, SOMAXCONN);
	if (nRet < 0) {
		closesocket(this->m_ServSock);
		throw "server scket failed to listen.";
	}

	for (size_t i = 0; i < NUM_BUFFERED_CLIENTS; i++) {
		CClientDataPtr data = std::make_shared<CClientData>();
		m_bufferedClients.push_back(data);
	}
}


void CServerService::Start()
{
	if (!running_) {
		running_ = true;
		thread_accept_ = std::thread(&CServerService::ThreadAccept, this);
		thread_recv_ = std::thread(&CServerService::ThreadRecv, this);
	}

	if (m_handler) {
		m_handler->Start();
	}

	JLOG(L"ServerService started successfully");
}


void CServerService::Stop()
{
	AUTO_LOG_FUNCTION;
	

	if (running_) {
		running_ = false;
		thread_accept_.join();
		thread_recv_.join();
	}

	if (m_handler) {
		m_handler->Stop();
	}
}


void CServerService::ThreadAccept()
{
	AUTO_LOG_FUNCTION;
	timeval timeout = { 1, 0 };
	
	while (running_) {
		std::this_thread::sleep_for(std::chrono::milliseconds(1));
		if (!running_) {
			break;
		}

		struct sockaddr_in sForeignAddIn;
		int nLength = sizeof(struct sockaddr_in);
		fd_set rfd;
		int nfds;
		bool ok = false;
		
		while (1) {
			std::this_thread::sleep_for(std::chrono::milliseconds(0));
			if (!running_) {
				break;
			}

			FD_ZERO(&rfd);
			FD_SET(m_ServSock, &rfd);
			nfds = select(1, &rfd, (fd_set*)0, (fd_set*)0, &timeout);
			if (nfds == 0)
				continue;
			else if (nfds > 0){
				FD_CLR(m_ServSock, &rfd);
				ok = true;
				break;
			} else
				break;
		}

		if (!running_) {
			break;
		}

		if (!ok)
			continue;

		SOCKET client = accept(m_ServSock, (struct sockaddr*) &sForeignAddIn, &nLength);
		if (client == INVALID_SOCKET)
			continue;

		JLOG(L"got a new connection!++++++++++++++++++++++++++++++++++++++++++++++++++\n");
		if (m_livingClients.size() >= m_nMaxClients) {
			shutdown(client, 2);
			closesocket(client);
			JLOG(L"LiveConnections %d ***************************\n", m_livingClients.size());
			JLOG(L"LiveConnections >= m_nMaxClients %d.\n", m_nMaxClients);
			continue;
		}

		std::lock_guard<std::mutex> lock(m_cs4outstandingClients);
		CClientDataPtr data = AllocateClient();
		data->socket = client;
		data->ResetTime(false);
		memcpy(&data->foreignAddIn, &sForeignAddIn, sizeof(struct sockaddr_in));
		m_outstandingClients.push_back(data);
	}
}

void CServerService::ThreadRecv()
{
	AUTO_LOG_FUNCTION;
	
	while (running_) {
		std::this_thread::sleep_for(std::chrono::milliseconds(1));
		if (!running_) {
			break;
		}

		// handle outstanding clients
		{
			std::lock_guard<std::mutex> lock(m_cs4outstandingClients);
			for (auto client : m_outstandingClients) {
				std::this_thread::sleep_for(std::chrono::milliseconds(0));
				if (!running_) {
					break;
				}
				int ret = HandleClientEvents(client);
				if (ret == RESULT_CONTINUE) {
					continue;
				} else if (ret == RESULT_BREAK) {
					break;
				} else if (ret == RESULT_RECYCLE_AND_BREAK) {
					RecycleOutstandingClient(client);
					break;
				} else {
					assert(0); break;
				}
			}
		}

		if (!running_) {
			break;
		}

		// handle living clients
		{
			std::lock_guard<std::recursive_mutex> lock(m_cs4liveingClients);
			for (auto iter : m_livingClients) {
				std::this_thread::sleep_for(std::chrono::milliseconds(0));
				if (!running_) {
					break;
				}

				int ret = HandleClientEvents(iter.second);
				if (ret == RESULT_CONTINUE) {
					continue;
				} else if (ret == RESULT_BREAK) {
					break;
				} else if (ret == RESULT_RECYCLE_AND_BREAK) {
					RecycleLiveClient(iter.second, TRUE);
					break;
				} else {
					assert(0); break;
				}
			}
		}
	}
}


CServerService::HANDLE_EVENT_RESULT CServerService::HandleClientEvents(const net::server::CClientDataPtr& client)
{
	//AUTO_LOG_FUNCTION;
	if (!client->hangup) {
		long long lngTimeElapsed = client->GetTimeElapsed();
		if (0 < lngTimeElapsed && static_cast<long long>(m_nTimeoutVal) < lngTimeElapsed) {
			char buff[32] = { 0 };
			time_t last = client->tmLastActionTime;
			struct tm tmtm;
			localtime_s(&tmtm, &last);
			strftime(buff, 32, "%Y-%m-%d %H:%M:%S", &tmtm);
			JLOGA("last action time %s", buff);
			time_t now = time(nullptr);
			localtime_s(&tmtm, &now);
			strftime(buff, 32, "%Y-%m-%d %H:%M:%S", &tmtm);
			JLOGA("now %s", buff);
			JLOG(L"lngTimeElapsed %ld, timeout %d",
							lngTimeElapsed, m_nTimeoutVal);
			JLOG(L"client timeout, kick out. ademco_id %04d",
							client->ademco_id);
			return RESULT_RECYCLE_AND_BREAK;
		}
	}

	if (client->wait_to_kill) {
#ifdef _DEBUG
		JLOG(L"wait_to_kill true.");
		return RESULT_RECYCLE_AND_BREAK;
#else
		return RESULT_CONTINUE;
#endif // _DEBUG	
	}

	if (client->disconnectd) {
#ifdef _DEBUG
		JLOG(L"disconnectd true.");
		return RESULT_RECYCLE_AND_BREAK;
#else
		return RESULT_CONTINUE;
#endif // _DEBUG		
	}

	timeval tv = { 0, 0 };
	fd_set fd_read, fd_write;
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
			JLOG(FormatWSAError(WSAGetLastError()));
			JLOG(L"dwLenToRead %d recv %d bytes, kick out %04d, conn_id %d, continue",
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
			JLOG(FormatWSAError(WSAGetLastError()));
			JLOG(L"dwLenToRead %d recv %d bytes, kick out %04d, conn_id %d, continue",
							dwLenToRead,
							bytes_transfered,
							m_clients[i].ademco_id,
							m_clients[i].conn_id);
			Release(&m_clients[i]);
#else
			client->disconnectd = true;
			JLOG(FormatWSAError(WSAGetLastError()));
			JLOG(L"dwLenToRead %d recv %d bytes, no kick out %04d, continue",
							dwLenToRead, bytes_transfered, client->ademco_id);
			return RESULT_CONTINUE;
#endif
		} else if (m_handler) {
			client->ResetTime(false);
			client->buff.wpos += bytes_transfered;
			DWORD result = ademco::RESULT_OK;
			BOOL resolved = FALSE;
			result = m_handler->OnRecv(this, client, resolved);
			while (1) {
				std::this_thread::sleep_for(std::chrono::milliseconds(0));
				if (!running_) {
					break;
				}

				unsigned int bytes_not_commited = client->buff.wpos - client->buff.rpos;
				if (bytes_not_commited == 0) { if (client->buff.wpos == BUFF_SIZE) { client->buff.Clear(); } break; }
				if (client->buff.wpos == BUFF_SIZE) {
					memmove_s(client->buff.buff, BUFF_SIZE, client->buff.buff + client->buff.rpos, bytes_not_commited);
					memset(client->buff.buff + bytes_not_commited, 0, BUFF_SIZE - bytes_not_commited);
					client->buff.wpos -= client->buff.rpos; client->buff.rpos = 0;
					result = m_handler->OnRecv(this, client, resolved);
				} else { result = m_handler->OnRecv(this, client, resolved); }

				if (result == ademco::RESULT_NOT_ENOUGH) { break; }
			}

			if (resolved) { return RESULT_BREAK; }
		}
	}// handle recv

	// handle send
	if (bWrite) {
		TaskPtr task = client->GetFirstTask();
		if (task) {
			bool bNeedSend = false;
			if (!task->send_once_) {
				bNeedSend = true;
			}

			if (!bNeedSend) {
				auto now = std::chrono::steady_clock::now();
				auto span = std::chrono::duration_cast<std::chrono::seconds>(now - task->_last_send_time);
				if (span.count() > 5) {
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
				JLOG(L"++++++++++++++task list size %d, cur task seq %d, retry_times %d, ademco_id %d, event %d, gg %d, zone %d\n",
					client->taskList.size(), task->_seq, task->_retry_times, task->_ademco_id,
					task->_ademco_event, task->_gg, task->_zone);

				if (task->send_once_)
					task->_retry_times++;
				task->_last_send_time = std::chrono::steady_clock::now();
				task->send_once_ = true;

				static ademco::AdemcoPacket packet;
				char data[1024] = { 0 };
				size_t data_len = packet.Make(data, 1024,
											  ademco::g_valid_ademco_protocals[client->protocal_],
											  task->_seq,
											  nullptr,
											  task->_ademco_id,
											  task->_ademco_event,
											  task->_gg,
											  task->_zone,
											  task->_xdata);
				RealSendToClient(client, data, data_len);
#ifndef ENABLE_SEQ_CONFIRM
				m_clients[i].RemoveFirstTask();
#endif
			}
		}
	}// handle send

	return RESULT_CONTINUE;
}


bool CServerService::RealSendToClient(const net::server::CClientDataPtr& client, const char* data, size_t data_len)
{
	//AUTO_LOG_FUNCTION;
	do {
		assert(client);
		if (client->socket == INVALID_SOCKET)
			break;
		int nRet = 0;
		nRet = send(client->socket, data, data_len, 0);
#ifdef KICKOUT_CLIENT_IF_RECV_OR_SEND_RESULT_EQUALS_TO_0
		if (nRet == 0){
			JLOG(L"send %d bytes, kick out %04d, conn_id %d",
				nRet, client->ademco_id, client->conn_id);
			Release(client);
			break;
		} else if (nRet < 0) {
#else
		if (nRet <= 0) {
#endif
#ifdef KICKOUT_CLIENT_IF_RECV_OR_SEND_RESULT_LESS_THAN_0
			JLOG(L"send %d bytes, kick out %04d, conn_id %d", 
						   nRet, client->ademco_id, client->conn_id);
			Release(client);
			break;
#else
			JLOG(L"send %d bytes, no kick out %04d",
						   nRet, client->ademco_id);
			break;
#endif
		} else {
			JLOG(L"send %d bytes, #%04d",
						   nRet, client->ademco_id);
			client->ResetTime(false);
		}
		return true;
	} while (0);
	JLOG(L"CServerService::SendToClient failed..++++++++++++++++\n");
	return false;
}


bool CServerService::SendToClient(int ademco_id, ademco::ADEMCO_EVENT ademco_event, int gg,
								  int zone, const ademco::char_array_ptr& xdata)
{
	do {
		CClientDataPtr client = FindClient(ademco_id);
		if (client == nullptr)
			break;
		client->AddTask(std::make_shared<Task>(ademco_id, ademco_event, gg, zone, xdata));
		return true;
	} while (0);
	return false;
}

CClientDataPtr CServerService::FindClient(int ademco_id)
{
	AUTO_LOG_FUNCTION;
	std::lock_guard<std::recursive_mutex> lock(m_cs4liveingClients);
	auto iter = m_livingClients.find(ademco_id);
	if (iter == m_livingClients.end()) {
		return false;
	} else {
		return iter->second;
	}
}


void CServerService::ResolveOutstandingClient(const net::server::CClientDataPtr& client, BOOL& bTheSameIpPortClientReconnect)
{
	AUTO_LOG_FUNCTION;
	int ademco_id = client->ademco_id;
	auto iter = m_livingClients.find(ademco_id);
	if (iter != m_livingClients.end()) {
		JLOG(L"same client, offline-reconnect, donot show its offline info to user. ademco_id %04d\n", client->ademco_id);
		bTheSameIpPortClientReconnect = TRUE;
		iter->second->MoveTaskListToNewObj(client);
		RecycleLiveClient(iter->second, FALSE);
	}
	m_livingClients[ademco_id] = client;
	m_outstandingClients.remove(client);
	JLOG(L"Outstanding Connections %d -----------------------------------\n", m_outstandingClients.size());
	JLOG(L"Live Connections %d -----------------------------------\n", m_livingClients.size());
}


CClientDataPtr CServerService::AllocateClient()
{
	CClientDataPtr data = nullptr;
	if (m_bufferedClients.size() > 0) {
		data = m_bufferedClients.front();
		m_bufferedClients.pop_front();
		if (m_bufferedClients.size() < NUM_BUFFERED_CLIENTS / 2) {
			size_t num = NUM_BUFFERED_CLIENTS - m_bufferedClients.size();
			for (size_t i = 0; i < num; i++) {
				CClientDataPtr newData = std::make_shared<CClientData>();
				m_bufferedClients.push_back(newData);
			}
		}
	} else {
		data = std::make_shared<CClientData>();
	}
	return data;
}


void CServerService::RecycleOutstandingClient(const net::server::CClientDataPtr& client)
{
	AUTO_LOG_FUNCTION;
	m_outstandingClients.remove(client);
	RecycleClient(client);
	JLOG(L"Outstanding Connections %d -----------------------------------\n", m_outstandingClients.size());
}


void CServerService::RecycleLiveClient(const net::server::CClientDataPtr& client, BOOL bShowOfflineInfo)
{
	AUTO_LOG_FUNCTION;
	if (bShowOfflineInfo && m_handler) {
		m_handler->OnConnectionLost(this, client);
	}
	int ademco_id = client->ademco_id;
	RecycleClient(client);
	m_livingClients.erase(ademco_id);
	JLOG(L"Live Connections %d -----------------------------------\n", m_livingClients.size());
}


void CServerService::RecycleClient(const net::server::CClientDataPtr& client)
{
	AUTO_LOG_FUNCTION;
	assert(client);
	shutdown(client->socket, 2);
	closesocket(client->socket);
	client->Clear();
	client->ResetTime(true);
	
	m_bufferedClients.push_back(client);
	while (m_bufferedClients.size() > NUM_BUFFERED_CLIENTS * 2) {
		m_bufferedClients.pop_front();
	}
	JLOG(L"Buffered clients %d -----------------------------------\n", m_bufferedClients.size());
}


};};
