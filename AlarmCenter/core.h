#pragma once

#include <list>
#include <map>
#include <memory>


class CMapView;
class CDetector;

namespace net {
	namespace server {
		class CClientData;
		typedef std::shared_ptr<CClientData> CClientDataPtr;
	};
};

namespace core {
	typedef std::shared_ptr<CWnd> CWndPtr;

	typedef struct ProgressEx {
		bool main;
		int progress;
		int value;
		int total;
		ProgressEx() : progress(0), value(0), total(0) {}
	}ProgressEx;
	typedef std::shared_ptr<ProgressEx> ProgressExPtr;
	typedef void(__stdcall *LoadDBProgressCB)(void* udata, const ProgressExPtr& progress);

	

	class CZoneInfo;
	typedef std::shared_ptr<CZoneInfo> CZoneInfoPtr;
	typedef std::list<CZoneInfoPtr> CZoneInfoList;

	class CCameraInfo;
	typedef std::shared_ptr<CCameraInfo> CCameraInfoPtr;
	typedef std::list<CCameraInfoPtr> CCameraInfoList;

	class CMapInfo;
	typedef std::shared_ptr<CMapInfo> CMapInfoPtr;
	typedef std::weak_ptr<CMapInfo> CMapInfoWeakPtr;
	typedef std::list<CMapInfoPtr> CMapInfoList;

	class CDetectorBindInterface;
	typedef std::shared_ptr<CDetectorBindInterface> CDetectorBindInterfacePtr;

	class CDetectorLibData;
	typedef std::shared_ptr<CDetectorLibData> CDetectorLibDataPtr;
	typedef std::list<CDetectorLibDataPtr> CDetectorLibDataList;

	class CDetectorInfo;
	typedef std::shared_ptr<CDetectorInfo> CDetectorInfoPtr;
	typedef std::list<CDetectorInfoPtr> CDetectorInfoList;

	class CAlarmMachine;
	typedef std::shared_ptr<CAlarmMachine> CAlarmMachinePtr;
	typedef std::list<CAlarmMachinePtr> CAlarmMachineList;
	typedef std::map<int, CAlarmMachinePtr> CAlarmMachineMap;

	class CGroupInfo;
	typedef std::shared_ptr<CGroupInfo> CGroupInfoPtr;
	typedef std::list<CGroupInfoPtr> CGroupInfoList;
	typedef std::weak_ptr<CGroupInfo> CGroupInfoWeakPtr;
	typedef std::list<CGroupInfoWeakPtr> CGroupInfoWeakList;


	typedef struct AlarmText {
		int _zone;
		int _subzone;
		int _event;
		CString _txt;
		AlarmText() : _zone(0), _subzone(0), _event(0), _txt(_T("")) { AUTO_LOG_FUNCTION; JLOG(L"%p", this); }
		AlarmText(const AlarmText& rhs) : _zone(rhs._zone), _subzone(rhs._subzone), _event(rhs._event), _txt(rhs._txt) { AUTO_LOG_FUNCTION; JLOG(L"%p", this); }

		AlarmText& operator=(const AlarmText& rhs) {
			_zone = rhs._zone;
			_subzone = rhs._subzone;
			_event = rhs._event;
			_txt = rhs._txt;
			return *this;
		}
	}AlarmText;

	typedef std::shared_ptr<AlarmText> AlarmTextPtr;

	// ������Ƶ�ͼʵ������
	enum InversionControlMapCommand {
		ICMC_SHOW,				// ��ʾ��ͼ
		ICMC_ADD_ALARM_TEXT,	// ��ӱ������ֲ���ʾ(�踽�Ӳ���AlarmText)
		ICMC_DEL_ALARM_TEXT,	// ɾ����������
		ICMC_CLR_ALARM_TEXT,	// �����������
		ICMC_MODE_EDIT,			// ����༭ģʽ
		ICMC_MODE_NORMAL,		// �˳��༭ģʽ
		ICMC_RENAME,			// ������
		ICMC_CHANGE_IMAGE,		// ����ͼƬ
		ICMC_NEW_DETECTOR,		// ����̽ͷ
		ICMC_DEL_DETECTOR,		// ɾ��̽ͷ
		//ICMC_DESTROY,			// �ͷŶ��Լ�������
	};

	typedef struct IcmcBuffer {
		InversionControlMapCommand _icmc;
		AlarmTextPtr _at;
		IcmcBuffer(InversionControlMapCommand icmc, const AlarmTextPtr& at) :_icmc(icmc), _at(at) {}
		~IcmcBuffer() {}
	}IcmcBuffer;
	typedef std::shared_ptr<IcmcBuffer> IcmcBufferPtr;


	typedef std::shared_ptr<CMapView> CMapViewPtr;
	typedef std::weak_ptr<CMapView> CMapViewWeakPtr;
	typedef void(__stdcall *OnInversionControlMapCB)(const CMapViewPtr& view,
													 const IcmcBufferPtr& icmc);


	typedef enum InversionControlZoneCommand {
		ICZC_ALARM_START,	// ����
		ICZC_ALARM_STOP,	// ����
		ICZC_SET_FOCUS,		// ����
		ICZC_KILL_FOCUS,	// ȡ������
		ICZC_ROTATE,		// ��ת
		ICZC_DISTANCE,		// �������(����Զ���̽ͷ)
		ICZC_MOVE,			// �ƶ�
		ICZC_CLICK,			// ����
		ICZC_RCLICK,		// �һ�
							//ICZC_ALIAS_CHANGED, // �������޸�
		//ICZC_DESTROY,		// CZoneInfo������
	}InversionControlZoneCommand;

	typedef struct IczcBuffer {
		InversionControlZoneCommand _iczc;
		DWORD _extra;
		IczcBuffer(InversionControlZoneCommand iczc, DWORD extra) :_iczc(iczc), _extra(extra) {}
	}IczcBuffer;
	typedef std::shared_ptr<IczcBuffer> IczcBufferPtr;


	typedef std::shared_ptr<CDetector> CDetectorPtr;
	typedef std::weak_ptr<CDetector> CDetectorWeakPtr;
	typedef void(__stdcall *OnInversionControlZoneCB)(const CDetectorPtr& detector,
													  const IczcBufferPtr& iczc);


	typedef enum RemoteControlCommandConn
	{
		RCCC_HANGUP,
		RCCC_RESUME,
		RCCC_DISCONN,
	}RemoteControlCommandConn;

	typedef void(__stdcall *RemoteControlCommandConnCB)(net::server::CClientDataPtr udata, RemoteControlCommandConn rccc);
	typedef struct RemoteControlCommandConnObj
	{
		net::server::CClientDataPtr udata;
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


	class HistoryRecord;
	typedef std::shared_ptr<HistoryRecord> HistoryRecordPtr;

	template <typename dlg_type>
	class HistoryRecordObserver : public dp::observer<HistoryRecordPtr>
	{
	public:
		explicit HistoryRecordObserver(dlg_type* dlg) : _dlg(dlg) {}

		virtual void on_update(const HistoryRecordPtr& ptr) {
			if (_dlg) {
				_dlg->OnNewRecordResult(ptr);
			}
		}
	private:
		dlg_type* _dlg;
	};


	class CUserInfo;
	typedef std::shared_ptr<CUserInfo> CUserInfoPtr;

};
