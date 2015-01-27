#include "stdafx.h"
#include "ServerService.h"
#include <assert.h>
#pragma comment(lib, "ws2_32.lib")



#include "ademco_func.h"
using namespace Ademco;

namespace net {
CServerService::CServerService()
{}


CServerService::~CServerService()
{
	Stop();
	::DeleteCriticalSection(&m_cs);
	::DeleteCriticalSection(&m_cs4client);
}



CServerService::CServerService(unsigned short nPort, unsigned int nMaxClients,
							   unsigned int nTimeoutVal,
							   bool blnCreateAsync, bool blnBindLocal)
							   : m_ServSock(INVALID_SOCKET)
							   , m_ShutdownEvent(INVALID_HANDLE_VALUE)
							   , m_phThreadAccept(NULL)
							   , m_phThreadRecv(NULL)
							   , m_hThreadTimeoutChecker(INVALID_HANDLE_VALUE)
							   , m_nLiveConnections(0)
							   , m_handler(NULL)
							   , m_nMaxClients(nMaxClients)
							   , m_nTimeoutVal(nTimeoutVal)
{
	int nRet = -1;
	unsigned long lngMode = 1;
	struct sockaddr_in sAddrIn;
	memset(&sAddrIn, 0, sizeof(sAddrIn));

	// Create the server socket, set the necessary 
	// parameters for making it IOCP compatible.
	m_ServSock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);// , NULL, 0, WSA_FLAG_OVERLAPPED);
	if (INVALID_SOCKET == this->m_ServSock) {
		//throw L"server socket creation failed.";
		int errnono = WSAGetLastError();
		wchar_t *err = FormatWSAError(errnono);
		throw err;
	}

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
	nRet = bind(this->m_ServSock, (struct sockaddr *) &sAddrIn, sizeof(sAddrIn));
	if (nRet < 0) {
		closesocket(this->m_ServSock);
		throw "server socket failed to bind on given port.";
	}

	// Set server socket in listen mode and set the listen queue to 20.
	nRet = listen(this->m_ServSock, SOMAXCONN);
	if (nRet < 0) {
		closesocket(this->m_ServSock);
		throw "server scket failed to listen.";
	}
	/*
	for (unsigned int  i = 0; i < nMaxClients; i++) {
	CLIENT* client = new CLIENT();
	m_clients.push_back(client);
	}
	*/
	m_clients = new CLIENT[nMaxClients];

	::InitializeCriticalSection(&m_cs);
	::InitializeCriticalSection(&m_cs4client);
}


void CServerService::Start()
{
	if (INVALID_HANDLE_VALUE == m_ShutdownEvent) {
		m_ShutdownEvent = CreateEvent(NULL, TRUE, FALSE, NULL);
	}

	if (NULL == m_phThreadAccept) {
		m_phThreadAccept = new HANDLE[THREAD_ACCEPT_NO];
		for (int i = 0; i < THREAD_ACCEPT_NO; i++) {
			m_phThreadAccept[i] = CreateThread(NULL, 0, ThreadAccept, this, 0, NULL);
		}
	}

	if (NULL == m_phThreadRecv) {
		m_phThreadRecv = new HANDLE[THREAD_RECV_NO];
		for (int i = 0; i < THREAD_RECV_NO; i++) {
			m_phThreadRecv[i] = CreateThread(NULL, 0, ThreadRecv, this, CREATE_SUSPENDED, NULL);
			SetThreadPriority(m_phThreadRecv[i], THREAD_PRIORITY_ABOVE_NORMAL);
			ResumeThread(m_phThreadRecv[i]);
		}
	}

	if (m_handler) {
		m_handler->Start();
	}

	if (m_nTimeoutVal > 0 && m_hThreadTimeoutChecker == INVALID_HANDLE_VALUE) {
		m_hThreadTimeoutChecker = CreateThread(NULL, 0, ThreadTimeoutChecker, this, 0, NULL);
	}

	CLog::WriteLog(L"ServerService started successfully");
}


