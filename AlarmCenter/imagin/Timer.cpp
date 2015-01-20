#include "stdafx.h"
#include "Timer.h"
#include <assert.h>
#include <Windows.h>
#include <mmsystem.h>
#pragma comment(lib, "winmm.lib")

namespace Imagin {

void __stdcall TimeOutCallback(UINT /*timeID*/,
							   UINT /*msg*/,
							   DWORD_PTR user,
							   DWORD_PTR ,
							   DWORD_PTR)
{
	CTimer *timer = reinterpret_cast<CTimer*>(user); assert(timer);
	timer->OnTimer();
}

CTimer::CTimer(TimeoutCb timeoutCb, void* udata)
	: _timerID(0)
	, _timeoutCb(timeoutCb)
	, _udata(udata)
{

}


CTimer::~CTimer()
{
	Stop();
	_timeoutCb = NULL;
}


void CTimer::Start(unsigned int delay, bool oneShot)
{
	_timerID = timeSetEvent(delay, 
							100, 
							TimeOutCallback, 
							(DWORD_PTR)this, 
							oneShot ? TIME_ONESHOT : TIME_PERIODIC);
}


void CTimer::Stop()
{
	if (_timerID != 0) {
		timeKillEvent(_timerID);
		_timerID = 0;
	}
}


void CTimer::OnTimer()
{
	if (_timerID == 0) {
		return;
	}

	if (_timeoutCb != NULL) {
		_timeoutCb(this, _udata);
	}
}


NAMESPACE_END
