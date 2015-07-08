#include "stdafx.h"
#include "ServerService.h"
#include <assert.h>
#include <WinSock2.h>
#pragma comment(lib, "ws2_32.lib")
#include "ademco_func.h"
//#define KICKOUT_CLIENT_IF_RECV_OR_SEND_RESULT_EQUALS_TO_0
//#define KICKOUT_CLIENT_IF_RECV_OR_SEND_RESULT_LESS_THAN_0

//#include "ademco_func.h"
//using namespace ademco;

namespace net {
namespace server {

CServerService::CServerService()
{}


CServerService::~CServerService()
{
	Stop();
	::DeleteCriticalSection(&m_cs);
	::DeleteCriticalSection(&m_cs4client); 
	::DeleteCriticalSection(&m_cs4clientReference);
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
							   , m_phThreadAccept(NULL)
							   , m_phThreadRecv(NULL)
							   //, m_hThreadTimeoutChecker(INVALID_HANDLE_VALUE)
							   , m_nLiveConnections(0)
							   , m_handler(NULL)
							   , m_nMaxClients(nMaxClients)
							   , m_nTimeoutVal(nTimeoutVal)
{
	int nRet = -1;
	unsigned long lngMode = 0;
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
	/*if (nRet < 0) {
		closesocket(this->m_ServSock);
		throw "server socket failed to bind on given port.";
	}*/

	// Set server socket in listen mode and set the listen queue to 20.
	nRet = listen(this->m_ServSock, SOMAXCONN);
	if (nRet < 0) {
		closesocket(this->m_ServSock);
		throw "server scket failed to listen.";
	}
	/*
	for (unsigned int  i = 0; i < nMaxClients; i++) {
	CClientData* client = new CClientData();
	m_clients.push_back(client);
	}
	*/
	//m_clients = new CClientData[nMaxClients];
	//m_clientsReference = new PCClientData[nMaxClients];
	for (unsigned int i = 0; i < nMaxClients; i++) {
		m_clientsReference[i] = NULL;
	}
	::InitializeCriticalSection(&m_cs);
	::InitializeCriticalSection(&m_cs4client);
	::InitializeCriticalSection(&m_cs4clientReference);
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
			THREAD_PARAM* param = new THREAD_PARAM();
			param->service = this;
			param->thread_no = i;
			m_phThreadRecv[i] = CreateThread(NULL, 0, ThreadRecv, param, CREATE_SUSPENDED, NULL);
			SetThreadPriority(m_phThreadRecv[i], THREAD_PRIORITY_ABOVE_NORMAL);
			ResumeThread(m_phThreadRecv[i]);
		}
	}

	if (m_handler) {
		m_handler->Start();
	}

	//if (m_nTimeoutVal > 0 && m_hThreadTimeoutChecker == INVALID_HANDLE_VALUE) {
	//	m_hThreadTimeoutChecker = CreateThread(NULL, 0, ThreadTimeoutChecker, this, 0, NULL);
	//}

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

		/*if (INVALID_HANDLE_VALUE != m_hThreadTimeoutChecker) {
			WaitForSingleObject(m_hThreadTimeoutChecker, INFINITE);
			CloseHandle(m_hThreadTimeoutChecker);
			m_hThreadTimeoutChecker = INVALID_HANDLE_VALUE;
		}*/

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

	//if (m_clients) {
	//	delete[] m_clients;
	//	m_clients = NULL;
	//}

	//if (m_clientsReference) {
	//	delete[] m_clientsReference;
	//	m_clientsReference = NULL;
	//}
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
		if (server->m_nLiveConnections >= server->m_nMaxClients) {
			shutdown(client, 2);
			closesocket(client);
			LOG(L"m_nLiveConnections %d ***************************\n", server->m_nLiveConnections);
			LOG(L"m_nLiveConnections >= m_nMaxClients %d.\n", server->m_nMaxClients);
			continue;
		}
		//CLocalLock lock(&server->m_cs);
		bool bFoundIdleClientConnid = false;
		//unsigned int half_client_no = server->m_nMaxClients / 2;
		//bool bFirstFindInFirstHalf = server->m_nLiveConnections < half_client_no;
		unsigned int conn_id = CONNID_IDLE;
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
			//set_timeout(client, 10000);
			server->m_clients[conn_id].socket = client;
			memcpy(&server->m_clients[conn_id].foreignAddIn, &sForeignAddIn, sizeof(struct sockaddr_in));
			server->m_clients[conn_id].ResetTime(false);
			server->m_clients[conn_id].conn_id = conn_id;
			InterlockedIncrement(&server->m_nLiveConnections);
			LOG(L"m_nLiveConnections %d +++++++++++++++++++++++++++++++++++++\n", server->m_nLiveConnections);
			if (server->m_handler) {
				server->m_handler->OnConnectionEstablished(server, &server->m_clients[conn_id]);
			}
		} else {
			assert(0);
			shutdown(client, 2);
			closesocket(client);
			LOG(L"m_nLiveConnections %d ***************************\n", server->m_nLiveConnections);
			LOG(L"bFoundIdleClientConnid false.\n");
			continue;
		}
	}

	return 0;
}

