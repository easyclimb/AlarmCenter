#pragma once 


#include <vector>
#include <memory>

#include "observer.h"

#ifdef USES_ADEMCO_EVENT_TO_STRING
#include <string>
#include <sstream>
#endif



typedef int ADEMCO_EVENT;

namespace ademco
{
	typedef std::vector<char> char_array;
	typedef std::shared_ptr<char_array> char_array_ptr;


#pragma region event_definetion

	// -------------------接警中心内部使用事件------------------------------
	static const ADEMCO_EVENT EVENT_INVALID_EVENT		= 0;
	static const ADEMCO_EVENT EVENT_PRIVATE_EVENT_BASE	= 0x00010000;
	static const ADEMCO_EVENT EVENT_CLEARMSG			= EVENT_PRIVATE_EVENT_BASE;
	static const ADEMCO_EVENT EVENT_OFFLINE				= EVENT_PRIVATE_EVENT_BASE + 1;
	static const ADEMCO_EVENT EVENT_ONLINE				= EVENT_PRIVATE_EVENT_BASE + 2;
	static const ADEMCO_EVENT EVENT_SUBMACHINECNT		= EVENT_PRIVATE_EVENT_BASE + 3;
	static const ADEMCO_EVENT EVENT_MACHINE_ALIAS		= EVENT_PRIVATE_EVENT_BASE + 4;
	static const ADEMCO_EVENT EVENT_IM_GONNA_DIE		= EVENT_PRIVATE_EVENT_BASE + 5;
	static const ADEMCO_EVENT EVENT_PRIVATE_EVENT_MAX	= EVENT_IM_GONNA_DIE;
	// ------------------------------------------------------------------

	// -------------------标准安定宝协议事件--------------------------------
	// 主机或分机状态报告
	static const ADEMCO_EVENT EVENT_ARM				= 3400; // 布防
	static const ADEMCO_EVENT EVENT_DISARM			= 1400; // 撤防
	static const ADEMCO_EVENT EVENT_HALFARM			= 3456; // 半布防
	static const ADEMCO_EVENT EVENT_EMERGENCY		= 1120; // 紧急报警

	// 防区报警
	static const ADEMCO_EVENT EVENT_BURGLAR			= 1130; // 盗警
	static const ADEMCO_EVENT EVENT_DOORRINGING		= 1134; // 门铃
	static const ADEMCO_EVENT EVENT_FIRE			= 1110; // 火警
	static const ADEMCO_EVENT EVENT_DURESS			= 1121; // 胁迫
	static const ADEMCO_EVENT EVENT_GAS				= 1151; // 煤气
	static const ADEMCO_EVENT EVENT_WATER			= 1113; // 水警
	static const ADEMCO_EVENT EVENT_TEMPER			= 1137; // 防拆

	// 防区异常
	static const ADEMCO_EVENT EVENT_LOWBATTERY		= 1302; // 低电
	static const ADEMCO_EVENT EVENT_BATTERY_RECOVER	= 3302; // 复电
	static const ADEMCO_EVENT EVENT_BADBATTERY		= 1311; // 坏电
	static const ADEMCO_EVENT EVENT_SOLARDISTURB	= 1387; // 光扰
	static const ADEMCO_EVENT EVENT_DISCONNECT		= 1381; // 失效
	static const ADEMCO_EVENT EVENT_RECONNECT		= 3381; // 恢复
	// ------------------------------------------------------------------

	// ------------------私有事件-----------------------------------------
	static const ADEMCO_EVENT EVENT_SERIAL485DIS	= 1485; // 网络模块与主机485串口连接断开
	static const ADEMCO_EVENT EVENT_SERIAL485CONN	= 3485; // 网络模块与主机485串口连接恢复

	static const ADEMCO_EVENT EVENT_CONN_HANGUP		= 1700; // 链路挂起
	static const ADEMCO_EVENT EVENT_CONN_RESUME		= 3700; // 链路恢复
	
