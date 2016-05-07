#pragma once

namespace net
{
namespace client {
class CClient;
}

namespace server {
class CServer;
}

enum restart_server_number {
	server_1 = 1,
	server_2 = 2,
	all = 3,
};

class CNetworkConnector
{
public:
	int GetWorkingClientCount() const;

	BOOL Send(int ademco_id, int ademco_event, int gg, int zone,
			  const ademco::char_array_ptr& xdata = nullptr,
			  const ademco::char_array_ptr& cmd = nullptr,
			  ademco::EventSource path = ademco::ES_UNKNOWN);

	void StopNetwork();

	BOOL StartNetwork();

	BOOL RestartClient(restart_server_number number);

	~CNetworkConnector();
private:
	std::shared_ptr<net::client::CClient> g_client = nullptr;
	std::shared_ptr<net::client::CClient> g_client_bk = nullptr;
	std::shared_ptr<net::server::CServer> g_server = nullptr;
	//CNetworkConnector();
	//static CNetworkConnector* m_pInstance;
	//static std::mutex m_lock4Instance;
	HANDLE m_hEvent;
	HANDLE m_hThread;
	static DWORD WINAPI ThreadWorker(LPVOID lp);
	DECLARE_SINGLETON(CNetworkConnector)
};

NAMESPACE_END
