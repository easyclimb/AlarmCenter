#include "stdafx.h"
#include "BaiduMapProcessMgr.h"
namespace core
{
	IMPLEMENT_SINGLETON(CBaiduMapProcessMgr);
	CBaiduMapProcessMgr::CBaiduMapProcessMgr()
		: m_hThread(INVALID_HANDLE_VALUE)
		, m_hEvent(INVALID_HANDLE_VALUE)
		, m_hChildProcess(INVALID_HANDLE_VALUE)
		, m_hChildThread(INVALID_HANDLE_VALUE)
		
	{
		m_hEvent = CreateEvent(nullptr, FALSE, FALSE, nullptr);
		m_hThread = CreateThread(nullptr, 0, ThreadWorker, this, 0, nullptr);
	}


	CBaiduMapProcessMgr::~CBaiduMapProcessMgr()
	{

	
	}


	DWORD WINAPI CBaiduMapProcessMgr::ThreadWorker(LPVOID /*lp*/)
	{

		return 0;
	}










};
