#pragma once

#include <boost/noncopyable.hpp>

namespace net {
namespace client {
#ifdef _DEBUG
#define BUFF_SIZE 4096
#else
#define BUFF_SIZE 4096
#endif
	
class CClientEventHandler;
class CClientService : public boost::noncopyable
{
	//friend class CMyClientEventHandler;
public:
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
	DATA_BUFF m_buff;
	explicit CClientService(bool main_client = true);
	virtual ~CClientService();
	bool connection_established_ = false;
private:
	int cur_seq_ = 1;
	SOCKET m_socket;
	
	struct sockaddr_in m_server_addr;
	std::shared_ptr<CClientEventHandler> m_handler;
	HANDLE m_hEventShutdown;
	HANDLE m_hThreadRecv;
	//HANDLE m_hThreadReconnectServer;
	//HANDLE m_hThreadLinkTest;
	std::string m_server_ip;
	unsigned int m_server_port;
	volatile BOOL m_bShuttingDown;
	bool main_client_;
	std::list<std::vector<char>> buffer_;
	std::mutex buffer_lock_;
	COleDateTime last_recv_time_;
	COleDateTime disconnected_time_;
	COleDateTime last_conn_time_;
	bool showed_disconnected_info_to_user_ = true;
public:
	int get_cur_seq() { if (cur_seq_ > 9999) cur_seq_ = 1; return cur_seq_++; }
	void PrepairToSend(int ademco_id, const char* buff, size_t buff_size);
	bool main_client() const { return main_client_; }
	//void Restart();
	BOOL Start(const std::string& server_ip, unsigned int server_port);
	void SetEventHandler(std::shared_ptr<CClientEventHandler> handler);
	void Stop();
	
protected:
	void Disconnect();
	int Send(const char* buff, size_t buff_size);
	BOOL Connect();
	static DWORD WINAPI ThreadWorker(LPVOID lp);
	//static DWORD WINAPI ThreadReconnectServer(LPVOID lp);
	//static DWORD WINAPI ThreadLinkTest(LPVOID lp);
};


class CClientEventHandler 
{
public:
	CClientEventHandler() {}
	virtual ~CClientEventHandler() {}
	virtual void OnConnectionEstablished(CClientService* service) = 0;
	virtual void OnConnectionLost(CClientService* service) = 0;
	virtual DWORD OnRecv(CClientService* service) = 0;
	virtual DWORD GenerateLinkTestPackage(char* buff, size_t buff_len) = 0;
	virtual void set_event_source(ademco::EventSource es) { _event_source = es; }
protected:
	volatile ademco::EventSource _event_source = ademco::ES_UNKNOWN;

};


class CClient : public boost::noncopyable
{
public:
	
	BOOL IsConnectionEstablished() const { return m_bClientServiceStarted; }
	explicit CClient(bool main_client = true);
	~CClient() {}
	BOOL Start(const std::string& server_ip, unsigned int server_port);
	void Stop();
	int SendToTransmitServer(int ademco_id, ADEMCO_EVENT ademco_event, int gg, int zone, 
							 const ademco::char_array_ptr& xdata = nullptr,
							 const ademco::char_array_ptr& cmd = nullptr);
	void set_event_source(ademco::EventSource es) { _event_source = es; }
private:
	std::shared_ptr<CClientService> _client_service = nullptr;
	std::shared_ptr<CClientEventHandler> _client_event_handler = nullptr;
	BOOL m_bClientServiceStarted;
	bool main_client_;
	volatile ademco::EventSource _event_source = ademco::ES_UNKNOWN;
};
};};
