#include "stdafx.h"
#include "BaiduMapProcessMgr.h"


//#ifdef _DEBUG
////#define BOOST_INTERPROCESS_WINDOWS_COINIT_MODEL COINIT_SPEED_OVER_MEMORY
//#pragma comment(lib, "C:/boost_1_59_0/stage/lib/libboost_date_time-vc140-mt-sgd-1_59.lib")
//#else
//#pragma comment(lib, "C:/boost_1_59_0/stage/lib/libboost_date_time-vc140-mt-s-1_59.lib")
//#endif
#define BOOST_DATE_TIME_NO_LIB

#include <boost/interprocess/ipc/message_queue.hpp>
#include <cstdio>

namespace core
{
	static const char g_subprocess_name[] = "BaiduMap.exe";
	static const char g_shared_memory_name[] = "MySharedMemory4AlarmCenter_1365_ce476ccfa4111007fb0c73a08b68ef9e";
	static const char g_instance_name[] = "my_struct instance";

	typedef struct mq_remover 
	{
		~mq_remover() { boost::interprocess::message_queue::remove(g_shared_memory_name); }
	}mq_remover;

	typedef enum status 
	{
		chaos,
		created,

	}status;


	IMPLEMENT_SINGLETON(CBaiduMapProcessMgr);
	CBaiduMapProcessMgr::CBaiduMapProcessMgr()
		: m_hThread(INVALID_HANDLE_VALUE)
		, m_hEvent(INVALID_HANDLE_VALUE)
		, m_hChildProcess(INVALID_HANDLE_VALUE)
		, m_hChildThread(INVALID_HANDLE_VALUE)
		
	{
		m_hEvent = CreateEvent(nullptr, FALSE, FALSE, nullptr);
		m_hThread = CreateThread(nullptr, 0, ThreadWorker, this, 0, nullptr);
		mq_remover remover;
	}


	CBaiduMapProcessMgr::~CBaiduMapProcessMgr()
	{
		SetEvent(m_hEvent);
		WaitForSingleObject(m_hThread, INFINITE);
		CLOSEHANDLE(m_hEvent);
		CLOSEHANDLE(m_hThread);
		force_close_subprocess();
		mq_remover remover;
	}


	DWORD WINAPI CBaiduMapProcessMgr::ThreadWorker(LPVOID lp)
	{
		CBaiduMapProcessMgr* mgr = reinterpret_cast<CBaiduMapProcessMgr*>(lp);
		for (;;) {
			if (WAIT_OBJECT_0 == WaitForSingleObject(mgr->m_hEvent, 100)) { break; }

		}
		return 0;
	}


	void CBaiduMapProcessMgr::start_subprocess()
	{
		force_close_subprocess();

	}


	void CBaiduMapProcessMgr::force_close_subprocess()
	{
		std::string cmd = "taskkill /F /IM ";
		cmd += g_subprocess_name;
		std::system(cmd.c_str());
	}




};