DWORD WINAPI CServerService::ThreadRecv(LPVOID lParam)
{
	AUTO_LOG_FUNCTION;
	THREAD_PARAM* param = reinterpret_cast<THREAD_PARAM*>(lParam);
	CServerService *server = param->service;
	unsigned int thread_no = param->thread_no;
	unsigned int client_per_thread = server->m_nMaxClients / THREAD_RECV_NO;
	unsigned int conn_id_range_begin = thread_no * client_per_thread;
	unsigned int conn_id_range_end = conn_id_range_begin + client_per_thread;
	delete param;
	timeval tv = { 0, 0 };
	fd_set fd_read, fd_write;
	for (;;) {
		if (WAIT_OBJECT_0 == WaitForSingleObject(server->m_ShutdownEvent, 0))
			break;
		//CLocalLock lock(&server->m_cs4client);
		for (unsigned int i = conn_id_range_begin; i < conn_id_range_end; i++) {
			if (WAIT_OBJECT_0 == WaitForSingleObject(server->m_ShutdownEvent, /*(i % 1000 == 0) ? 1 : */0))
				break;
			if (CONNID_IDLE != server->m_clients[i].conn_id) {
				/*if (server->m_clients[i].ademco_id == 62) {
					LOG(L"-------------------------client 62------------------------\n");
				}*/
				if (!server->m_clients[i].hangup) {
					long long lngTimeElapsed = server->m_clients[i].GetTimeElapsed();
					if (0 < lngTimeElapsed && static_cast<long long>(server->m_nTimeoutVal) < lngTimeElapsed) {
						char buff[32] = { 0 };
						time_t last = server->m_clients[i].tmLastActionTime;
						struct tm tmtm;
						localtime_s(&tmtm, &last);
						strftime(buff, 32, "%Y-%m-%d %H:%M:%S", &tmtm);
						CLog::WriteLogA("last action time %s", buff);
						time_t now = time(NULL);
						localtime_s(&tmtm, &now);
						strftime(buff, 32, "%Y-%m-%d %H:%M:%S", &tmtm);
						CLog::WriteLogA("now %s", buff);
						CLog::WriteLog(L"lngTimeElapsed %ld, timeout %d",
									   lngTimeElapsed, server->m_nTimeoutVal);
						CLog::WriteLog(L"client timeout， kick out. conn_id: %d, ademco_id %04d",
									   server->m_clients[i].conn_id, server->m_clients[i].ademco_id);
						server->Release(&server->m_clients[i]);
						continue;
					}
				}

				FD_ZERO(&fd_read);
				FD_ZERO(&fd_write);
				FD_SET(server->m_clients[i].socket, &fd_read);
				FD_SET(server->m_clients[i].socket, &fd_write);
				int ret = select(0, &fd_read, &fd_write, NULL, &tv);
				if (ret <= 0)
					continue;
				BOOL bRead = FD_ISSET(server->m_clients[i].socket, &fd_read);
				BOOL bWrite = FD_ISSET(server->m_clients[i].socket, &fd_write);
				
				// handle recv
				if (bRead) {
					/*if (server->m_clients[i].ademco_id == 62) {
						LOG(L"-------------------------client 62 bRead------------------------\n");
					}*/
					//char buff[4096] = { 0 };
					//char *temp = server->m_clients[i].buff.buff + server->m_clients[i].buff.wpos;
					//unsigned int len_to_read = BUFF_SIZE - server->m_clients[i].buff.wpos;
					char* temp = server->m_clients[i].buff.buff + server->m_clients[i].buff.wpos;
					DWORD dwLenToRead = BUFF_SIZE - server->m_clients[i].buff.wpos;
					int bytes_transfered = recv(server->m_clients[i].socket, temp, dwLenToRead, 0);
#ifdef KICKOUT_CLIENT_IF_RECV_OR_SEND_RESULT_EQUALS_TO_0
					if (bytes_transfered == 0) {
						LOG(FormatWSAError(WSAGetLastError()));
						CLog::WriteLog(L"dwLenToRead %d recv %d bytes, kick out %04d, conn_id %d, continue", 
									   dwLenToRead,
									   bytes_transfered,
									   server->m_clients[i].ademco_id,
									   server->m_clients[i].conn_id);
						server->Release(&server->m_clients[i]);
					} 
					if (bytes_transfered < 0) {
#else
					if (bytes_transfered <= 0) {
#endif 
#ifdef KICKOUT_CLIENT_IF_RECV_OR_SEND_RESULT_LESS_THAN_0
						LOG(FormatWSAError(WSAGetLastError()));
						CLog::WriteLog(L"dwLenToRead %d recv %d bytes, kick out %04d, conn_id %d, continue",
									   dwLenToRead,
									   bytes_transfered,
									   server->m_clients[i].ademco_id,
									   server->m_clients[i].conn_id);
						server->Release(&server->m_clients[i]);
#else
						LOG(FormatWSAError(WSAGetLastError()));
						CLog::WriteLog(L"dwLenToRead %d recv %d bytes, no kick out %04d, conn_id %d, continue",
									   dwLenToRead,
									   bytes_transfered,
									   server->m_clients[i].ademco_id,
									   server->m_clients[i].conn_id);
						continue;
#endif
					} else if (server->m_handler) {
						server->m_clients[i].ResetTime(false);
						server->m_clients[i].buff.wpos += bytes_transfered;

						DWORD ret = RESULT_OK;
						ret = server->m_handler->OnRecv(server, &server->m_clients[i]);

						while (1) {
							if (WAIT_OBJECT_0 == WaitForSingleObject(server->m_ShutdownEvent, 0))
								break;
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
								ret = server->m_handler->OnRecv(server, &server->m_clients[i]);
							} else {
								ret = server->m_handler->OnRecv(server, &server->m_clients[i]);
							}
							if (ret == RESULT_NOT_ENOUGH) {
								break;
							}
						}
					}

					/*if (server->m_clients[i].ademco_id == 62) {
						LOG(L"-------------------------client 62 bRead over------------------------\n");
					}*/
				}

				// handle send
				if (bWrite) {
					/*if (server->m_clients[i].ademco_id == 62) {
						LOG(L"-------------------------client 62 bWrite------------------------\n");
					}
					*/
					Task* task = server->m_clients[i].GetFirstTask();
					if (task) {
						bool bNeedSend = false;
						//ULONGLONG now = GetTickCount64();
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
								server->m_clients[i].RemoveFirstTask();
							}
						}
						if (bNeedSend) {
							LOG(L"++++++++++++++task list size %d, cur task seq %d, retry_times %d, ademco_id %d, event %d, gg %d, zone %d, xdata_len %d\n",
								server->m_clients[i].taskList.size(), task->_seq, task->_retry_times, task->_ademco_id,
								task->_ademco_event, task->_gg, task->_zone, task->_xdata_len);
							if (task->_last_send_time.GetStatus() == COleDateTime::valid)
								task->_retry_times++;
							task->_last_send_time = COleDateTime::GetCurrentTime();
							ademco::AdemcoPacket packet;
							char data[1024] = { 0 };
							size_t data_len = packet.Make(data, 1024, ademco::AID_HB, task->_seq,
														  server->m_clients[i].acct, task->_ademco_id,
														  task->_ademco_event, task->_gg, task->_zone,
														  task->_xdata, task->_xdata_len);
							server->SendToClient(&server->m_clients[i], data, data_len);
#ifndef ENABLE_SEQ_CONFIRM
							server->m_clients[i].RemoveFirstTask();
#endif
						}
					}
					/*if (server->m_clients[i].ademco_id == 62) {
						LOG(L"-------------------------client 62 bWrite over------------------------\n");
					}*/
				}
			}
		}
	}

	return 0;
}

