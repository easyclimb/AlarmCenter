#pragma once
#include "SerialPort.h"
#include "GenericBuffer.h"
#include "ademco_func.h"
#include "ademco_event.h"
#include <map>

namespace core {

class congwin_fe100_mgr : public dp::singleton<congwin_fe100_mgr>, public util::CSerialPort
{
public:
	
	virtual ~congwin_fe100_mgr();
	BOOL Open(int port);
	void Close();

	void add_event(const ademco::AdemcoDataSegment* data);

protected:
	congwin_fe100_mgr();

	virtual void OnConnectionEstablished() {}
	virtual BOOL OnRecv(const char *cmd, WORD wLen);
	virtual BOOL OnSend(IN char* cmd, IN WORD wLen, OUT WORD& wRealLen);


	util::CGenericBuffer m_recvBuff = {};

	typedef std::pair<std::chrono::steady_clock::time_point, std::shared_ptr<ademco::AdemcoDataSegment>> data_time_pair;
	typedef std::list<data_time_pair> data_times;
	typedef std::shared_ptr<data_times> data_times_ptr;
	//std::list<std::shared_ptr<ademco::AdemcoDataSegment>> data_buffer_ = {};
	std::list<std::shared_ptr<ademco::AdemcoDataSegment>> data_buffer_ = {};
	std::map<int, data_times_ptr> data_buffer_filter_ = {};
	std::mutex m_lock = {};
	BOOL m_bOpened = FALSE;
	BOOL m_bWaitingATaskReponce = FALSE;
	std::chrono::steady_clock::time_point last_time_send_ = {};
	bool time_to_send_heart_beat_ = false;
	bool running_ = true;
	std::thread thread_ = {};
	void worker();
};

}
