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

// 主机类型
typedef enum machine_type {
	MT_UNKNOWN = 0,
	MT_WIFI,							// wifi主机
	MT_NETMOD,							// 带网络模块的工程主机
	MT_IMPRESSED_GPRS_MACHINE_2050,		// 改进型卧式主机2050型
	MT_LCD,								// 液晶主机
	MT_WIRE,							// 网线主机
	MT_MAX,
}machine_type;

inline machine_type Integer2MachineType(int type)
{
	switch (type) {
		case MT_WIFI:	return MT_WIFI;		break;
		case MT_NETMOD:	return MT_NETMOD;	break;
		case MT_IMPRESSED_GPRS_MACHINE_2050: 	return MT_IMPRESSED_GPRS_MACHINE_2050;		break;
		case MT_LCD:	return MT_LCD;		break;
		case MT_WIRE:	return MT_WIRE;		break;
		default:		return MT_UNKNOWN;	break;
	}
}


// 主机信号强度
typedef enum signal_strength {
	SIGNAL_STRENGTH_0,
	SIGNAL_STRENGTH_1,
	SIGNAL_STRENGTH_2,
	SIGNAL_STRENGTH_3,
	SIGNAL_STRENGTH_4,
	SIGNAL_STRENGTH_5,
}signal_strength;

inline signal_strength Integer2SignalStrength(int strength) {
	if (0 <= strength && strength <= 5) {
		return SIGNAL_STRENGTH_0;
	} else if (6 <= strength && strength <= 10) {
		return SIGNAL_STRENGTH_1;
	} else if (11 <= strength && strength <= 15) {
		return SIGNAL_STRENGTH_2;
	} else if (16 <= strength && strength <= 20) {
		return SIGNAL_STRENGTH_3;
	} else if (21 <= strength && strength <= 25) {
		return SIGNAL_STRENGTH_4;
	} else {
		return SIGNAL_STRENGTH_5;
	}
}


// 获取主机最大防区号
inline int get_machine_max_zone_by_type(machine_type mt) {
	switch (mt) {
		case core::MT_WIFI:
			return 30;
			break;

		case core::MT_WIRE:
		case core::MT_IMPRESSED_GPRS_MACHINE_2050:
			return 99;
			break;

		case core::MT_LCD:
			return 256;
			break;

		case core::MT_NETMOD:
		default:
			return 999;
			break;
	}
}

// 获取主机防区数量的位数
inline int get_figure_of_machine_max_zone_by_type(machine_type mt) {
	int zone_count = get_machine_max_zone_by_type(mt);
	int figure = 0;
	while (zone_count) {
		zone_count /= 10;
		figure++;
	}
	return figure;
}

// 获取主机防区数量的位数的格式化字符串
inline const wchar_t* get_format_string_of_machine_zone_count_figure_by_type(machine_type mt) {
	auto figure = get_figure_of_machine_max_zone_by_type(mt);
	switch (figure) {
		case 2:
			return L"%02d";
			break;
		default:
			return L"%03d";
			break;
	}
}

// 是否只能以索要的方式添加防区
inline bool is_machine_can_only_add_zone_by_retrieve(machine_type mt) {
	switch (mt) {
		case core::MT_WIFI:
		case core::MT_IMPRESSED_GPRS_MACHINE_2050:
		case core::MT_WIRE:
			return true;
			break;

		case core::MT_LCD:
			return false;
			break;

		case core::MT_NETMOD:
		default:
			return false;
			break;
	}
}

// 是否具有短信报警功能
inline bool is_machine_can_report_alarm_by_sms(machine_type mt) {
	return mt == core::MT_IMPRESSED_GPRS_MACHINE_2050;
}

// 是否具有信号强度报告功能
inline bool is_machine_can_report_signal_strength(machine_type mt) {
	return mt == core::MT_IMPRESSED_GPRS_MACHINE_2050;
}

// 主机是否可以挂载分机
inline bool is_machine_can_link_submachine(machine_type mt) {
	return mt == core::MT_NETMOD;
}


static const int MIN_MACHINE = 1;
static const int MAX_MACHINE = 999999;

static const int ZONE_VALUE_FOR_MACHINE_SELF = 666666;

static const int MIN_MACHINE_ZONE = 1;
static const int MAX_MACHINE_ZONE = 1000;

static const int MIN_SUBMACHINE_ZONE = 1;
static const int MAX_SUBMACHINE_ZONE = 100;

// 2016年8月18日16:32:33 警情处理之警情类型
enum alarm_type {
	alarm_type_min,
	alarm_type_true,						// 真实警情
	alarm_type_device_false_positive,		// 设备误报
	alarm_type_test_device,					// 测试设备
	alarm_type_man_made_false_positive,		// 人为误报
	alarm_type_cannot_determine,			// 无法判断
	alarm_type_max,
};

