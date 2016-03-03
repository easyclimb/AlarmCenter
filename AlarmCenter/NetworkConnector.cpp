#include "stdafx.h"
#include "NetworkConnector.h"
#include "Client.h"
#include "Server.h"
#include "ConfigHelper.h"

namespace net
{

	namespace detail {
		
	}

IMPLEMENT_SINGLETON(CNetworkConnector)

CNetworkConnector::CNetworkConnector() 
	: m_hEvent(INVALID_HANDLE_VALUE)
	, m_hThread(INVALID_HANDLE_VALUE)
{}


CNetworkConnector::~CNetworkConnector()
{
	g_server = nullptr;
	g_client = nullptr;
	g_client_bk = nullptr;
}


int CNetworkConnector::GetWorkingClientCount() const
{
	int cnt = 0;
	if (g_client) cnt++;
	if (g_client_bk) cnt++;
	return cnt;
}


BOOL CNetworkConnector::StartNetwork()
{
	AUTO_LOG_FUNCTION;
	

	do {
		auto cfg = util::CConfigHelper::GetInstance();
		auto listeningPort = cfg->get_listening_port();

		using namespace detail;

		BOOL ok = true;
		auto mode = cfg->get_network_mode();

		if (mode & util::NETWORK_MODE_CSR) {
			if (g_server == nullptr) {
				g_server = std::make_shared<net::server::CServer>();
			}

			ok = g_server->Start(listeningPort);
			if (ok)
				cfg->set_listening_port(listeningPort);
		}

		if (mode & util::NETWORK_MODE_TRANSMIT) {
			if (g_client == nullptr) {
				g_client = std::make_shared<net::client::CClient>();
			}

			if (g_client_bk == nullptr) {
				g_client_bk = std::make_shared<net::client::CClient>(false);
			}

			BOOL ok1 = FALSE, ok2 = FALSE;
			ok1 = g_client->Start(cfg->get_server1_ip().c_str(), cfg->get_server1_port());
			ok2 = g_client_bk->Start(cfg->get_server2_ip().c_str(), cfg->get_server2_port());
			ok &= (ok1 || ok2);
		}
		//m_hEvent = CreateEvent(nullptr, FALSE, FALSE, nullptr);
		//m_hThread = CreateThread(nullptr, 0, ThreadWorker, this, 0, nullptr);

		return ok;
	} while (0);

	return FALSE;
}


void CNetworkConnector::StopNetwork()
{
	AUTO_LOG_FUNCTION;
	//SetEvent(m_hEvent);
	//WaitForSingleObject(m_hThread, INFINITE);
	//CLOSEHANDLE(m_hEvent);
	//CLOSEHANDLE(m_hThread);
	using namespace detail;
	if (g_client) {
		g_client->Stop();
		g_client = nullptr;
	}

	if (g_client_bk) {
		g_client_bk->Stop();
		g_client_bk = nullptr;
	}

	if (g_server) {
		g_server->Stop();
		g_server = nullptr;
	}

	
}


BOOL CNetworkConnector::RestartClient()
{
	//StopNetwork();
	//return StartNetwork();
	auto cfg = util::CConfigHelper::GetInstance();

	using namespace detail;
	if (g_client) {
		g_client->Stop();
		g_client = nullptr;
	}

	if (g_client_bk) {
		g_client_bk->Stop();
		g_client_bk = nullptr;
	}

	if (g_client == nullptr) {
		g_client = std::make_shared<net::client::CClient>();
	}

	if (g_client_bk == nullptr) {
		g_client_bk = std::make_shared<net::client::CClient>(false);
	}

	BOOL ok1 = FALSE, ok2 = FALSE;
	ok1 = g_client->Start(cfg->get_server1_ip().c_str(), cfg->get_server1_port());
	ok2 = g_client_bk->Start(cfg->get_server2_ip().c_str(), cfg->get_server2_port());

	//m_hEvent = CreateEvent(nullptr, FALSE, FALSE, nullptr);
	//m_hThread = CreateThread(nullptr, 0, ThreadWorker, this, 0, nullptr);

	return (ok1 || ok2);
}


BOOL CNetworkConnector::Send(int ademco_id, int ademco_event, int gg, int zone, 
							 const ademco::char_array_ptr& xdata, 
							 const ademco::char_array_ptr& cmd, 
							 ademco::EventSource path)
{
	AUTO_LOG_FUNCTION;
	JLOG(L"ademco_id %04d, ademco_event %04d, gg %02d, zone %03d\n",
		ademco_id, ademco_event, gg, zone);

	using namespace detail;
	BOOL ok1 = FALSE; BOOL ok2 = FALSE, ok3 = FALSE;
	switch (path)
	{
	case ademco::ES_UNKNOWN:
		if (g_server && g_server->IsConnectionEstablished()) {
			ok1 = g_server->SendToClient(ademco_id, ademco_event, gg, zone, xdata);
		}
		if (g_client && g_client->IsConnectionEstablished()) {
			ok2 = g_client->SendToTransmitServer(ademco_id, ademco_event, gg, zone, xdata, cmd);
		}
		if (g_client_bk && g_client_bk->IsConnectionEstablished()) {
			ok3 = g_client_bk->SendToTransmitServer(ademco_id, ademco_event, gg, zone, xdata, cmd);
		}
		break;
	case ademco::ES_TCP_CLIENT:
		if (g_server && g_server->IsConnectionEstablished()) {
			ok1 = g_server->SendToClient(ademco_id, ademco_event, gg, zone, xdata);
		}
		break;
	case ademco::ES_TCP_SERVER:
		if (g_client && g_client->IsConnectionEstablished()) {
			ok2 = g_client->SendToTransmitServer(ademco_id, ademco_event, gg, zone, xdata, cmd);
		}

		if (g_client_bk && g_client_bk->IsConnectionEstablished()) {
			ok3 = g_client_bk->SendToTransmitServer(ademco_id, ademco_event, gg, zone, xdata, cmd);
		}
		break;
	case ademco::ES_SMS:
		break;
	default:
		break;
	}

	BOOL ok = ok1 || ok2 || ok3;
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
