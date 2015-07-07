#include "stdafx.h"
#include "Gsm.h"

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
{}


BOOL CGsm::Open(int port)
{
	if (!m_bOpened) {
		m_bOpened = InitPort(NULL, port, 115200);
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
		SetEvent(m_hEventExit);
		WaitForSingleObject(m_hThreadWorker, INFINITE);
		CLOSEHANDLE(m_hThreadWorker);
		CLOSEHANDLE(m_hEventExit);
	}
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
			if (difftime(now, task->_send_time) > 30.0) {
				cmd = task->_content;
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
			cmd = task->_content;
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
	CGsm* gsm = reinterpret_cast<CGsm*>(lp);
	char buff[64] = { 0 };
	while (1) {
		if (WAIT_OBJECT_0 == WaitForSingleObject(gsm->m_hEventExit, 3000))
			break;
		if (gsm->m_recvBuff.Read(buff, 1) == 1) {

		}
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

