#pragma once

namespace net {
namespace client {
#define BUFF_SIZE 4096
class CClientEventHandler;
class CClientService
{
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
	//static CClientService* GetInstance();
	CClientService();
	virtual ~CClientService();
private:
	SOCKET m_socket;
	BOOL m_bConnectionEstablished;
	struct sockaddr_in m_server_addr;
	CClientEventHandler *m_handler;
	HANDLE m_hEventShutdown;
	HANDLE m_hThreadRecv;
	HANDLE m_hThreadReconnectServer;
	HANDLE m_hThreadLinkTest;
	char m_server_ip[64];
	unsigned short m_server_port;
	BOOL m_bShuttingDown;
public:
	void Restart();
	BOOL Start(const char* server_ip, unsigned short server_port);
	void SetEventHandler(CClientEventHandler* handler);
	void Stop();
	int Send(const char* buff, size_t buff_size);
	void Release();
protected:
	static DWORD WINAPI ThreadRecv(LPVOID lp);
	static DWORD WINAPI ThreadReconnectServer(LPVOID lp);
	static DWORD WINAPI ThreadLinkTest(LPVOID lp);
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
};


class CClient
{
public:
	BOOL IsConnectionEstablished() const { return m_bClientServiceStarted; }
	~CClient() {}
	BOOL Start(const char* server_ip, unsigned short server_port);
	void Stop();
	int SendToTransmitServer(int ademco_id, int ademco_event, const char* psw);
private:
	//CClient() : m_bClientServiceStarted(FALSE) {}
	//static CLock m_lock4Instance;
	BOOL m_bClientServiceStarted;
	DECLARE_SINGLETON(CClient)
};

NAMESPACE_END
NAMESPACE_END