	static const ADEMCO_EVENT EVENT_DISARM_PWD_ERR					= 1701; // 撤防密码错误
	static const ADEMCO_EVENT EVENT_SUB_MACHINE_SENSOR_EXCEPTION	= 1702; // 分机探头异常
	static const ADEMCO_EVENT EVENT_SUB_MACHINE_SENSOR_RESUME		= 3702; // 分机探头恢复
	static const ADEMCO_EVENT EVENT_SUB_MACHINE_POWER_EXCEPTION		= 1703; // 分机电源异常
	static const ADEMCO_EVENT EVENT_SUB_MACHINE_POWER_RESUME		= 3703; // 分机电源恢复
	static const ADEMCO_EVENT EVENT_RETRIEVE_ZONE_OR_SUB_MACHINE	= 1704; // 索要防区或分机信息
	static const ADEMCO_EVENT EVENT_QUERY_SUB_MACHINE				= 1705; // 查询分机信息
	static const ADEMCO_EVENT EVENT_WRITE_TO_MACHINE				= 1706; // 写入主机信息
	static const ADEMCO_EVENT EVENT_I_AM_NET_MODULE					= 1707; // 我是网络模块
	static const ADEMCO_EVENT EVENT_PHONE_USER_SOS					= 1709; // 手机用户SOS
	static const ADEMCO_EVENT EVENT_PHONE_USER_CANCLE_ALARM			= 1711; // 手机用户消警
	static const ADEMCO_EVENT EVENT_I_AM_EXPRESSED_GPRS_2050_MACHINE = 1717; // 我是改进型卧室主机2050型
	// ------------------------------------------------------------------


#ifdef USES_ADEMCO_EVENT_TO_STRING
	static const ADEMCO_EVENT gc_AdemcoEvent[] = {
		EVENT_ARM,
		EVENT_DISARM,
		EVENT_HALFARM,
		EVENT_EMERGENCY,
		EVENT_BURGLAR,
		EVENT_DOORRINGING,
		EVENT_FIRE,
		EVENT_DURESS,
		EVENT_GAS,
		EVENT_WATER,
		EVENT_TEMPER,
		EVENT_LOWBATTERY,
		EVENT_BATTERY_RECOVER,
		EVENT_BADBATTERY,
		EVENT_SOLARDISTURB,
		EVENT_DISCONNECT,
		EVENT_RECONNECT,
		EVENT_SERIAL485DIS,
		EVENT_SERIAL485CONN,
		EVENT_CONN_HANGUP, 
		EVENT_CONN_RESUME,
		EVENT_SUB_MACHINE_SENSOR_EXCEPTION,
		EVENT_SUB_MACHINE_SENSOR_RESUME,
		EVENT_SUB_MACHINE_POWER_EXCEPTION,
		EVENT_SUB_MACHINE_POWER_RESUME,
		EVENT_RETRIEVE_ZONE_OR_SUB_MACHINE,
		EVENT_QUERY_SUB_MACHINE,
		EVENT_WRITE_TO_MACHINE,
		EVENT_I_AM_NET_MODULE,
		EVENT_PHONE_USER_SOS,
		EVENT_I_AM_EXPRESSED_GPRS_2050_MACHINE,
	};

