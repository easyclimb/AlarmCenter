#pragma once

#include "core.h"
#include <list>
//#include <vector>
#include <map>

//using namespace std;

namespace net {
namespace server {

#ifdef _DEBUG
#define BUFF_SIZE 512
#else
#define BUFF_SIZE 1024 
#endif

#define MAX_CLIENTS 100000
#define CONNID_IDLE 0xffffffff
#define THREAD_ACCEPT_NO 1
#define THREAD_RECV_NO 1



typedef struct Task {
	int _retry_times;
	COleDateTime _last_send_time;
	int _seq;
	int _ademco_id;
	int _ademco_event;
	int _gg;
	int _zone;
	char* _xdata;
	int _xdata_len;
	Task() : _retry_times(0), _last_send_time(), _seq(1), _ademco_id(0),
		_ademco_event(0), _gg(0), _zone(0), _xdata(nullptr), _xdata_len(0) {}
	Task(int ademco_id, int ademco_event, int gg, int zone, const char* xdata, int xdata_len) :
		_retry_times(0), _last_send_time(), _seq(1), _ademco_id(ademco_id),
		_ademco_event(ademco_event), _gg(gg), _zone(zone), _xdata(nullptr), _xdata_len(xdata_len) {
		if (xdata && xdata_len > 0) {
			_xdata = new char[xdata_len];
			memcpy(_xdata, xdata, xdata_len);
		}
	}
	~Task() { if (_xdata) delete[] _xdata; _xdata = nullptr; }
}Task;
typedef std::list<Task*> TaskList;



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
	time_t tmLastActionTime;
	//volatile unsigned int conn_id;
	SOCKET socket;
	struct sockaddr_in foreignAddIn;
	bool online;
	bool hangup;
	int ademco_id;
	DATA_BUFF buff;
	TaskList taskList;
	CLock lock4TaskList;
	int cur_seq;
	bool has_data_to_send;
	bool wait_to_kill = false;
	bool disconnectd;
	CClientData() {
		tmLastActionTime = 0;
		Clear();
	}

	~CClientData() {
		Clear();
	}

	void Clear() {
		online = false;
		hangup = false;
		has_data_to_send = false;
		wait_to_kill = false;
		disconnectd = false;
		//conn_id = CONNID_IDLE;
		socket = INVALID_SOCKET;
		memset(&foreignAddIn, 0, sizeof(foreignAddIn));
		ademco_id = CONNID_IDLE;
		buff.Clear();
		for (auto task : taskList) {
			delete task;
		}
		taskList.clear();
		cur_seq = 0;
	}
	void ResetTime(bool toZero)	{
		if (toZero) tmLastActionTime = 0;
		else {
			time_t lLastActionTime;
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

	static void __stdcall OnConnHangup(void* udata, core::RemoteControlCommandConn rccc)
	{
		CClientData* data = reinterpret_cast<CClientData*>(udata);
		if (rccc == core::RCCC_HANGUP)
			data->hangup = true;
		else if (rccc == core::RCCC_RESUME)
			data->hangup = false;
		else if (rccc == core::RCCC_DISCONN)
			data->wait_to_kill = true;
		else {
			ASSERT(0);
		}
	}

	void AddTask(Task* task) {
		AUTO_LOG_FUNCTION;
		lock4TaskList.Lock();
		task->_seq = cur_seq++;
		if (cur_seq == 10000)
			cur_seq = 0;
		taskList.push_back(task);
		has_data_to_send = true;
		lock4TaskList.UnLock();
	}
	Task* GetFirstTask() {
		if (has_data_to_send && lock4TaskList.TryLock()) {
			if (taskList.size() > 0) {
				lock4TaskList.UnLock();
				return taskList.front();
			}	
			lock4TaskList.UnLock();
		}
		return nullptr;
	}
	void RemoveFirstTask() {
		AUTO_LOG_FUNCTION;
		lock4TaskList.Lock();
		if (taskList.size() > 0) {
			Task* task = taskList.front();
			delete task;
			taskList.pop_front();
		}
		if (taskList.size() == 0)
			has_data_to_send = false;
		lock4TaskList.UnLock();
	}
	void MoveTaskListToNewObj(CClientData* client) {
		AUTO_LOG_FUNCTION;
		lock4TaskList.Lock();
		client->cur_seq = cur_seq;
		std::copy(taskList.begin(), taskList.end(), std::back_inserter(client->taskList));
		taskList.clear();
		lock4TaskList.UnLock();
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
	virtual DWORD OnRecv(CServerService *server, CClientData *client, BOOL& resolved) = 0;
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
	inline void SetEventHander(CServerEventHandler* handler) { m_handler = handler; }
private:
	CServerService();
	HANDLE *m_phThreadAccept;
	HANDLE *m_phThreadRecv;
	HANDLE m_ShutdownEvent;
	SOCKET m_ServSock;
	unsigned int m_nMaxClients;
	unsigned int m_nTimeoutVal;
	std::map<int, CClientData*> m_livingClients;
	std::list<CClientData*> m_outstandingClients;
	std::list<CClientData*> m_bufferedClients;
	CServerEventHandler *m_handler;
	CRITICAL_SECTION m_cs4liveingClients;
	CRITICAL_SECTION m_cs4outstandingClients;
protected:
	CClientData* AllocateClient();
	void RecycleClient(CClientData* client);
	bool FindClient(int ademco_id, CClientData** client);
	typedef enum HANDLE_EVENT_RESULT {
		RESULT_CONTINUE,
		RESULT_BREAK,
		RESULT_RECYCLE_AND_BREAK,
	}HANDLE_EVENT_RESULT;
	HANDLE_EVENT_RESULT HandleClientEvents(CClientData* client);
	void RecycleLiveClient(CClientData* client);
public:
	static DWORD WINAPI ThreadAccept(LPVOID lParam);
	static DWORD WINAPI ThreadRecv(LPVOID lParam);
	void Start();
	void Stop();
	void ResolveOutstandingClient(CClientData* client, BOOL& bTheSameIpPortClientReconnect);
	void RecycleOutstandingClient(CClientData* client);
	bool SendToClient(int ademco_id, int ademco_event, int gg, int zone, const char* xdata, int xdata_len);
	bool SendToClient(CClientData* client, const char* data, size_t data_len);	
};

NAMESPACE_END
NAMESPACE_END

