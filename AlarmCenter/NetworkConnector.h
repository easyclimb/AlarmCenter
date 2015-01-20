#pragma once

namespace net
{

class CNetworkConnector
{
public:
	BOOL Send(int ademco_id, int ademco_event, const char* psw = NULL);
	void StopNetWork();
	BOOL StartNetwork(WORD listeningPort, const char* tranmit_server_ip, WORD transmit_server_port);
	static CNetworkConnector* GetInstance() {
		m_lock4Instance.Lock();
		if (m_pInstance == NULL) {
			static CNetworkConnector connector;
			m_pInstance = &connector;
		}
		m_lock4Instance.UnLock();
		return m_pInstance;
	}
	~CNetworkConnector();
private:
	CNetworkConnector();
	static CNetworkConnector* m_pInstance;
	static CLock m_lock4Instance;
};

NAMESPACE_END