void CServerService::Release(CClientData* client, BOOL bNeed2UnReference)
{
	AUTO_LOG_FUNCTION;
	assert(client);
	if (client->conn_id == CONNID_IDLE)
		return;

	if (bNeed2UnReference) {
		while (!TryEnterCriticalSection(&m_cs4clientReference)) {
			LOG(L"TryEnterCriticalSection failed.\n");
			Sleep(100);
		}
		m_clientsReference[client->ademco_id] = NULL;
		LeaveCriticalSection(&m_cs4clientReference);
	}

	if (m_handler) {
		m_handler->OnConnectionLost(this, client);
	}

	shutdown(client->socket, 2);
	closesocket(client->socket);
	client->Clear();
	client->ResetTime(true);
	InterlockedDecrement(&m_nLiveConnections);
	if (m_nLiveConnections < 0)
		m_nLiveConnections = 0;
	LOG(L"m_nLiveConnections %d ++++++++++++++++++++++++++++++++++++++++++++++++n", m_nLiveConnections);
}


bool CServerService::SendToClient(unsigned int conn_id, const char* data, size_t data_len)
{
	AUTO_LOG_FUNCTION;
	do {
		if (conn_id == CONNID_IDLE)
			break;
		if (IsBadReadPtr(data, data_len))
			break;
		if (m_clients[conn_id].conn_id != conn_id)
			break;
		if (m_clients[conn_id].socket == INVALID_SOCKET)
			break;
		int nRet = 0;
		timeval tv = { 0, 10000 };
		fd_set fdWrite;
		FD_ZERO(&fdWrite);
		FD_SET(m_clients[conn_id].socket, &fdWrite);
		do {
			nRet = select(m_clients[conn_id].socket + 1, NULL, &fdWrite, NULL, &tv);
		} while (nRet <= 0 && !FD_ISSET(m_clients[conn_id].socket, &fdWrite));
		nRet = send(m_clients[conn_id].socket, data, data_len, 0);
#ifdef KICKOUT_CLIENT_IF_RECV_OR_SEND_RESULT_EQUALS_TO_0
		if (nRet == 0) {
			CLog::WriteLog(L"send %d bytes, kick out %04d", nRet, m_clients[conn_id].ademco_id);
			Release(&m_clients[conn_id]);
			break;
		} else if (nRet < 0) {
#else
		if (nRet <= 0) {
#endif
#ifdef KICKOUT_CLIENT_IF_RECV_OR_SEND_RESULT_LESS_THAN_0
			CLog::WriteLog(L"send %d bytes, kick out %04d", nRet, m_clients[conn_id].ademco_id);
			Release(&m_clients[conn_id]);
			break;
#else
			CLog::WriteLog(L"send %d bytes, no kick out %04d, conn_id %d",
						   nRet, m_clients[conn_id].ademco_id, m_clients[conn_id].conn_id);
			break;
#endif
		} else {
			m_clients[conn_id].ResetTime(false);
		}
		//LOG(L"CServerService::SendToClient success.++++++++++++++++\n");
		return true;
	} while (0);
	LOG(L"CServerService::SendToClient failed..++++++++++++++++\n");
	return false;
}


bool CServerService::SendToClient(CClientData* client, const char* data, size_t data_len)
{
	AUTO_LOG_FUNCTION;
	do {
		if (client == NULL)
			break;
		if (IsBadReadPtr(data, data_len))
			break;
		if (client->socket == INVALID_SOCKET)
			break;
		int nRet = 0;
		/*timeval tv = { 0, 10000 };
		fd_set fdWrite;
		FD_ZERO(&fdWrite);
		FD_SET(client->socket, &fdWrite);
		do {
			nRet = select(client->socket + 1, NULL, &fdWrite, NULL, &tv);
		} while (nRet <= 0 && !FD_ISSET(client->socket, &fdWrite));*/
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
			CLog::WriteLog(L"send %d bytes, no kick out %04d, conn_id %d",
						   nRet, client->ademco_id, client->conn_id);
			break;
#endif
		} else {
			client->ResetTime(false);
		}
		//LOG(L"CServerService::SendToClient success.++++++++++++++++\n");
		return true;
	} while (0);
	LOG(L"CServerService::SendToClient failed..++++++++++++++++\n");
	return false;
}