void CServerService::Stop()
{
	if (m_handler) {
		m_handler->Stop();
	}

	if (INVALID_HANDLE_VALUE != m_ShutdownEvent) {
		SetEvent(m_ShutdownEvent);

		if (INVALID_HANDLE_VALUE != m_hThreadTimeoutChecker) {
			WaitForSingleObject(m_hThreadTimeoutChecker, INFINITE);
			CloseHandle(m_hThreadTimeoutChecker);
			m_hThreadTimeoutChecker = INVALID_HANDLE_VALUE;
		}

		if (NULL != m_phThreadAccept) {
			WaitForMultipleObjects(THREAD_ACCEPT_NO, m_phThreadAccept, TRUE, INFINITE);
			for (int i = 0; i < THREAD_ACCEPT_NO; i++) {
				CloseHandle(m_phThreadAccept[i]);
			}
			delete[] m_phThreadAccept;
			m_phThreadAccept = NULL;
		}

		if (NULL != m_phThreadRecv) {
			WaitForMultipleObjects(THREAD_RECV_NO, m_phThreadRecv, TRUE, INFINITE);
			for (int i = 0; i < THREAD_RECV_NO; i++) {
				CloseHandle(m_phThreadRecv[i]);
			}
			delete[] m_phThreadRecv;
			m_phThreadRecv = NULL;
		}

		CloseHandle(m_ShutdownEvent);
		m_ShutdownEvent = INVALID_HANDLE_VALUE;
	}

	//for (unsigned int i = 0; i < m_nMaxClients; i++) {
	//	Release(&m_clients[i]);
	//}

	if (m_clients) {
		delete[] m_clients;
		m_clients = NULL;
	}
}


DWORD WINAPI CServerService::ThreadAccept(LPVOID lParam)
{
	CServerService *server = static_cast<CServerService*>(lParam);
	CLog::WriteLog(L"Server service now start listening. ThreadAccept is running.");
	while (1) {
		if (WAIT_OBJECT_0 == WaitForSingleObject(server->m_ShutdownEvent, 1))
			break;

		struct sockaddr_in sForeignAddIn;
		int nLength = sizeof(struct sockaddr_in);
		SOCKET client = accept(server->m_ServSock, (struct sockaddr*) &sForeignAddIn, &nLength);
		if (client == INVALID_SOCKET)
			continue;
		if (server->m_nLiveConnections >= server->m_nMaxClients) {
			shutdown(client, 2);
			closesocket(client);
			continue;
		}
		CLocalLock lock(&server->m_cs);
		bool bFoundIdleClientConnid = false;
		//unsigned int half_client_no = server->m_nMaxClients / 2;
		//bool bFirstFindInFirstHalf = server->m_nLiveConnections < half_client_no;
		unsigned int conn_id = 0xffffffff;
		/*do {
			if (bFirstFindInFirstHalf) {
			for (unsigned int i = 0; i < half_client_no; i++) {
			if (CONNID_IDLE == server->m_clients[i].conn_id) {
			conn_id = i;
			bFoundIdleClientConnid = true;
			break;
			}
			}
			}
			if (bFoundIdleClientConnid)
			break;
			for (unsigned int i = half_client_no; i < server->m_nMaxClients; i++) {
			if (CONNID_IDLE == server->m_clients[i].conn_id) {
			conn_id = i;
			bFoundIdleClientConnid = true;
			break;
			}
			}
			} while (0);*/

		do {
			if (server->m_nLiveConnections < server->m_nMaxClients) {
				for (unsigned int i = server->m_nLiveConnections; i < server->m_nMaxClients; i++) {
					if (CONNID_IDLE == server->m_clients[i].conn_id) {
						conn_id = i;
						bFoundIdleClientConnid = true;
						break;
					}
				}
			}

			if (bFoundIdleClientConnid)
				break;

			for (unsigned int i = 0; i < server->m_nLiveConnections; i++) {
				if (CONNID_IDLE == server->m_clients[i].conn_id) {
					conn_id = i;
					bFoundIdleClientConnid = true;
					break;
				}
			}
		} while (0);

		if (bFoundIdleClientConnid) {
			server->m_clients[conn_id].socket = client;
			memcpy(&server->m_clients[conn_id].foreignAddIn, &sForeignAddIn, sizeof(struct sockaddr_in));
			server->m_clients[conn_id].conn_id = conn_id;
			server->m_clients[conn_id].ResetTime(false);
			InterlockedIncrement(&server->m_nLiveConnections);
			if (server->m_handler) {
				server->m_handler->OnConnectionEstablished(server, &server->m_clients[conn_id]);
			}
		} else {
			assert(0);
			shutdown(client, 2);
			closesocket(client);
			continue;
		}
	}

	CLog::WriteLog(L"Server service's ThreadAccept exited.");
	return 0;
}