	inline std::string GetAdemcoEventStringEnglish(ADEMCO_EVENT ademco_event)
	{
		auto n_to_s = [](int n) ->std::string {std::stringstream ss; ss << n << "--"; return ss.str(); };
		switch (ademco_event) {
			case EVENT_ARM:			return n_to_s(ademco_event) + "ARM";		break;
			case EVENT_BURGLAR:		return n_to_s(ademco_event) + "BURGLAR";	break;
			case EVENT_DISARM:		return n_to_s(ademco_event) + "DISARM";	break;
			case EVENT_DURESS:		return n_to_s(ademco_event) + "DURESS";	break;
			case EVENT_EMERGENCY:	return n_to_s(ademco_event) + "EMERGENCY";	break;
			case EVENT_FIRE:		return n_to_s(ademco_event) + "FIRE";		break;
			case EVENT_GAS:			return n_to_s(ademco_event) + "GAS";		break;
			case EVENT_HALFARM:		return n_to_s(ademco_event) + "HALFARM";	break;
			case EVENT_TEMPER:		return n_to_s(ademco_event) + "TEMPER";	break;
			case EVENT_WATER:		return n_to_s(ademco_event) + "WATER";		break;
			case EVENT_LOWBATTERY:	return n_to_s(ademco_event) + "LOWBATTERY";	break;
			case EVENT_BATTERY_RECOVER: return n_to_s(ademco_event) + "BATATTERY_RECOVER"; break;
			case EVENT_BADBATTERY:	return n_to_s(ademco_event) + "BADBATTERY";	break;
			case EVENT_SOLARDISTURB:return n_to_s(ademco_event) + "SOLARDISTURB";	break;
			case EVENT_DISCONNECT:	return n_to_s(ademco_event) + "DISCONNECT";		break;
			case EVENT_RECONNECT:	return n_to_s(ademco_event) + "RECONNECT";		break;
			case EVENT_SERIAL485DIS:return n_to_s(ademco_event) + "485DIS";	break;
			case EVENT_SERIAL485CONN:return n_to_s(ademco_event) + "485CONN";	break;
			case EVENT_DOORRINGING:	return n_to_s(ademco_event) + "DOORRINGING";	break;
			case EVENT_CONN_HANGUP: return n_to_s(ademco_event) + "CONN_HANGUP"; break;
			case EVENT_CONN_RESUME: return n_to_s(ademco_event) + "CONN_RESUME"; break;
			case EVENT_SUB_MACHINE_SENSOR_EXCEPTION: return n_to_s(ademco_event) + "SUB_SENSOR_EXCEPTION"; break;
			case EVENT_SUB_MACHINE_SENSOR_RESUME: return n_to_s(ademco_event) + "SUB_SENSOR_RESUME"; break;
			case EVENT_SUB_MACHINE_POWER_EXCEPTION: return n_to_s(ademco_event) + "SUB_POWER_EXCEPTION"; break;
			case EVENT_SUB_MACHINE_POWER_RESUME: return n_to_s(ademco_event) + "SUB_POWER_RESUME"; break;
			case EVENT_RETRIEVE_ZONE_OR_SUB_MACHINE:return n_to_s(ademco_event) + "RETRIEVE"; break;
			case EVENT_QUERY_SUB_MACHINE:return n_to_s(ademco_event) + "QUERY"; break;
			case EVENT_WRITE_TO_MACHINE:return n_to_s(ademco_event) + "WRITE_TO_MACHINE"; break;
			case EVENT_I_AM_NET_MODULE:return n_to_s(ademco_event) + "I_AM_NET_MODULE"; break;
			case EVENT_PHONE_USER_SOS:return n_to_s(ademco_event) + "SOS"; break;
			case EVENT_I_AM_EXPRESSED_GPRS_2050_MACHINE: return n_to_s(ademco_event) + "I_AM_EXPRESSED_GPRS_2050_MACHINE"; break;
			default: return n_to_s(ademco_event) + "undefined"; break;
		}
	}