bool CServerService::FindClient(int ademco_id, CClientData** client)
{
	AUTO_LOG_FUNCTION;
	do {
		if (ademco_id < 0 || static_cast<unsigned int>(ademco_id) >= this->m_nMaxClients)
			break;
		/*for (unsigned int i = 0; i < m_nMaxClients; i++) {
			if (m_clients[i].conn_id != CONNID_IDLE
				&& m_clients[i].ademco_id == ademco_id) {
				*client = &m_clients[i];
				return true;
			}
		}*/
		//CLocalLock lock(&m_cs4clientReference);
		while (!TryEnterCriticalSection(&m_cs4clientReference)) {
			LOG(L"TryEnterCriticalSection failed.\n");
			Sleep(100);
		}
		*client = m_clientsReference[ademco_id];
		LeaveCriticalSection(&m_cs4clientReference);
		return *client != NULL;
	} while (0);
	return false;
}

//bool CServerService::GetClient(unsigned int conn_id, CClientData** client) const
//{
//	do {
//		if (conn_id < 0 || static_cast<unsigned int>(conn_id) >= this->m_nMaxClients)
//			break;
//		if (m_clients[conn_id].conn_id == conn_id) {
//			*client = &m_clients[conn_id];
//			return true;
//		}
//	} while (0);
//	return false;
//}


