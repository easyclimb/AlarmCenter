#pragma once

#include "core.h"
#include <list>
//#include <vector>

//using namespace std;

namespace net {
namespace server {

#ifdef _DEBUG
#define BUFF_SIZE 512
#else
#define BUFF_SIZE 4096 
#endif
#define MAX_CLIENTS 100000
#define CONNID_IDLE 0xffffffff
#define THREAD_ACCEPT_NO 1
#define THREAD_RECV_NO 4

typedef struct RemoteControlCommand {
	int _retry_times;
	long long _last_send_time;
	int _seq;
	int _ademco_id;
	int _ademco_event; 
	int _gg; 
	int _zone;
	char* _xdata;
	int _xdata_len;
	RemoteControlCommand() : _retry_times(0), _last_send_time(GetTickCount64()), _seq(0), _ademco_id(0), _ademco_event(0), _gg(0), _zone(0), _xdata(NULL), _xdata_len(0) {}
	~RemoteControlCommand() { if (_xdata) delete[] _xdata; _xdata = NULL; }
}RemoteControlCommand;

typedef std::list<RemoteControlCommand*> RccList;

class CClientData
{
	typedef struct DATA_BUFF
	{
		unsigned int	rpos;
		unsigned int	wpos;
		char			buff[BUFF_SIZE];
		DATA_BUFF() { Clear(); }
		void Clear() { memset(this, 0, sizeof(DATA_BUFF)); }
	}DATA_BUFF;
public:
	volatile time_t tmLastActionTime;
	volatile unsigned int conn_id;
	volatile SOCKET socket;
	struct sockaddr_in foreignAddIn;
	volatile bool online;
	volatile bool hangup;
	volatile int ademco_id;
	char acct[64];
	DATA_BUFF buff;
	//RemoteControlCommand rcc[10000];
	RccList* rccList;

	CClientData() {
		tmLastActionTime = 0;
		rccList = NULL;
		Clear();
	}

	void Clear() {
		online = false;
		hangup = false;
		conn_id = CONNID_IDLE;
		socket = INVALID_SOCKET;
		memset(&foreignAddIn, 0, sizeof(foreignAddIn));
		ademco_id = CONNID_IDLE;
		memset(acct, 0, sizeof(acct));
		buff.Clear();
		if (rccList) {
			std::list<RemoteControlCommand*>::iterator iter = rccList->begin();
			while (iter != rccList->end()) {
				RemoteControlCommand* rcc = *iter++;
				delete rcc;
			}
			rccList->clear();
			delete rccList;
		}
		rccList = NULL;
	}
	void ResetTime(bool toZero)	{
		if (toZero) tmLastActionTime = 0;
		else {
			time_t	lLastActionTime;
			time(&lLastActionTime);
			tmLastActionTime = lLastActionTime;
		}
	};

	unsigned long GetTimeElapsed() {
		time_t tmCurrentTime;
		if (0 == tmLastActionTime) return 0;
		time(&tmCurrentTime);
		return static_cast<unsigned long>(tmCurrentTime - tmLastActionTime);
	};

	static void __stdcall OnConnHangup(void* udata, bool hangup) {
		CClientData* data = reinterpret_cast<CClientData*>(udata);
		data->hangup = hangup;
	}
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
	CServerService(unsigned short& nPort, unsigned int nMaxClients, unsigned int nTimeoutVal, bool blnCreateAsync = false, bool blnBindLocal = true);
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
	CClientData m_clients[MAX_CLIENTS];
	PCClientData m_clientsReference[MAX_CLIENTS];
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
	bool FindClient(int ademco_id, CClientData** client);
	//bool GetClient(unsigned int conn_id, CClientData** client) const;

	// 2015��4��11�� 17:46:11 �ظ��ĺϷ��������ߣ����ߵ����������
	//void UnReferenceClient(int ademco_id);
	void ReferenceClient(int ademco_id, CClientData* client, BOOL& bTheSameIpPortClientReconnect);
};

NAMESPACE_END
NAMESPACE_END