	inline const std::wstring GetAdemcoEventStringChinese(ADEMCO_EVENT ademco_event)
	{
		auto n_to_s = [](int n) ->std::wstring {std::wstringstream ss; ss << n << L"--"; return ss.str(); };
		switch (ademco_event) {
		case EVENT_ARM:			return n_to_s(ademco_event) + L"布防";		break;
		case EVENT_BURGLAR:		return n_to_s(ademco_event) + L"盗警";	break;
		case EVENT_DISARM:		return n_to_s(ademco_event) + L"撤防";	break;
		case EVENT_DURESS:		return n_to_s(ademco_event) + L"胁迫";	break;
		case EVENT_EMERGENCY:	return n_to_s(ademco_event) + L"紧急报警";	break;
		case EVENT_FIRE:		return n_to_s(ademco_event) + L"火警";		break;
		case EVENT_GAS:			return n_to_s(ademco_event) + L"煤气";		break;
		case EVENT_HALFARM:		return n_to_s(ademco_event) + L"半布防";	break;
		case EVENT_TEMPER:		return n_to_s(ademco_event) + L"防拆";	break;
		case EVENT_WATER:		return n_to_s(ademco_event) + L"水警";		break;
		case EVENT_LOWBATTERY:	return n_to_s(ademco_event) + L"低电";	break;
		case EVENT_BATTERY_RECOVER: return n_to_s(ademco_event) + L"复电"; break;
		case EVENT_BADBATTERY:	return n_to_s(ademco_event) + L"坏电";	break;
		case EVENT_SOLARDISTURB:return n_to_s(ademco_event) + L"光扰";	break;
		case EVENT_DISCONNECT:	return n_to_s(ademco_event) + L"失效";		break;
		case EVENT_RECONNECT:	return n_to_s(ademco_event) + L"恢复";		break;
		case EVENT_SERIAL485DIS:return n_to_s(ademco_event) + L"485断开";	break;
		case EVENT_SERIAL485CONN:return n_to_s(ademco_event) + L"485恢复";	break;
		case EVENT_DOORRINGING:	return n_to_s(ademco_event) + L"门铃";	break;
		case EVENT_CONN_HANGUP: return n_to_s(ademco_event) + L"链路挂起"; break;
		case EVENT_CONN_RESUME: return n_to_s(ademco_event) + L"链路恢复"; break;
		case EVENT_SUB_MACHINE_SENSOR_EXCEPTION: return n_to_s(ademco_event) + L"分防区异常"; break;
		case EVENT_SUB_MACHINE_SENSOR_RESUME: return n_to_s(ademco_event) + L"分防区恢复"; break;
		case EVENT_SUB_MACHINE_POWER_EXCEPTION: return n_to_s(ademco_event) + L"分防区电源异常"; break;
		case EVENT_SUB_MACHINE_POWER_RESUME: return n_to_s(ademco_event) + L"分防区电源恢复"; break;
		case EVENT_RETRIEVE_ZONE_OR_SUB_MACHINE:return n_to_s(ademco_event) + L"索要"; break;
		case EVENT_QUERY_SUB_MACHINE:return n_to_s(ademco_event) + L"查询"; break;
		case EVENT_WRITE_TO_MACHINE:return n_to_s(ademco_event) + L"写入主机信息"; break;
		case EVENT_I_AM_NET_MODULE:return n_to_s(ademco_event) + L"我是网络模块"; break;
		case EVENT_PHONE_USER_SOS:return n_to_s(ademco_event) + L"手机用户SOS"; break;
		case EVENT_I_AM_EXPRESSED_GPRS_2050_MACHINE: return n_to_s(ademco_event) + L"我是改进型卧室主机2050型"; break;
		default: return n_to_s(ademco_event) + L"未定义"; break;
		}
	}
#endif

	// 安定宝事件级别
	typedef enum EventLevel
	{
		EVENT_LEVEL_NULL,
		EVENT_LEVEL_STATUS,				// 主机状态
		EVENT_LEVEL_EXCEPTION_RESUME,	// 黄色报警
		EVENT_LEVEL_EXCEPTION,			// 橙色报警
		EVENT_LEVEL_ALARM,				// 红色报警
	}EventLevel;

	
	inline COLORREF GetEventLevelColor(EventLevel level)
	{
		switch (level) {
			case ademco::EVENT_LEVEL_EXCEPTION_RESUME:
				return RGB(0xFF, 0xFF, 0x90); // 黄色
				break;
			case ademco::EVENT_LEVEL_EXCEPTION:
				return RGB(0xFF, 0x80, 0x00); // 橙色
				break;
			case ademco::EVENT_LEVEL_ALARM:
				return RGB(0xFF, 0x18, 0x18); // 红色
				break;
			default:
				return RGB(0xFF, 0xFF, 0xFF); // 白色
				break;
		}
	}


	// 获取安定宝事件级别
	inline EventLevel GetEventLevel(ADEMCO_EVENT ademco_event)
	{
		switch (ademco_event) {
			case EVENT_ARM:			
			case EVENT_DISARM:
			case EVENT_HALFARM:
				return EVENT_LEVEL_STATUS;		
				break;
			case EVENT_RECONNECT:
			case EVENT_SERIAL485CONN:
			case EVENT_SUB_MACHINE_SENSOR_RESUME:
			case EVENT_SUB_MACHINE_POWER_RESUME:
			case EVENT_BATTERY_RECOVER:
				return EVENT_LEVEL_EXCEPTION_RESUME;
				break;
			case EVENT_LOWBATTERY:
			case EVENT_BADBATTERY:
			case EVENT_SOLARDISTURB:
			case EVENT_DISCONNECT:
			//case EVENT_DOORRINGING: //case EVENT_CONN_HANGUP: //case EVENT_CONN_RESUME:
			case EVENT_SUB_MACHINE_SENSOR_EXCEPTION:
			case EVENT_SUB_MACHINE_POWER_EXCEPTION:
				return EVENT_LEVEL_EXCEPTION;
				break;
			case EVENT_SERIAL485DIS:
			case EVENT_BURGLAR:
			case EVENT_DURESS:
			case EVENT_EMERGENCY:
			case EVENT_FIRE:
			case EVENT_GAS:
			case EVENT_TEMPER:
			case EVENT_WATER:
				return EVENT_LEVEL_ALARM;
			default: break;
		}
		return EVENT_LEVEL_NULL;
	}

