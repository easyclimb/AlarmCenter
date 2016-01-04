#include "stdafx.h"
#include "Gsm.h"
#include "ademco_func.h"
#include "AlarmMachineManager.h"

namespace core {

IMPLEMENT_SINGLETON(CGsm)
CGsm::CGsm()
	: m_hEventExit(INVALID_HANDLE_VALUE)
	, m_hThreadWorker(INVALID_HANDLE_VALUE)
	, m_recvBuff()
	, m_taskList()
	, m_lock()
	, m_bOpened(FALSE)
	, m_bWaitingATaskReponce(FALSE)
{
	AUTO_LOG_FUNCTION;
}


CGsm::~CGsm()
{
	AUTO_LOG_FUNCTION;
	Close();
}


BOOL CGsm::Open(int port)
{
	AUTO_LOG_FUNCTION;
	if (!m_bOpened) {
		m_bOpened = InitPort(nullptr, port, 9600);
		if (m_bOpened) {
			StartMonitoring();
			m_hEventExit = CreateEvent(nullptr, TRUE, FALSE, nullptr);
			m_hThreadWorker = CreateThread(nullptr, 0, ThreadWorker, this, 0, nullptr);
		}
	}
	return m_bOpened;
}


void CGsm::Close()
{
	AUTO_LOG_FUNCTION;
	if (m_bOpened) {
		m_bOpened = FALSE;
		ClosePort();
		SetEvent(m_hEventExit);
		WaitForSingleObject(m_hThreadWorker, INFINITE);
		CLOSEHANDLE(m_hThreadWorker);
		CLOSEHANDLE(m_hEventExit);
	}

	m_taskList.clear();
}


BOOL CGsm::OnRecv(const char *cmd, WORD wLen)
{
	m_recvBuff.Write(cmd, wLen);
	return TRUE;
}


BOOL CGsm::OnSend(IN char* cmd, IN WORD wLen, OUT WORD& wRealLen)
{
	if (!m_taskList.empty()) {
		if (m_bWaitingATaskReponce) {
			time_t now = time(nullptr);
			m_lock.Lock();
			SendSmsTaskPtr task = m_taskList.front();
			if (task->_failed || difftime(now, task->_send_time) > 30.0) {
				//memcpy(cmd, task->_content, task->_len);
				std::copy(task->_content.begin(), task->_content.end(), cmd);
				//cmd = task->_content;
				wLen = task->_content.size() & 0xFFFF;
				wRealLen = task->_content.size() & 0xFFFF;
				task->_send_time = time(nullptr);
				m_lock.UnLock();
				return TRUE;
			}
			m_lock.UnLock();
		} else {
			m_lock.Lock();
			SendSmsTaskPtr task = m_taskList.front();
			//memcpy(cmd, task->_content, task->_len);
			std::copy(task->_content.begin(), task->_content.end(), cmd);
			//cmd = task->_content;
			wLen = task->_content.size() & 0xFFFF;
			wRealLen = task->_content.size() & 0xFFFF;
			task->_send_time = time(nullptr);
			m_bWaitingATaskReponce = TRUE;
			m_lock.UnLock();
			return TRUE;
		}
		
	}
	return FALSE;
}


DWORD WINAPI CGsm::ThreadWorker(LPVOID lp)
{
	USES_CONVERSION;
	static const char* SMS_SUCCESS = "SMS_SEND_SUCESS";
	static const char* SMS_FAILED = "SMS_SEND_FAIL";
	static const char* SMS_HEAD = "+CMS:01234567890123456789xxyyzz";
	CGsm* gsm = reinterpret_cast<CGsm*>(lp);
	char buff[1024] = { 0 };
	while (1) {
		if (WAIT_OBJECT_0 == WaitForSingleObject(gsm->m_hEventExit, 1000))
			break;
		memset(buff, 0, sizeof(buff));
		if (gsm->m_recvBuff.Read(buff, 1) == 1) {
			switch (buff[0]) {
				case 'S':
					while (gsm->m_recvBuff.GetValidateLen() < strlen(SMS_FAILED) - 1) {
						if (WAIT_OBJECT_0 == WaitForSingleObject(gsm->m_hEventExit, 100))
							break;
					}
					if (WAIT_OBJECT_0 == WaitForSingleObject(gsm->m_hEventExit, 0))
						break;
					gsm->m_recvBuff.Read(buff + 1, strlen(SMS_FAILED) - 1);
					if (strcmp(SMS_FAILED, buff) == 0) {
						// failed
						gsm->m_lock.Lock();
						SendSmsTaskPtr task = gsm->m_taskList.front();
						task->_failed = true;
						gsm->m_lock.UnLock();
					} else {
						DWORD len2rd = (strlen(SMS_SUCCESS) - strlen(SMS_FAILED));
						while (gsm->m_recvBuff.GetValidateLen() < len2rd) {
							if (WAIT_OBJECT_0 == WaitForSingleObject(gsm->m_hEventExit, 100))
								break;
						}
						if (WAIT_OBJECT_0 == WaitForSingleObject(gsm->m_hEventExit, 0))
							break;
						gsm->m_recvBuff.Read(buff + strlen(SMS_FAILED), len2rd);
						if (strcmp(SMS_SUCCESS, buff) == 0) {
							// success
							gsm->m_lock.Lock();
							gsm->m_taskList.pop_front();
							gsm->m_lock.UnLock();
						}
					}
					break;
				case '+':
					while (gsm->m_recvBuff.GetValidateLen() < strlen(SMS_HEAD) - 1) {
						if (WAIT_OBJECT_0 == WaitForSingleObject(gsm->m_hEventExit, 200))
							break;
					}
					if (WAIT_OBJECT_0 == WaitForSingleObject(gsm->m_hEventExit, 100))
						break;
					gsm->m_recvBuff.Read(buff + 1, strlen(SMS_HEAD) - 1);
					if (strncmp(SMS_HEAD, buff, 5) == 0) {
						char phone[20] = { 0 };
						memcpy(phone, buff + 5, 20);
						for (int i = 0; i < 20; i++) {
							if (WAIT_OBJECT_0 == WaitForSingleObject(gsm->m_hEventExit, 200))
								break;
							if (phone[i] == ' ') {
								phone[i] = 0;
								break;
							}
						}
						char* pos = buff + strlen(SMS_HEAD);
						while (gsm->m_recvBuff.Read(pos, 1) == 1) {
							if (WAIT_OBJECT_0 == WaitForSingleObject(gsm->m_hEventExit, 200))
								break;
							if (*pos == '\r') {
								*pos = 0;
								break;
							}
							pos++;
						}
						
						std::string sphone(phone);
						std::string content(buff + strlen(SMS_HEAD));
						std::string txt = sphone + ":" + content;
						JLOGA(txt.c_str());

						if (content.front() != '[') {
							content.insert(content.begin(), '[');
						}

						if (content.back() != ']') {
							content.push_back(']');
						}

						for (auto&& c : content) {
							if (c != '[' && c != ']' && c != '#' && c != ' ' && !('0' <= c && c <= '9') && !('A' <= c && c <= 'Z') && !('a' <= c && c <= 'z')) {
								c = '|';
								break;
							}
						}

						txt = sphone + ":" + content;
						JLOGA(txt.c_str());

						ademco::AdemcoDataSegment data;
						if (data.Parse(content.c_str(), content.size())) {
							CAlarmMachineManager* mgr = CAlarmMachineManager::GetInstance();
							if (mgr->CheckIsValidMachine(data._ademco_id, data._zone)) {
								auto t = time(nullptr);
								mgr->MachineEventHandler(ademco::ES_SMS,
														 data._ademco_id,
														 data._ademco_event,
														 data._zone,
														 data._gg,
														 t, t);
							}
						}
					}
					break;
				default:
					break;
			}
		}

		if (WAIT_OBJECT_0 == WaitForSingleObject(gsm->m_hEventExit, 100))
			break;
	}
	return 0;
}

//
//void CGsm::SendSms(std::string& phone, const char* cmd, WORD len)
//{
//	SendSmsTask* task = new SendSmsTask();
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
//void CGsm::SendSms(std::string& phone, std::string& content)
//{
//	SendSmsTask* task = new SendSmsTask();
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


void CGsm::SendSms(const wchar_t* wphone, const ademco::AdemcoDataSegment* data, const CString& wcontent)
{
	SendSmsTaskPtr task = std::make_shared<SendSmsTask>();
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

	m_lock.Lock();
	m_taskList.push_back(task);
	m_lock.UnLock();
}

NAMESPACE_END