DWORD WINAPI CServerService::ThreadRecv(LPVOID lParam)
{
	CServerService *server = static_cast<CServerService*>(lParam);
	CLog::WriteLog(L"Server service's ThreadRecv now start running.");
	timeval tv = { 0, 0 };	// 超时时间1ms
	fd_set fd_read;
	for (;;) {
		if (WAIT_OBJECT_0 == WaitForSingleObject(server->m_ShutdownEvent, 1))
			break;
		CLocalLock lock(&server->m_cs4client);
		for (unsigned int i = 0; i < server->m_nMaxClients; i++) {
			if (WAIT_OBJECT_0 == WaitForSingleObject(server->m_ShutdownEvent, 0))
				break;
			if (CONNID_IDLE != server->m_clients[i].conn_id) {
				FD_ZERO(&fd_read);
				FD_SET(server->m_clients[i].socket, &fd_read);
				int ret = select(0, &fd_read, NULL, NULL, &tv);
				if (ret <= 0)
					continue;
				if (!FD_ISSET(server->m_clients[i].socket, &fd_read))
					continue;
				//char buff[4096] = { 0 };
				//char *temp = server->m_clients[i].buff.buff + server->m_clients[i].buff.wpos;
				//unsigned int len_to_read = BUFF_SIZE - server->m_clients[i].buff.wpos;
				char* temp = server->m_clients[i].buff.buff + server->m_clients[i].buff.wpos;
				DWORD dwLenToRead = BUFF_SIZE - server->m_clients[i].buff.wpos;
				int bytes_transfered = recv(server->m_clients[i].socket, temp, dwLenToRead, 0);
				if (bytes_transfered <= 0) {
					CLog::WriteLog(L"dwLenToRead %d recv %d bytes, kick out %04d", 
								   dwLenToRead,
								   bytes_transfered, 
								   server->m_clients[i].ademco_id);
					server->Release(&server->m_clients[i]);
				} else if (server->m_handler) {
					server->m_clients[i].ResetTime(false);
					/*server->m_clients[i].buff.wpos += bytes_transfered;
					server->m_handler->OnRecv(server, &server->m_clients[i]);
					if (server->m_clients[i].buff.wpos == BUFF_SIZE) {
						unsigned int bytes_not_commited =
							server->m_clients[i].buff.wpos - server->m_clients[i].buff.rpos;
						memmove_s(server->m_clients[i].buff.buff, BUFF_SIZE,
								  server->m_clients[i].buff.buff + server->m_clients[i].buff.rpos,
								  bytes_not_commited);
						memset(server->m_clients[i].buff.buff + bytes_not_commited,
							   0, BUFF_SIZE - bytes_not_commited);
						server->m_clients[i].buff.wpos -= server->m_clients[i].buff.rpos;
						server->m_clients[i].buff.rpos = 0;
						server->m_handler->OnRecv(server, &server->m_clients[i]);
					}*/
					server->m_clients[i].buff.wpos += bytes_transfered;
					DWORD ret = ARV_OK;
					ret = server->m_handler->OnRecv(server, &server->m_clients[i]);

					while (1) {
						unsigned int bytes_not_commited = 
							server->m_clients[i].buff.wpos - server->m_clients[i].buff.rpos;
						if (bytes_not_commited == 0) {
							if (server->m_clients[i].buff.wpos == BUFF_SIZE) {
								server->m_clients[i].buff.Clear();
							}
							break;
						}
						if (server->m_clients[i].buff.wpos == BUFF_SIZE) {
							memmove_s(server->m_clients[i].buff.buff, 
									  BUFF_SIZE,
									  server->m_clients[i].buff.buff + 
									  server->m_clients[i].buff.rpos,
									  bytes_not_commited);
							memset(server->m_clients[i].buff.buff + bytes_not_commited,
								   0, BUFF_SIZE - bytes_not_commited);
							server->m_clients[i].buff.wpos -= 
								server->m_clients[i].buff.rpos;
							server->m_clients[i].buff.rpos = 0;
							ret = server->m_handler->OnRecv(server, 
															&server->m_clients[i]);
						} else {
							ret = server->m_handler->OnRecv(server, 
															&server->m_clients[i]);
						}
						if (ret == ARV_PACK_NOT_ENOUGH) {
							break;
						}
					}
				}
			}
		}
	}

	CLog::WriteLog(L"Server service's ThreadRecv exited.");
	return 0;
}

void CServerService::Release(CLIENT* client)
{
	assert(client);
	if (client->conn_id == CONNID_IDLE)
		return;
	if (m_handler) {
		m_handler->OnConnectionLost(this, client);
	}
	shutdown(client->socket, 2);
	closesocket(client->socket);
	client->Clear();
	client->ResetTime(true);
	InterlockedDecrement(&m_nLiveConnections);
}


