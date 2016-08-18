#pragma once
#include "SerialPort.h"
#include "GenericBuffer.h"
#include <string>
#include <list>
#include <vector>
#include <memory>

namespace ademco { class AdemcoDataSegment; };
namespace core {
	
typedef struct send_sms_task
{
	bool _failed;
	time_t _send_time;
	std::vector<char> _content;
	send_sms_task() : _failed(false), _send_time(0), _content() {}
	~send_sms_task() {}
}send_sms_task;

typedef std::shared_ptr<send_sms_task> send_sms_task_ptr;

class gsm_manager : public dp::singleton<gsm_manager>, public util::CSerialPort
{
public:
	BOOL Open(int port);
	void Close();
	//void SendSms(std::string& phone, const char* cmd, WORD len);
	//void SendSms(std::string& phone, std::string& content);
	void SendSms(const wchar_t* phone, const ademco::AdemcoDataSegment* data, const CString& content);
	virtual ~gsm_manager();
	void test_input(const char* cmd, int len);

protected:
	virtual void OnConnectionEstablished() {}
	virtual BOOL OnRecv(const char *cmd, WORD wLen);
	virtual BOOL OnSend(IN char* cmd, IN WORD wLen, OUT WORD& wRealLen);

protected:


	util::CGenericBuffer m_recvBuff;

	std::list<send_sms_task_ptr> m_taskList;
	std::mutex m_lock;
	BOOL m_bOpened;
	BOOL m_bWaitingATaskReponce;

	bool running_ = true;
	std::thread thread_ = {};
	void ThreadWorker();

	//DECLARE_SINGLETON(gsm_manager);
	//gsm_manager();
	


protected:

	gsm_manager();
};
};


