#pragma once

#include "core.h"
#include <list>
//#include <vector>
#include <map>
#include <memory>

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
	ademco::char_array_ptr _xdata;

	Task() : _retry_times(0), _last_send_time(), _seq(1), _ademco_id(0),
		_ademco_event(0), _gg(0), _zone(0), _xdata()
	{}

	Task(int ademco_id, int ademco_event, int gg, int zone, const ademco::char_array_ptr& xdata) :
		_retry_times(0), _last_send_time(), _seq(1), _ademco_id(ademco_id),
		_ademco_event(ademco_event), _gg(gg), _zone(zone), _xdata(xdata) 
	{}

	~Task() {}

}Task;

typedef std::shared_ptr<Task> TaskPtr;
typedef std::list<TaskPtr> TaskList;



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
	std::mutex lock4TaskList;
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
		socket = INVALID_SOCKET;
		memset(&foreignAddIn, 0, sizeof(foreignAddIn));
		ademco_id = CONNID_IDLE;
		buff.Clear();
		taskList.clear();
		cur_seq = 0;
	}

	void ResetTime(bool toZero)	{
		if (toZero) {
			tmLastActionTime = 0;
		} else {
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

	static void __stdcall OnConnHangup(CClientDataPtr client, core::RemoteControlCommandConn rccc)
	{
		if (rccc == core::RCCC_HANGUP)
			client->hangup = true;
		else if (rccc == core::RCCC_RESUME)
			client->hangup = false;
		else if (rccc == core::RCCC_DISCONN)
			client->wait_to_kill = true;
		else {
			ASSERT(0);
		}
	}

	void AddTask(TaskPtr task) {
		AUTO_LOG_FUNCTION;
		std::lock_guard<std::mutex> lock(lock4TaskList);
		task->_seq = cur_seq++;
		if (cur_seq == 10000)
			cur_seq = 0;
		taskList.push_back(task);
		has_data_to_send = true;
	}

	TaskPtr GetFirstTask() {
		if (has_data_to_send && lock4TaskList.try_lock()) {
			std::lock_guard<std::mutex> lock(lock4TaskList, std::adopt_lock);
			if (!taskList.empty()) {
				return taskList.front();
			}	
		}
		return nullptr;
	}

	void RemoveFirstTask() {
		AUTO_LOG_FUNCTION;
		std::lock_guard<std::mutex> lock(lock4TaskList);
		if (taskList.size() > 0) {
			taskList.pop_front();
		}
		if (taskList.size() == 0)
			has_data_to_send = false;
	}

	void MoveTaskListToNewObj(const net::server::CClientDataPtr& client) {
		AUTO_LOG_FUNCTION;
		std::lock_guard<std::mutex> lock(lock4TaskList);
		client->cur_seq = cur_seq;
		std::copy(taskList.begin(), taskList.end(), std::back_inserter(client->taskList));
		taskList.clear();
	}
};


class CServerService;
class CServerEventHandler
{
public:
	CServerEventHandler() {}
	virtual ~CServerEventHandler() {}
	virtual void Start() = 0;
	virtual void Stop() = 0;
	virtual DWORD OnRecv(CServerService *server, const net::server::CClientDataPtr& client, BOOL& resolved) = 0;
	virtual void OnConnectionEstablished(CServerService *server, const net::server::CClientDataPtr& client) = 0;
	virtual void OnConnectionLost(CServerService *server, const net::server::CClientDataPtr& client) = 0;
};

class CServerService
{
	typedef struct THREAD_PARAM {
		CServerService* service;
		int thread_no;
	}THREAD_PARAM;

public:
	CServerService(unsigned int& nPort,
				   unsigned int nMaxClients,
				   unsigned int nTimeoutVal, 
				   bool blnCreateAsync = false,
				   bool blnBindLocal = true);

	~CServerService();

	inline void SetEventHander(const std::shared_ptr<CServerEventHandler>& handler) { m_handler = handler; }
private:
	CServerService();
	HANDLE *m_phThreadAccept;
	HANDLE *m_phThreadRecv;
	HANDLE m_ShutdownEvent;
	SOCKET m_ServSock;
	unsigned int m_nMaxClients;
	unsigned int m_nTimeoutVal;
	std::map<int, CClientDataPtr> m_livingClients;
	std::list<CClientDataPtr> m_outstandingClients;
	std::list<CClientDataPtr> m_bufferedClients;
	std::shared_ptr<CServerEventHandler> m_handler;
	std::mutex m_cs4liveingClients;
	std::mutex m_cs4outstandingClients;
protected:
	CClientDataPtr AllocateClient();
	void RecycleClient(const net::server::CClientDataPtr& client);
	CClientDataPtr FindClient(int ademco_id);
	typedef enum HANDLE_EVENT_RESULT {
		RESULT_CONTINUE,
		RESULT_BREAK,
		RESULT_RECYCLE_AND_BREAK,
	}HANDLE_EVENT_RESULT;
	HANDLE_EVENT_RESULT HandleClientEvents(const net::server::CClientDataPtr& client);
	void RecycleLiveClient(const net::server::CClientDataPtr& client, BOOL bShowOfflineInfo);
public:
	static DWORD WINAPI ThreadAccept(LPVOID lParam);
	static DWORD WINAPI ThreadRecv(LPVOID lParam);
	void Start();
	void Stop();
	void ResolveOutstandingClient(const net::server::CClientDataPtr& client, BOOL& bTheSameIpPortClientReconnect);
	void RecycleOutstandingClient(const net::server::CClientDataPtr& client);
	bool SendToClient(int ademco_id, int ademco_event, int gg, int zone, const ademco::char_array_ptr& xdata = nullptr);
	bool SendToClient(const net::server::CClientDataPtr& client, const char* data, size_t data_len);
};

NAMESPACE_END
NAMESPACE_END