bool CServerService::SendToClient(unsigned int conn_id, const char* data, size_t data_len)
{
	do {
		if (conn_id == CONNID_IDLE)
			break;
		if (IsBadReadPtr(data, data_len))
			break;
		if (m_clients[conn_id].conn_id != conn_id)
			break;
		if (m_clients[conn_id].socket == INVALID_SOCKET)
			break;
		timeval tv = { 0, 10000 };
		fd_set fdWrite;
		FD_ZERO(&fdWrite);
		FD_SET(m_clients[conn_id].socket, &fdWrite);
		int nRet = 0;
		do {
			nRet = select(m_clients[conn_id].socket + 1, NULL, &fdWrite, NULL, &tv);
		} while (nRet <= 0 && !FD_ISSET(m_clients[conn_id].socket, &fdWrite));
		nRet = send(m_clients[conn_id].socket, data, data_len, 0);
		if (nRet <= 0) {
			CLog::WriteLog(L"send %d bytes, kick out %04d", nRet, m_clients[conn_id].ademco_id);
			Release(&m_clients[conn_id]);
			break;
		} else {
			m_clients[conn_id].ResetTime(false);
		}
		return true;
	} while (0);
	return false;
}


bool CServerService::SendToClient(CLIENT* client, const char* data, size_t data_len)
{
	do {
		if (client == NULL)
			break;
		if (IsBadReadPtr(data, data_len))
			break;
		if (client->socket == INVALID_SOCKET)
			break;
		timeval tv = { 0, 10000 };
		fd_set fdWrite;
		FD_ZERO(&fdWrite);
		FD_SET(client->socket, &fdWrite);
		int nRet = 0;
		do {
			nRet = select(client->socket + 1, NULL, &fdWrite, NULL, &tv);
		} while (nRet <= 0 && !FD_ISSET(client->socket, &fdWrite));
		nRet = send(client->socket, data, data_len, 0);
		if (nRet <= 0) {
			CLog::WriteLog(L"send %d bytes, kick out %04d", nRet, client->ademco_id);
			Release(client);
			break;
		} else {
			client->ResetTime(false);
		}
		return true;
	} while (0);
	return false;
}

bool CServerService::FindClient(int ademco_id, CLIENT** client) const
{
	do {
		if (ademco_id < 0 || static_cast<unsigned int>(ademco_id) >= this->m_nMaxClients)
			break;
		for (unsigned int i = 0; i < m_nMaxClients; i++) {
			if (m_clients[i].conn_id != CONNID_IDLE
				&& m_clients[i].ademco_id == ademco_id) {
				*client = &m_clients[i];
				return true;
			}
		}
	} while (0);
	return false;
}

bool CServerService::GetClient(unsigned int conn_id, CLIENT** client) const
{
	do {
		if (conn_id < 0 || static_cast<unsigned int>(conn_id) >= this->m_nMaxClients)
			break;
		if (m_clients[conn_id].conn_id == conn_id) {
			*client = &m_clients[conn_id];
			return true;
		}
	} while (0);
	return false;
}


void CServerService::KillOtherClients(unsigned int conn_id, int ademco_id)
{
	do {
		if (ademco_id < 0 || static_cast<unsigned int>(ademco_id) >= this->m_nMaxClients)
			break;
		for (unsigned int i = 0; i < m_nMaxClients; i++) {
			if (i != conn_id
				&& m_clients[i].conn_id != CONNID_IDLE
				&& m_clients[i].ademco_id == ademco_id) {
				Release(&m_clients[i]);
			}
		}
	} while (0);
}


DWORD WINAPI CServerService::ThreadTimeoutChecker(LPVOID lParam)
{
	CServerService *server = reinterpret_cast<CServerService*>(lParam);
	CLog::WriteLog(L"Server service's time out checker now start running.");
	for (;;) {
		if (WAIT_OBJECT_0 == WaitForSingleObject(server->m_ShutdownEvent, 1000))
			break;
		CLocalLock lock(&server->m_cs4client);
		for (unsigned int i = 0; i < server->m_nMaxClients; i++) {
			if (WAIT_OBJECT_0 == WaitForSingleObject(server->m_ShutdownEvent, 0))
				break;
			if (CONNID_IDLE != server->m_clients[i].conn_id) {
				unsigned long lngTimeElapsed = server->m_clients[i].GetTimeElapsed();
				// check elapsed time, since last registered action,
				// with the configured time-out.
				if (lngTimeElapsed > static_cast<unsigned long>(server->m_nTimeoutVal)) {
					// clear the time and push the socket to 
					// the IOCP with status _CLOSE. Socket 
					// will be closed shortly.
					CLog::WriteLog(L"网络模块连接超时， kick out. conn_id: %d", 
								   server->m_clients[i].conn_id);
					server->Release(&server->m_clients[i]);
				}
			}
		}
	}
	CLog::WriteLog(L"Server service's time out checker exited.");
	return 0;
}

NAMESPACE_END
