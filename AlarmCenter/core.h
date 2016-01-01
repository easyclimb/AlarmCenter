#pragma once

#include <list>

namespace core {


	class CZoneInfo;
	typedef CZoneInfo* PZone;
	typedef std::list<CZoneInfo*> CZoneInfoList;

	class CCameraInfo;
	typedef std::list<CCameraInfo*> CCameraInfoList;

	class CMapInfo;
	typedef std::shared_ptr<CMapInfo> CMapInfoPtr;
	typedef std::list<CMapInfoPtr> CMapInfoList;

	class CAlarmMachine;


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


	typedef enum MachineStatus {
		MACHINE_ARM = 0,
		MACHINE_HALFARM,
		MACHINE_DISARM,
		MACHINE_STATUS_UNKNOWN,
	}MachineStatus;


	inline MachineStatus Integer2MachineStatus(int status) {
		switch (status) {
		case core::MACHINE_ARM:return MACHINE_ARM;
			break;
		case core::MACHINE_HALFARM:return MACHINE_HALFARM;
			break;
		case core::MACHINE_DISARM:return MACHINE_DISARM;
			break;
		default:return MACHINE_STATUS_UNKNOWN;
			break;
		}
	}

	inline ADEMCO_EVENT MachineStatus2AdemcoEvent(MachineStatus status) {
		switch (status) {
		case core::MACHINE_ARM:return ademco::EVENT_ARM;
			break;
		case core::MACHINE_HALFARM:return ademco::EVENT_HALFARM;
			break;
		case core::MACHINE_DISARM:
		default:return ademco::EVENT_DISARM;
			break;
		}
	}


	NAMESPACE_END
