#pragma once

namespace core {
typedef enum RemoteControlCommandConn
{
	RCCC_HANGUP,
	RCCC_RESUME,
	RCCC_DISCONN,
}RemoteControlCommandConn;

typedef void(__stdcall *RemoteControlCommandConnCB)(void* udata, RemoteControlCommandConn rccc);
typedef struct RemoteControlCommandConnObj
{
	void* udata;
	RemoteControlCommandConnCB cb;
	RemoteControlCommandConnObj() : udata(nullptr), cb(nullptr) {}
	void reset() { udata = nullptr; cb = nullptr; }
	bool valid() const { return ((udata != nullptr) && (cb != nullptr)); }
}RemoteControlCommandConnObj;

typedef struct SmsConfigure
{
	int id;
	bool report_alarm;
	bool report_exception;
	bool report_status;
	bool report_alarm_bk;
	bool report_exception_bk;
	bool report_status_bk;

	SmsConfigure() : id(-1), report_alarm(false), report_exception(false), report_status(false),
		report_alarm_bk(false), report_exception_bk(false), report_status_bk(false)
	{}

	SmsConfigure(const SmsConfigure& rhs) 
	{
		id = rhs.id;
		report_alarm = rhs.report_alarm;
		report_exception = rhs.report_exception;
		report_status = rhs.report_status;
		report_alarm_bk = rhs.report_alarm_bk;
		report_exception_bk = rhs.report_exception_bk;
		report_status_bk = rhs.report_status_bk;
	}

	SmsConfigure& operator = (const SmsConfigure& rhs)
	{
		id = rhs.id;
		report_alarm = rhs.report_alarm;
		report_exception = rhs.report_exception;
		report_status = rhs.report_status;
		report_alarm_bk = rhs.report_alarm_bk;
		report_exception_bk = rhs.report_exception_bk;
		report_status_bk = rhs.report_status_bk;
		return *this;
	}
}SmsConfigure;

NAMESPACE_END
