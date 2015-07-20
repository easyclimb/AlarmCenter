#include "stdafx.h"
#include "Gsm.h"


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
{}


CGsm::~CGsm()
{
	Close();
}


BOOL CGsm::Open(int port)
{
	if (!m_bOpened) {
		m_bOpened = InitPort(NULL, port, 9600);
		if (m_bOpened) {
			StartMonitoring();
			m_hEventExit = CreateEvent(NULL, TRUE, FALSE, NULL);
			m_hThreadWorker = CreateThread(NULL, 0, ThreadWorker, this, 0, NULL);
		}
	}
	return m_bOpened;
}


void CGsm::Close()
{
	if (m_bOpened) {
		m_bOpened = FALSE;
		ClosePort();
		SetEvent(m_hEventExit);
		WaitForSingleObject(m_hThreadWorker, INFINITE);
		CLOSEHANDLE(m_hThreadWorker);
		CLOSEHANDLE(m_hEventExit);
	}

	std::list<SendSmsTask*>::iterator iter = m_taskList.begin();
	while (iter != m_taskList.end()) {
		SendSmsTask* task = *iter++;
		delete task;
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
	if (m_taskList.size() > 0) {
		if (m_bWaitingATaskReponce) {
			time_t now = time(NULL);
			m_lock.Lock();
			SendSmsTask* task = m_taskList.front();
			if (task->_failed || difftime(now, task->_send_time) > 30.0) {
				memcpy(cmd, task->_content, task->_len);
				//cmd = task->_content;
				wLen = task->_len;
				wRealLen = task->_len;
				task->_send_time = time(NULL);
				m_lock.UnLock();
				return TRUE;
			}
			m_lock.UnLock();
		} else {
			m_lock.Lock();
			SendSmsTask* task = m_taskList.front();
			memcpy(cmd, task->_content, task->_len);
			//cmd = task->_content;
			wLen = task->_len;
			wRealLen = task->_len;
			task->_send_time = time(NULL);
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
						SendSmsTask* task = gsm->m_taskList.front();
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
							SendSmsTask* task = gsm->m_taskList.front();
							gsm->m_taskList.pop_front();
							delete task;
							gsm->m_lock.UnLock();
						}
					}
					break;
				case '+':
					while (gsm->m_recvBuff.GetValidateLen() < strlen(SMS_HEAD) - 1) {
						if (WAIT_OBJECT_0 == WaitForSingleObject(gsm->m_hEventExit, 100))
							break;
					}
					if (WAIT_OBJECT_0 == WaitForSingleObject(gsm->m_hEventExit, 0))
						break;
					gsm->m_recvBuff.Read(buff + 1, strlen(SMS_HEAD) - 1);
					if (strncmp(SMS_HEAD, buff, 5) == 0) {
						char phone[20] = { 0 };
						memcpy(phone, buff + 5, 20);
						for (int i = 0; i < 20; i++) {
							if (phone[i] == ' ') {
								phone[i] = 0;
								break;
							}
						}
						char* pos = buff + strlen(SMS_HEAD);
						while (gsm->m_recvBuff.Read(pos, 1) == 1) {
							if (*pos == '\r') {
								*pos = 0;
								break;
							}
							pos++;
						}
						
						std::string content(phone);
						content.push_back(':');
						content += buff + strlen(SMS_HEAD);
						CString c = A2W(content.c_str());
						LOG(c);
					}
					break;
				default:
					break;
			}
		}

		if (WAIT_OBJECT_0 == WaitForSingleObject(gsm->m_hEventExit, 0))
			break;
	}
	return 0;
}


void CGsm::SendSms(std::string& phone, const char* cmd, WORD len)
{
	SendSmsTask* task = new SendSmsTask();
	task->_len = static_cast<WORD>(phone.size() + 3 + len);
	task->_content = new char[task->_len];
	memcpy(task->_content, phone.c_str(), phone.size());
	memcpy(task->_content + phone.size(), ":1:", 3);
	memcpy(task->_content + phone.size() + 3, cmd, len);

	m_lock.Lock();
	m_taskList.push_back(task);
	m_lock.UnLock();
}


void CGsm::SendSms(std::string& phone, std::string& content)
{
	SendSmsTask* task = new SendSmsTask();
	task->_len = static_cast<WORD>(phone.size() + 3 + content.size());
	task->_content = new char[task->_len];
	memcpy(task->_content, phone.c_str(), phone.size());
	memcpy(task->_content + phone.size(), ":0:", 3);
	memcpy(task->_content + phone.size() + 3, content.c_str(), content.size());

	m_lock.Lock();
	m_taskList.push_back(task);
	m_lock.UnLock();
}


void CGsm::SendSms(const CString& wphone, const CString& wcontent)
{
	SendSmsTask* task = new SendSmsTask();
	std::string phone(Utf16ToAnsi(wphone));
	std::string content(Utf16ToAnsi(wcontent));
	task->_len = static_cast<WORD>(phone.size() + 3 + content.size());
	task->_content = new char[task->_len];
	memcpy(task->_content, phone.c_str(), phone.size());
	memcpy(task->_content + phone.size(), ":0:", 3);
	memcpy(task->_content + phone.size() + 3, content.c_str(), content.size());

	m_lock.Lock();
	m_taskList.push_back(task);
	m_lock.UnLock();
}

NAMESPACE_END

