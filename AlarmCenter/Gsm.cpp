#include "stdafx.h"
#include "Gsm.h"
#include "ademco_func.h"
#include "AlarmMachineManager.h"

#include "C:/dev/Global/win32/UnicodeTool.h"

namespace core {

gsm_manager::gsm_manager()
	: m_recvBuff()
	, m_taskList()
	, m_lock()
	, m_bOpened(FALSE)
	, m_bWaitingATaskReponce(FALSE)
{
	AUTO_LOG_FUNCTION;
}


gsm_manager::~gsm_manager()
{
	AUTO_LOG_FUNCTION;
	Close();
}

void gsm_manager::test_input(const char * cmd, int len)
{
	m_recvBuff.Write(cmd, len);
}


BOOL gsm_manager::Open(int port)
{
	AUTO_LOG_FUNCTION;
	if (!m_bOpened) {
		m_bOpened = InitPort(nullptr, port, 9600);
		if (m_bOpened) {
			StartMonitoring();
			
			running_ = true;
			thread_ = std::thread(&gsm_manager::ThreadWorker, this);

		}
	}
	return m_bOpened;
}


void gsm_manager::Close()
{
	AUTO_LOG_FUNCTION;
	if (m_bOpened) {
		m_bOpened = FALSE;
		ClosePort();
		ReleasePort();
		
		running_ = false;
		thread_.join();
	}

	m_taskList.clear();
}


BOOL gsm_manager::OnRecv(const char *cmd, WORD wLen)
{
	m_recvBuff.Write(cmd, wLen);
	return TRUE;
}


BOOL gsm_manager::OnSend(IN char* cmd, IN WORD wLen, OUT WORD& wRealLen)
{
	if (!m_taskList.empty()) {
		if (m_bWaitingATaskReponce) {
			time_t now = time(nullptr);
			std::lock_guard<std::mutex> lock(m_lock);
			send_sms_task_ptr task = m_taskList.front();
			if (task->_failed || difftime(now, task->_send_time) > 30.0) {
				std::copy(task->_content.begin(), task->_content.end(), cmd);
				wLen = task->_content.size() & 0xFFFF;
				wRealLen = task->_content.size() & 0xFFFF;
				task->_send_time = time(nullptr);
				return TRUE;
			}
		} else {
			std::lock_guard<std::mutex> lock(m_lock);
			send_sms_task_ptr task = m_taskList.front();
			std::copy(task->_content.begin(), task->_content.end(), cmd);
			wLen = task->_content.size() & 0xFFFF;
			wRealLen = task->_content.size() & 0xFFFF;
			task->_send_time = time(nullptr);
			m_bWaitingATaskReponce = TRUE;
			return TRUE;
		}

	}
	return FALSE;
}


void gsm_manager::ThreadWorker()
{
	static const char* SMS_SUCCESS = "SMS_SEND_SUCESS";
	static const char* SMS_FAILED = "SMS_SEND_FAIL";
	static const char* SMS_HEAD = "+CMS:01234567890123456789xxyyzz";
	char buff[1024] = { 0 };
	
	while (running_) {
		std::this_thread::sleep_for(std::chrono::milliseconds(1000));

		if (!running_) {
			break;
		}

		memset(buff, 0, sizeof(buff));
		if (m_recvBuff.Read(buff, 1) == 1) {
			switch (buff[0]) {
			case 'S':
				while (m_recvBuff.GetValidateLen() < strlen(SMS_FAILED) - 1) {
					std::this_thread::sleep_for(std::chrono::milliseconds(100));
					if (!running_)
						break;
				}

				if (!running_) {
					break;
				}

				m_recvBuff.Read(buff + 1, strlen(SMS_FAILED) - 1);
				if (strcmp(SMS_FAILED, buff) == 0) {
					// failed
					std::lock_guard<std::mutex> lock(m_lock);
					send_sms_task_ptr task = m_taskList.front();
					task->_failed = true;
				} else {
					DWORD len2rd = (strlen(SMS_SUCCESS) - strlen(SMS_FAILED));
					while (m_recvBuff.GetValidateLen() < len2rd) {
						std::this_thread::sleep_for(std::chrono::milliseconds(100));
						if (!running_)
							break;
					}

					if (!running_)
						break;

					m_recvBuff.Read(buff + strlen(SMS_FAILED), len2rd);
					if (strcmp(SMS_SUCCESS, buff) == 0) {
						// success
						std::lock_guard<std::mutex> lock(m_lock);
						m_taskList.pop_front();
					}
				}
				break;
			case '+':
				while (m_recvBuff.GetValidateLen() < strlen(SMS_HEAD) - 1) {
					std::this_thread::sleep_for(std::chrono::milliseconds(200));
					if (!running_)
						break;
				}

				if (!running_)
					break;

				m_recvBuff.Read(buff + 1, strlen(SMS_HEAD) - 1);
				if (strncmp(SMS_HEAD, buff, 5) == 0) {
					char phone[20] = { 0 };
					memcpy(phone, buff + 5, 20);
					for (int i = 0; i < 20; i++) {
						std::this_thread::sleep_for(std::chrono::milliseconds(200));

						if (!running_)
							break;

						if (phone[i] == ' ') {
							phone[i] = 0;
							break;
						}
					}

					if (!running_)
						break;

					char* pos = buff + strlen(SMS_HEAD);
					while (m_recvBuff.Read(pos, 1) == 1) {
						std::this_thread::sleep_for(std::chrono::milliseconds(200));

						if (!running_)
							break;

						if (*pos == '\r') {
							*pos = 0;
							break;
						}
						pos++;
					}

					if (!running_)
						break;

					std::string sphone(phone);
					std::string content(buff + strlen(SMS_HEAD));
					std::string txt = sphone + ":" + content;
					JLOGA(txt.c_str());

					for (auto&& c : content) {
						if (c != '[' && c != ']' && c != '#' && c != ' ' && !('0' <= c && c <= '9') && !('A' <= c && c <= 'Z') && !('a' <= c && c <= 'z')) {
							c = '|';
						}
					}

					txt = sphone + ":" + content;
					JLOGA(txt.c_str());

					do {
						if (content.front() != '#') {
							break;
						}
						if (content.length() < 10) {
							break;
						}
						auto data_head = content.substr(0, 8);

						auto s_len = content.at(8);
						size_t len = s_len - '0';
						if (len < 0 || len > 9)
							break;

						if (10 + len * 11 > content.length())
							break;

						std::list<std::string> data_array;
						for (size_t i = 0; i < len; i++) {
							auto data = content.substr(10 + i * 12, 11);
							data_array.push_back('[' + data_head + data + ']');
						}

						ademco::AdemcoDataSegment parser;
						for (auto data : data_array) {
							if (parser.Parse(data.c_str(), data.size())) {
								auto mgr = alarm_machine_manager::get_instance();
								if (mgr->CheckIsValidMachine(parser._ademco_id, parser._zone)) {
									auto t = time(nullptr);
									mgr->MachineEventHandler(ademco::ES_SMS,
															 parser._ademco_id,
															 parser._ademco_event,
															 parser._zone,
															 parser._gg,
															 t, t);
								}
							}
						}
					} while (false);
				}
				break;
			default:
				break;
			}
		}
	}
}

//
//void gsm_manager::SendSms(std::string& phone, const char* cmd, WORD len)
//{
//	send_sms_task* task = new send_sms_task();
//	task->_len = static_cast<WORD>(phone.size() + 3 + len);
//	task->_content = new char[task->_len];
//	memcpy(task->_content, phone.c_str(), phone.size());
//	memcpy(task->_content + phone.size(), ":1:", 3);
//	memcpy(task->_content + phone.size() + 3, cmd, len);
//
//	m_lock.Lock();
//	m_taskList.push_back(task);
//	m_lock.UnLock();
//}
//
//
//void gsm_manager::SendSms(std::string& phone, std::string& content)
//{
//	send_sms_task* task = new send_sms_task();
//	task->_len = static_cast<WORD>(phone.size() + 3 + content.size());
//	task->_content = new char[task->_len];
//	memcpy(task->_content, phone.c_str(), phone.size());
//	memcpy(task->_content + phone.size(), ":0:", 3);
//	memcpy(task->_content + phone.size() + 3, content.c_str(), content.size());
//
//	m_lock.Lock();
//	m_taskList.push_back(task);
//	m_lock.UnLock();
//}


void gsm_manager::SendSms(const wchar_t* wphone, const ademco::AdemcoDataSegment* data, const CString& wcontent)
{
	send_sms_task_ptr task = std::make_shared<send_sms_task>();
	auto a = std::unique_ptr<char[]>(Utf16ToAnsi(wphone));
	auto b = std::unique_ptr<char[]>(Utf16ToAnsi(wcontent));
	std::string phone(a.get());
	std::string content(b.get());
	//task->_len = static_cast<WORD>(phone.size() + 3 + data->_len + content.size());
	//task->_content = new char[task->_len];
	//memcpy(task->_content, phone.c_str(), phone.size());
	std::copy(phone.begin(), phone.end(), std::back_inserter(task->_content));
	//memcpy(task->_content + phone.size(), ":0:", 3);
	task->_content.push_back(':');
	task->_content.push_back('0');
	task->_content.push_back(':');
	//memcpy(task->_content + phone.size() + 3, &data->_data[0], data->_len);
	std::copy(data->_data.begin(), data->_data.end(), std::back_inserter(task->_content));
	//memcpy(task->_content + phone.size() + 3 + data->_len, content.c_str(), content.size());
	std::copy(content.begin(), content.end(), std::back_inserter(task->_content));

	std::lock_guard<std::mutex> lock(m_lock);
	m_taskList.push_back(task);
}

};