// 2016年8月22日15:14:40 警情处理之判断依据
enum alarm_judgement {
	alarm_judgement_min,
	alarm_judgement_by_video_image,			// 视频/图片复合
	alarm_judgement_by_confirm_with_owner,	// 已与机主确认
	alarm_judgement_by_platform_tip,		// 平台上传提示
	alarm_judgement_by_user_define,			// 用户自定义
	alarm_judgement_max,
};

// 2016年8月22日16:12:25 警情处理之警情状态
enum alarm_status {
	alarm_status_min,
	alarm_status_not_judged,				// 未判断
	alarm_status_not_handled,				// 已判断，未处理
	alarm_status_not_cleared,				// 已处理，未消警
	alarm_status_cleared,					// 消警成功
	alarm_status_max,
};

// ademco_id, zone_value
typedef std::pair<int, int> machine_uuid;

typedef std::shared_ptr<CWnd> CWndPtr;

typedef struct progress_ex {
	bool main;
	int progress;
	int value;
	int total;
	progress_ex() : progress(0), value(0), total(0) {}
}progress_ex;
typedef std::shared_ptr<progress_ex> progress_ex_ptr;
typedef void(__stdcall *LoadDBProgressCB)(void* udata, const progress_ex_ptr& progress);

// sub_zone: 
// 0 for zone of machine
// between 0 and 0xEE for zone of sub machine
// 0xee for sub machine
static const int INDEX_ZONE = 0;
static const int INDEX_SUB_MACHINE = 0xEE;

class zone_info;
typedef std::shared_ptr<zone_info> zone_info_ptr;
typedef std::list<zone_info_ptr> zone_info_list;

class camera_info;
typedef std::shared_ptr<camera_info> camera_info_ptr;
typedef std::list<camera_info_ptr> camera_info_list;

class map_info;
typedef std::shared_ptr<map_info> map_info_ptr;
typedef std::weak_ptr<map_info> map_info_weak_ptr;
typedef std::list<map_info_ptr> map_info_list;
typedef std::map<int, map_info_ptr> map_info_map;

class detector_bind_interface;
typedef std::shared_ptr<detector_bind_interface> detector_bind_interface_ptr;

class detector_lib_data;
typedef std::shared_ptr<detector_lib_data> detector_lib_data_ptr;
typedef std::list<detector_lib_data_ptr> detector_lib_data_list;

class detector_info;
typedef std::shared_ptr<detector_info> detector_info_ptr;
typedef std::list<detector_info_ptr> detector_info_list;

class alarm_machine;
typedef std::shared_ptr<alarm_machine> alarm_machine_ptr;
typedef std::list<alarm_machine_ptr> alarm_machine_list;
typedef std::map<int, alarm_machine_ptr> alarm_machine_map;

class group_info;
typedef std::shared_ptr<group_info> group_info_ptr;
typedef std::list<group_info_ptr> group_info_list;
typedef std::weak_ptr<group_info> group_info_weak_ptr;
typedef std::list<group_info_weak_ptr> group_info_weak_list;


typedef struct alarm_text {
	int _zone;
	int _subzone;
	ademco::ADEMCO_EVENT _event;
	time_t _time;
	CString _txt;
	alarm_text() : _zone(0), _subzone(0), _event(ademco::EVENT_INVALID_EVENT), _time(), _txt(_T("")) { }
	alarm_text(const alarm_text& rhs) : _zone(rhs._zone), _subzone(rhs._subzone), _event(rhs._event), _time(rhs._time), _txt(rhs._txt) { }

	alarm_text& operator=(const alarm_text& rhs) {
		_zone = rhs._zone;
		_subzone = rhs._subzone;
		_event = rhs._event;
		_time = rhs._time;
		_txt = rhs._txt;
		return *this;
	}

	CString get_text_with_formatted_date() const {
		CString ret;
		ret = time_point_to_wstring(std::chrono::system_clock::from_time_t(_time)).c_str();
		ret += _txt.Right(_txt.GetLength() - 9);
		return ret;
	}
}alarm_text;

typedef std::shared_ptr<alarm_text> alarm_text_ptr;

// 反向控制地图实体命令
enum inversion_control_map_command {
	ICMC_SHOW,				// 显示地图
	ICMC_ADD_ALARM_TEXT,	// 添加报警文字并显示(需附加参数alarm_text)
	ICMC_DEL_ALARM_TEXT,	// 删除报警文字
	ICMC_CLR_ALARM_TEXT,	// 清除报警文字
	ICMC_MODE_EDIT,			// 进入编辑模式
	ICMC_MODE_NORMAL,		// 退出编辑模式
	ICMC_RENAME,			// 重命名
	ICMC_CHANGE_IMAGE,		// 更换图片
	ICMC_NEW_DETECTOR,		// 新增探头
	ICMC_DEL_DETECTOR,		// 删除探头
	//ICMC_DESTROY,			// 释放对自己的引用
};

typedef struct icmc_buffer {
	inversion_control_map_command _icmc;
	alarm_text_ptr _at;
	icmc_buffer(inversion_control_map_command icmc, const alarm_text_ptr& at) :_icmc(icmc), _at(at) {}
	~icmc_buffer() {}
}icmc_buffer;
typedef std::shared_ptr<icmc_buffer> icmc_buffer_ptr;


