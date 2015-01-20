#pragma once

namespace Imagin {
class CTimer;
typedef void(__stdcall *TimeoutCb)(CTimer* timer, void* udata);

class CTimer
{
public:
	CTimer(TimeoutCb timeoutCb, void* udata);
	virtual ~CTimer();
	void Start(unsigned int delay, bool oneShot = true);
	void Stop();
	void OnTimer();
private:
	CTimer() {}
	unsigned int	_timerID;
	TimeoutCb		_timeoutCb;
	void*			_udata;
};
NAMESPACE_END
