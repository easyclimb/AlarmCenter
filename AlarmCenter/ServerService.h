#pragma once

//#include <vector>

//using namespace std;

namespace net {
namespace server {

#ifdef _DEBUG
#define BUFF_SIZE 512
#else
#define BUFF_SIZE 4096 
#endif
//#define MAX_CLIENTS 10000
#define CONNID_IDLE 0xffffffff
#define THREAD_ACCEPT_NO 1
#define THREAD_RECV_NO 4

class CClientData
{
	typedef struct DATA_BUFF
	{
		unsigned int	rpos;
		unsigned int	wpos;
		char			buff[BUFF_SIZE];
		DATA_BUFF() { Clear(); }
		void Clear()
		{
			memset(this, 0, sizeof(DATA_BUFF));
		}
	}DATA_BUFF;
public:
	volatile time_t tmLastActionTime;
	volatile unsigned int conn_id;
	SOCKET socket;
	struct sockaddr_in foreignAddIn;
	bool online;
	int ademco_id;
	char acct[64];
	DATA_BUFF buff;

	CClientData()
	{
		tmLastActionTime = 0;
		Clear();
	}

	void Clear()
	{
		online = false;
		conn_id = CONNID_IDLE;
		socket = INVALID_SOCKET;
		memset(&foreignAddIn, 0, sizeof(foreignAddIn));
		ademco_id = CONNID_IDLE;
		memset(acct, 0, sizeof(acct));
		buff.Clear();
	}

	void ResetTime(bool toZero)
	{
		if (toZero) tmLastActionTime = 0;
		else {
			time_t	lLastActionTime;
			time(&lLastActionTime);
			tmLastActionTime = lLastActionTime;
		}
	};

	unsigned long GetTimeElapsed()
	{
		time_t tmCurrentTime;

		if (0 == tmLastActionTime) return 0;

		time(&tmCurrentTime);
		return static_cast<unsigned long>(tmCurrentTime - tmLastActionTime);
	};
};

typedef CClientData* PCClientData;

class CServerService;

class CServerEventHandler
{
public:
	CServerEventHandler() {}
	virtual ~CServerEventHandler() {}
	virtual void Start() = 0;
	virtual void Stop() = 0;
	virtual DWORD OnRecv(CServerService *server, CClientData *client) = 0;
	virtual void OnConnectionEstablished(CServerService *server, CClientData *client) = 0;
	virtual void OnConnectionLost(CServerService *server, CClientData *client) = 0;
};

class CServerService
{
	typedef struct THREAD_PARAM {
		CServerService* service;
		int thread_no;
	}THREAD_PARAM;
public:
	CServerService(unsigned short nPort, unsigned int nMaxClients, unsigned int nTimeoutVal, bool blnCreateAsync = false, bool blnBindLocal = true);
	~CServerService();
	inline void SetEventHander(CServerEventHandler* handler)
	{
		m_handler = handler;
	}
	//int InitServer(unsigned int nPort, unsigned int nMaxClients, bool blnCreateAsync = false, bool blnBindLocal = true);
private:
	CServerService();
	HANDLE *m_phThreadAccept;
	HANDLE *m_phThreadRecv;
	//HANDLE m_hThreadTimeoutChecker;
	HANDLE m_ShutdownEvent;
	SOCKET m_ServSock;
	volatile unsigned int m_nLiveConnections;
	unsigned int m_nMaxClients;
	unsigned int m_nTimeoutVal;
	//vector<CClientData*> m_clients;
	//typedef vector<CClientData*>::iterator citer;
	CClientData *m_clients;
	PCClientData *m_clientsReference;
	CServerEventHandler *m_handler;
	CRITICAL_SECTION m_cs;
	CRITICAL_SECTION m_cs4client;
	CRITICAL_SECTION m_cs4clientReference;
public:
	static DWORD WINAPI ThreadAccept(LPVOID lParam);
	static DWORD WINAPI ThreadRecv(LPVOID lParam);
	//static DWORD WINAPI ThreadTimeoutChecker(LPVOID lParam);
	void Start();
	void Stop();
	void Release(CClientData* client, BOOL bNeed2UnReference = TRUE);
	bool SendToClient(unsigned int conn_id, const char* data, size_t data_len);
	bool SendToClient(CClientData* client, const char* data, size_t data_len);
	bool FindClient(int ademco_id, CClientData** client) const;
	bool GetClient(unsigned int conn_id, CClientData** client) const;

	// 2015年4月11日 17:46:11 重复的合法主机上线，将踢掉较早的链接
	//void UnReferenceClient(int ademco_id);
	void ReferenceClient(int ademco_id, CClientData* client);
};

NAMESPACE_END
NAMESPACE_END

