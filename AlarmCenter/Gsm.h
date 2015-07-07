#pragma once
#include "SerialPort.h"
#include "GenericBuffer.h"
#include <string>
#include <list>


typedef struct SendSmsTask
{
	time_t _send_time;
	//std::string _phone;
	char* _content;
	WORD _len;
	SendSmsTask() : _send_time(0),/* _phone(), */_content(NULL), _len(0) {}
	~SendSmsTask() { if (_content) { delete _content; } }
}SendSmsTask;


class CGsm :
	public CSerialPort
{
public:
	BOOL Open(int port);
	void Close();
	void SendSms(std::string& phone, const char* cmd, WORD len);
	void SendSms(std::string& phone, std::string& content);

protected:
	virtual void OnConnectionEstablished() {}
	virtual BOOL OnRecv(const char *cmd, WORD wLen);
	virtual BOOL OnSend(IN char* cmd, IN WORD wLen, OUT WORD& wRealLen);

protected:
	HANDLE m_hEventExit;
	HANDLE m_hThreadWorker;
	CGenericBuffer m_recvBuff;

	std::list<SendSmsTask*> m_taskList;
	CLock m_lock;
	BOOL m_bOpened;
	BOOL m_bWaitingATaskReponce;


	static DWORD WINAPI ThreadWorker(LPVOID lp);

	DECLARE_SINGLETON(CGsm);
	DECLARE_UNCOPYABLE(CGsm);
	//CGsm();
	virtual ~CGsm();
};