//void CServerService::UnReferenceClient(int ademco_id)
//{
//	/*do {
//		if (ademco_id < 0 || static_cast<unsigned int>(ademco_id) >= this->m_nMaxClients)
//			break;
//		for (unsigned int i = 0; i < m_nMaxClients; i++) {
//			if (i != conn_id
//				&& m_clients[i].conn_id != CONNID_IDLE
//				&& m_clients[i].ademco_id == ademco_id) {
//				Release(&m_clients[i]);
//			}
//		}
//	} while (0);*/
//	CLocalLock lock(&m_cs4clientReference);
//	CClientData* client = m_clientsReference[ademco_id];
//	if (client) {
//		Release(client, FALSE);
//		m_clientsReference[ademco_id] = NULL;
//	}
//}


void CServerService::ReferenceClient(int ademco_id, CClientData* client, BOOL& bTheSameIpPortClientReconnect)
{
	AUTO_LOG_FUNCTION;
	while (!TryEnterCriticalSection(&m_cs4clientReference)) {
		LOG(L"TryEnterCriticalSection failed.\n");
		Sleep(100);
	}
	CClientData* old_client = m_clientsReference[ademco_id];
	if (old_client) {
		//Release(old_client, FALSE);
		// If the new client is the same ip/port with old client,
		// No call Release which will call OnConnectionLost.
		// Trick to fix frequently client offline.

		/*const char* old_ip = inet_ntoa(old_client->foreignAddIn.sin_addr);
		unsigned short old_port = old_client->foreignAddIn.sin_port;
		const char* new_ip = inet_ntoa(client->foreignAddIn.sin_addr);
		unsigned short new_port = client->foreignAddIn.sin_port;
		*/
		//if ((strcmp(old_ip, new_ip) == 0) && (old_port == new_port)) {
			// same client, offline-reconnect, donot show its offline info to user.
			LOG(L"same client, offline-reconnect, donot show its offline info to user.\n");
			//LOG(L"%s:%d\n", old_ip, old_port);
			bTheSameIpPortClientReconnect = TRUE;
		/*} else if(m_handler) {
			m_handler->OnConnectionLost(this, old_client);
			bTheSameIpPortClientReconnect = FALSE;
			LOG(L"different client, show its offline info to user.\n");
			LOG(L"old:%s:%d\n", old_ip, old_port);
			LOG(L"new:%s:%d\n", new_ip, new_port);
		}*/

		LOG(L"new client conn_id %d, ademco_id %04d\n",
			client->conn_id, client->ademco_id);
		shutdown(old_client->socket, 2);
		closesocket(old_client->socket);
		//client->taskList = old_client->taskList;
		old_client->MoveTaskListToNewObj(client);
		old_client->Clear();
		old_client->ResetTime(true);
		InterlockedDecrement(&m_nLiveConnections);
		if (m_nLiveConnections < 0)
			m_nLiveConnections = 0;
		LOG(L"m_nLiveConnections %d ***************************\n", m_nLiveConnections);
	}
	m_clientsReference[ademco_id] = client;
	LeaveCriticalSection(&m_cs4clientReference);
}

//DWORD WINAPI CServerService::ThreadTimeoutChecker(LPVOID lParam)
//{
//	CServerService *server = reinterpret_cast<CServerService*>(lParam);
//	CLog::WriteLog(L"Server service's time out checker now start running.");
//	for (;;) {
//		if (WAIT_OBJECT_0 == WaitForSingleObject(server->m_ShutdownEvent, 1000))
//			break;
//		CLocalLock lock(&server->m_cs4client);
//		for (unsigned int i = 0; i < server->m_nMaxClients; i++) {
//			if (WAIT_OBJECT_0 == WaitForSingleObject(server->m_ShutdownEvent, 0))
//				break;
//			if (CONNID_IDLE != server->m_clients[i].conn_id) {
//				unsigned long lngTimeElapsed = server->m_clients[i].GetTimeElapsed();
//				// check elapsed time, since last registered action,
//				// with the configured time-out.
//				if (lngTimeElapsed > static_cast<unsigned long>(server->m_nTimeoutVal)) {
//					// clear the time and push the socket to 
//					// the IOCP with status _CLOSE. Socket 
//					// will be closed shortly.
//					CLog::WriteLog(L"网络模块连接超时， kick out. conn_id: %d", 
//								   server->m_clients[i].conn_id);
//					server->Release(&server->m_clients[i]);
//				}
//			}
//		}
//	}
//	CLog::WriteLog(L"Server service's time out checker exited.");
//	return 0;
//}

NAMESPACE_END
NAMESPACE_END
