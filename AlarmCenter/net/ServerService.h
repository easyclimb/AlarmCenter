#pragma once

//#include <vector>
#include <WinSock2.h>
//using namespace std;

namespace net {
#ifdef _DEBUG
#define BUFF_SIZE 512
#else
#define BUFF_SIZE 4096 
#endif
//#define MAX_CLIENTS 10000
#define CONNID_IDLE 0xffffffff
#define THREAD_ACCEPT_NO 1
#define THREAD_RECV_NO 1

class CLIENT
{
	typedef struct _DATA_BUFF
	{
		unsigned int	rpos;
		unsigned int	wpos;
		char			buff[BUFF_SIZE];
		_DATA_BUFF() { memset(this, 0, sizeof(this)); }
		void Clear()
		{
			rpos = wpos = 0;
			memset(buff, 0, sizeof(buff));
		}
	}DATA_BUFF;
public:
	volatile time_t tmLastActionTime;
	volatile unsigned int conn_id;
	SOCKET socket;
	struct sockaddr_in foreignAddIn;
	int ademco_id;
	char acct[64];
	DATA_BUFF buff;

	CLIENT()
	{
		tmLastActionTime = 0;
		Clear();
	}

	void Clear()
	{
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

class CServerService;

class CSockEventHandler
{
public:
	CSockEventHandler() {}
	virtual ~CSockEventHandler() {}
	virtual void Start() = 0;
	virtual void Stop() = 0;
	virtual DWORD OnRecv(CServerService *server, CLIENT *client) = 0;
	virtual void OnConnectionEstablished(CServerService *server, CLIENT *client) = 0;
	virtual void OnConnectionLost(CServerService *server, CLIENT *client) = 0;
};

class CServerService
{
public:
	CServerService(unsigned short nPort, unsigned int nMaxClients, unsigned int nTimeoutVal, bool blnCreateAsync = false, bool blnBindLocal = true);
	~CServerService();
	inline void SetEventHander(CSockEventHandler* handler)
	{
		m_handler = handler;
	}
	//int InitServer(unsigned int nPort, unsigned int nMaxClients, bool blnCreateAsync = false, bool blnBindLocal = true);
private:
	CServerService();
	HANDLE *m_phThreadAccept;
	HANDLE *m_phThreadRecv;
	HANDLE m_hThreadTimeoutChecker;
	HANDLE m_ShutdownEvent;
	SOCKET m_ServSock;
	volatile unsigned int m_nLiveConnections;
	unsigned int m_nMaxClients;
	unsigned int m_nTimeoutVal;
	//vector<CLIENT*> m_clients;
	//typedef vector<CLIENT*>::iterator citer;
	CLIENT *m_clients;
	CSockEventHandler *m_handler;
	CRITICAL_SECTION m_cs;
	CRITICAL_SECTION m_cs4client;
public:
	static DWORD WINAPI ThreadAccept(LPVOID lParam);
	static DWORD WINAPI ThreadRecv(LPVOID lParam);
	static DWORD WINAPI ThreadTimeoutChecker(LPVOID lParam);
	void Start();
	void Stop();
	void Release(CLIENT* client);
	bool SendToClient(unsigned int conn_id, const char* data, size_t data_len);
	bool SendToClient(CLIENT* client, const char* data, size_t data_len);
	bool FindClient(int ademco_id, CLIENT** client) const;
	bool GetClient(unsigned int conn_id, CLIENT** client) const;
	void KillOtherClients(unsigned int conn_id, int ademco_id);
};

NAMESPACE_END