typedef std::shared_ptr<CMapView> CMapViewPtr;
typedef std::weak_ptr<CMapView> CMapViewWeakPtr;
typedef void(*OnInversionControlMapCB)(const CMapViewPtr& view,
									   const icmc_buffer_ptr& icmc);


typedef enum inversion_control_zone_command {
	ICZC_ALARM_START,	// 报警
	ICZC_ALARM_STOP,	// 消警
	ICZC_SET_FOCUS,		// 高亮
	ICZC_KILL_FOCUS,	// 取消高亮
	ICZC_ROTATE,		// 旋转
	ICZC_DISTANCE,		// 调整间距(仅针对对射探头)
	ICZC_MOVE,			// 移动
	ICZC_CLICK,			// 单击
	ICZC_RCLICK,		// 右击
						//ICZC_ALIAS_CHANGED, // 别名已修改
	//ICZC_DESTROY,		// zone_info已析构
}inversion_control_zone_command;

typedef struct iczc_buffer {
	inversion_control_zone_command _iczc;
	DWORD _extra;
	iczc_buffer(inversion_control_zone_command iczc, DWORD extra) :_iczc(iczc), _extra(extra) {}
}iczc_buffer;
typedef std::shared_ptr<iczc_buffer> iczc_buffer_ptr;


typedef std::shared_ptr<CDetector> CDetectorPtr;
typedef std::weak_ptr<CDetector> CDetectorWeakPtr;
typedef void(__stdcall *OnInversionControlZoneCB)(const CDetectorPtr& detector,
												  const iczc_buffer_ptr& iczc);


typedef enum remote_control_command_conn
{
	RCCC_HANGUP,
	RCCC_RESUME,
	RCCC_DISCONN,
}remote_control_command_conn;

typedef void(__stdcall *remote_control_command_conn_call_back)(net::server::CClientDataPtr udata, remote_control_command_conn rccc);
typedef struct remote_control_command_conn_obj
{
	net::server::CClientDataPtr udata;
	remote_control_command_conn_call_back cb;
	remote_control_command_conn_obj() : udata(nullptr), cb(nullptr) {}
	void reset() { udata = nullptr; cb = nullptr; }
	bool valid() const { return ((udata != nullptr) && (cb != nullptr)); }
}remote_control_command_conn_obj;

typedef struct sms_config
{
	int id;
	bool report_alarm;
	bool report_exception;
	bool report_status;
	bool report_alarm_bk;
	bool report_exception_bk;
	bool report_status_bk;

	sms_config() : id(-1), report_alarm(false), report_exception(false), report_status(false),
		report_alarm_bk(false), report_exception_bk(false), report_status_bk(false)
	{}

	sms_config(const sms_config& rhs)
	{
		id = rhs.id;
		report_alarm = rhs.report_alarm;
		report_exception = rhs.report_exception;
		report_status = rhs.report_status;
		report_alarm_bk = rhs.report_alarm_bk;
		report_exception_bk = rhs.report_exception_bk;
		report_status_bk = rhs.report_status_bk;
	}

	sms_config& operator = (const sms_config& rhs)
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
}sms_config;


typedef enum machine_status {
	MACHINE_ARM = 0,
	MACHINE_HALFARM,
	MACHINE_DISARM,
	MACHINE_STATUS_UNKNOWN,
}machine_status;


inline machine_status Integer2MachineStatus(int status) {
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

inline ademco::ADEMCO_EVENT MachineStatus2AdemcoEvent(machine_status status) {
	switch (status) {
	case core::MACHINE_ARM:
		return ademco::EVENT_ARM;
		break;
	case core::MACHINE_HALFARM:
		return ademco::EVENT_HALFARM;
		break;
	case core::MACHINE_DISARM:
	default:return ademco::EVENT_DISARM;
		break;
	}
}


class history_record;
typedef std::shared_ptr<history_record> history_record_ptr;

/*template <typename dlg_type>
class history_record_observer : public dp::observer<history_record_ptr>
{
public:
	explicit history_record_observer(dlg_type* dlg) : _dlg(dlg) {}

	virtual void on_update(const history_record_ptr& ptr) {
		if (_dlg) {
			_dlg->OnNewRecordResult(ptr);
		}
	}
private:
	dlg_type* _dlg;
};*/


typedef enum user_priority {
	UP_SUPER = 0,
	UP_ADMIN,
	UP_OPERATOR,
}user_priority;

class user_info;
typedef std::shared_ptr<user_info> user_info_ptr;


struct consumer_type {
	int id;
	CString name;

	consumer_type(int id, const CString& name) : id(id), name(name) {}
};

typedef std::shared_ptr<consumer_type> consumer_type_ptr;
typedef std::map<int, consumer_type_ptr> consumer_type_map;

struct consumer;
typedef std::shared_ptr<consumer> consumer_ptr;
typedef std::list<consumer_ptr> consumer_list;

};
