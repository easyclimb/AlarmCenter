#pragma once

namespace net
{

class CNetworkConnector
{
public:
	BOOL Send(int ademco_id, int ademco_event, const char* xdata = NULL);
	void StopNetWork();
	BOOL StartNetwork(WORD listeningPort, const char* tranmit_server_ip, WORD transmit_server_port);
	~CNetworkConnector();
private:
	//CNetworkConnector();
	//static CNetworkConnector* m_pInstance;
	//static CLock m_lock4Instance;
	HANDLE m_hEvent;
	HANDLE m_hThread;
	static DWORD WINAPI ThreadWorker(LPVOID lp);
	DECLARE_SINGLETON(CNetworkConnector)
};

NAMESPACE_END
