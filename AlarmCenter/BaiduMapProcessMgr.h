#pragma once

namespace core
{
	class CBaiduMapProcessMgr
	{
	private:
		HANDLE m_hThread;
		HANDLE m_hEvent;
		HANDLE m_hChildProcess;
		HANDLE m_hChildThread;
	protected:
		static DWORD WINAPI ThreadWorker(LPVOID lp);
	public:

		//CBaiduMapProcessMgr();
		~CBaiduMapProcessMgr();
		DECLARE_UNCOPYABLE(CBaiduMapProcessMgr);
		DECLARE_SINGLETON(CBaiduMapProcessMgr);
	};
};