	// 获取异常恢复事件所对应的异常事件
	inline const ADEMCO_EVENT GetExceptionEventByResumeEvent(ADEMCO_EVENT resume_event)
	{
		switch (resume_event) {
			case EVENT_RECONNECT: return EVENT_DISCONNECT; break;
			case EVENT_SERIAL485CONN: return EVENT_SERIAL485DIS; break;
			case EVENT_SUB_MACHINE_SENSOR_RESUME:return EVENT_SUB_MACHINE_SENSOR_EXCEPTION; break;
			case EVENT_SUB_MACHINE_POWER_RESUME:return EVENT_SUB_MACHINE_POWER_EXCEPTION; break;
			case EVENT_LOWBATTERY:	return EVENT_BATTERY_RECOVER; break;
			default:return EVENT_INVALID_EVENT; break;
		}
	}

#pragma endregion

	// 安定宝事件源
	typedef enum EventSource 
	{
		ES_UNKNOWN,
		ES_TCP_CLIENT,  // 客户端发送的事件
		ES_TCP_SERVER,  // 服务器发送的事件
		ES_SMS,			// 接警中心短信模块发送的事件
	}EventSource;

	// 安定宝事件
	typedef struct AdemcoEvent
	{
		EventSource _resource;  // 来源
		int _event;				// 事件码
		int _zone;				// 防区号
		int _sub_zone;			// 分防区号
		time_t _timestamp;		// 时间戳
		time_t _recv_time;		// 接收时间

		char_array_ptr _xdata;
		
		AdemcoEvent() : _resource(ES_UNKNOWN), _event(0), _zone(0), _sub_zone(0), _timestamp(0),
			_recv_time(0), _xdata()
		{}

		AdemcoEvent(EventSource resource, int ademco_event, int zone, int sub_zone, const time_t& timestamp,
					const time_t& recv_time, const char_array_ptr& xdata = nullptr)
			: _resource(resource), _event(ademco_event), _zone(zone), _sub_zone(sub_zone),
			_timestamp(timestamp), _recv_time(recv_time), _xdata(xdata)
		{}

		AdemcoEvent(const AdemcoEvent& rhs)
			: _resource(rhs._resource), _event(rhs._event), _zone(rhs._zone), _sub_zone(rhs._sub_zone),
			_timestamp(rhs._timestamp), _recv_time(rhs._recv_time), _xdata(rhs._xdata)
		{}

		~AdemcoEvent()
		{}

		AdemcoEvent& operator=(const AdemcoEvent& rhs)
		{
			_resource = rhs._resource;
			_event = rhs._event;
			_zone = rhs._zone;
			_sub_zone = rhs._sub_zone;
			_timestamp = rhs._timestamp;
			_recv_time = rhs._recv_time;
			_xdata = rhs._xdata;
			return *this;
		}

		bool operator == (const AdemcoEvent& rhs)
		{
			return (_event == rhs._event)
				&& (_zone == rhs._zone)
				&& (_sub_zone == rhs._sub_zone);
		}
	}AdemcoEvent;

	typedef std::shared_ptr<AdemcoEvent> AdemcoEventPtr;

	template <typename dlg_type>
	class AdemcoEventObserver : public dp::observer<AdemcoEventPtr>
	{
	public:
		explicit AdemcoEventObserver(dlg_type* dlg) : _dlg(dlg) {}
		
		virtual void on_update(const ademco::AdemcoEventPtr& ptr) {
			if (_dlg) {
				_dlg->OnAdemcoEventResult(ptr);
			}
		}
	private:
		dlg_type* _dlg;
	};
	

};
