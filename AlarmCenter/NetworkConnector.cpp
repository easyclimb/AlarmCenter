#include "stdafx.h"
#include "NetworkConnector.h"
#include "Client.h"
#include "Server.h"

namespace net
{

IMPLEMENT_SINGLETON(CNetworkConnector)

CNetworkConnector::CNetworkConnector() 
	: m_hEvent(INVALID_HANDLE_VALUE)
	, m_hThread(INVALID_HANDLE_VALUE)
{}


CNetworkConnector::~CNetworkConnector()
{
	client::CClient::ReleaseObject();
	server::CServer::ReleaseObject();
}


BOOL CNetworkConnector::StartNetwork(WORD& listeningPort, 
									 const char* tranmit_server_ip, 
									 WORD transmit_server_port)
{
	AUTO_LOG_FUNCTION;
	int	nRet;
	WSAData	wsData;

	nRet = WSAStartup(MAKEWORD(2, 2), &wsData);
	if (nRet < 0) {
		CLog::WriteLog(L"Can't load winsock.dll.\n");
		return FALSE;
	}

	do {
		if (!server::CServer::GetInstance()->Start(listeningPort))
			break;

		if (tranmit_server_ip && !client::CClient::GetInstance()->Start(tranmit_server_ip, 
			transmit_server_port))
			break;

		//m_hEvent = CreateEvent(nullptr, FALSE, FALSE, nullptr);
		//m_hThread = CreateThread(nullptr, 0, ThreadWorker, this, 0, nullptr);

		return TRUE;
	} while (0);

	return FALSE;
}


void CNetworkConnector::StopNetWork()
{
	AUTO_LOG_FUNCTION;
	//SetEvent(m_hEvent);
	//WaitForSingleObject(m_hThread, INFINITE);
	//CLOSEHANDLE(m_hEvent);
	//CLOSEHANDLE(m_hThread);
	client::CClient::GetInstance()->Stop();
	server::CServer::GetInstance()->Stop();
	WSACleanup();
}


BOOL CNetworkConnector::Send(int ademco_id, int ademco_event, int gg, 
							 int zone, ademco::char_array_ptr xdata)
{
	AUTO_LOG_FUNCTION;
	JLOG(L"ademco_id %04d, ademco_event %04d, gg %02d, zone %03d\n",
		ademco_id, ademco_event, gg, zone);
	server::CServer* server = server::CServer::GetInstance();
	client::CClient* client = client::CClient::GetInstance();

	BOOL ok = FALSE;
	if (server->IsConnectionEstablished()) {
		ok = server->SendToClient(ademco_id, ademco_event, gg, zone, xdata);
	} 
	
	if (!ok && client->IsConnectionEstablished()) {
		ok = client->SendToTransmitServer(ademco_id, ademco_event, gg, zone, xdata);
	}

	JLOG(L"Send ok %d.\n", ok);
	return ok;
}


DWORD WINAPI CNetworkConnector::ThreadWorker(LPVOID lp)
{
	CNetworkConnector* conn = reinterpret_cast<CNetworkConnector*>(lp);
	static const int TP_GAP = 60 * 60 * 1000; // 1 HOUR
	DWORD dwLast = 0;

	while (1) {
		if (WAIT_OBJECT_0 == WaitForSingleObject(conn->m_hEvent, 1000)) {
			break;
		}

		if (GetTickCount() - dwLast > TP_GAP) {
			dwLast = GetTickCount();
			do {
				CLog::WriteLog(L"UpdateWindowsTime\n");

				SOCKET local = socket(AF_INET, SOCK_STREAM, 0);
				if (local == INVALID_SOCKET) {
					CLog::WriteLog(L"socket failed\n");
					break;
				}

				sockaddr_in ntpaddr;
				memset(&ntpaddr, 0, sizeof(ntpaddr));
				ntpaddr.sin_family = AF_INET;
				ntpaddr.sin_addr.S_un.S_addr = inet_addr("198.60.22.240");
				ntpaddr.sin_port = htons(37);

				if (connect(local, (sockaddr*)&ntpaddr, sizeof(sockaddr))) {
					CLog::WriteLog(L"connect failed\n");
					break;
				}

				ULONG ulTime = 0;
				if (recv(local, (char*)&ulTime, sizeof(ulTime), 0) <= 0) {
					CLog::WriteLog(L"recv failed\n");
					break;
				}
				closesocket(local);
				ulTime = ntohl(ulTime);

				FILETIME ft;
				SYSTEMTIME st;
				st.wYear = 1900;
				st.wMonth = 1;
				st.wDay = 1;
				st.wHour = 0;
				st.wMinute = 0;
				st.wSecond = 0;
				st.wMilliseconds = 0;
				SystemTimeToFileTime(&st, &ft);
				LONGLONG *pLLong = (LONGLONG *)&ft;
				*pLLong += (LONGLONG)10000000 * ulTime;
				FileTimeToSystemTime(&ft, &st);
				SetSystemTime(&st);

				CLog::WriteLog(L"new time: %04d-%02d-%02d %02d:%02d:%02d\n",
							   st.wYear, st.wMonth, st.wDay,
							   st.wHour, st.wMinute, st.wSecond);
			} while (0);
		}
	}
	return 0;
}



NAMESPACE_END
