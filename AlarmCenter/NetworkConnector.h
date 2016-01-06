#pragma once

namespace net
{

class CNetworkConnector
{
public:
	BOOL Send(int ademco_id, int ademco_event, int gg,
			  int zone, const ademco::char_array_ptr& xdata = nullptr);

	void StopNetwork();

	BOOL StartNetwork();

	BOOL RestartNetwork();

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
