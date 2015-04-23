#pragma once 

typedef const int ADEMCO_EVENT;

namespace ademco
{
#pragma region event_definetion

	// �Ӿ������ڲ�ʹ���¼�
	static ADEMCO_EVENT EVENT_CLEARMSG		= 0x00010000;
	static ADEMCO_EVENT EVENT_SUBMACHINECNT	= EVENT_CLEARMSG + 1;
	static ADEMCO_EVENT EVENT_MACHINE_ALIAS	= EVENT_CLEARMSG + 2;
	static ADEMCO_EVENT EVENT_INVALID_EVENT = 0xFFFFFFFF;

	// ��׼������Э���¼�
	static ADEMCO_EVENT EVENT_ARM				= 3400;
	static ADEMCO_EVENT EVENT_DISARM			= 1400;
	static ADEMCO_EVENT EVENT_HALFARM			= 3456;
	static ADEMCO_EVENT EVENT_EMERGENCY			= 1120;

	static ADEMCO_EVENT EVENT_BURGLAR			= 1130;
	static ADEMCO_EVENT EVENT_DOORRINGING		= 1134;
	static ADEMCO_EVENT EVENT_FIRE				= 1110;
	static ADEMCO_EVENT EVENT_DURESS			= 1121;
	static ADEMCO_EVENT EVENT_GAS				= 1151;
	static ADEMCO_EVENT EVENT_WATER				= 1113;
	static ADEMCO_EVENT EVENT_TEMPER			= 1137;

	static ADEMCO_EVENT EVENT_LOWBATTERY		= 1302;
	static ADEMCO_EVENT EVENT_BADBATTERY		= 1311;
	static ADEMCO_EVENT EVENT_SOLARDISTURB		= 1387;
	static ADEMCO_EVENT EVENT_DISCONNECT		= 1381;
	static ADEMCO_EVENT EVENT_RECONNECT			= 3381;

	static ADEMCO_EVENT EVENT_SERIAL485DIS		= 1485;
	static ADEMCO_EVENT EVENT_SERIAL485CONN		= 3485;

	static ADEMCO_EVENT EVENT_CONN_HANGUP		= 1700;
	static ADEMCO_EVENT EVENT_CONN_RESUME		= 3700;

	// ˽���¼�
	static ADEMCO_EVENT EVENT_DISARM_PWD_ERR				= 1701; // �����������
	static ADEMCO_EVENT EVENT_SUB_MACHINE_SENSOR_EXCEPTION	= 1702; // �ֻ�̽ͷ�쳣
	static ADEMCO_EVENT EVENT_SUB_MACHINE_SENSOR_RESUME		= 3702; // �ֻ�̽ͷ�ָ�
	static ADEMCO_EVENT EVENT_SUB_MACHINE_POWER_EXCEPTION	= 1703; // �ֻ���Դ�쳣
	static ADEMCO_EVENT EVENT_SUB_MACHINE_POWER_RESUME		= 3703; // �ֻ���Դ�ָ�
	static ADEMCO_EVENT EVENT_RETRIEVE_SUB_MACHINE			= 1704; // ��Ҫ�ֻ���Ϣ
	static ADEMCO_EVENT EVENT_QUERY_SUB_MACHINE				= 1705; // ��ѯ�ֻ���Ϣ
	static ADEMCO_EVENT EVENT_WRITE_TO_MACHINE				= 1706; // д��������Ϣ
	static ADEMCO_EVENT EVENT_I_AM_NET_MODULE				= 1707; // ��������ģ��

	static ADEMCO_EVENT gc_AdemcoEvent[] = {
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
	};

	/*static bool IsExceptionEvent(int ademco_event)
	{
		return (EVENT_LOWBATTERY	== ademco_event)
			|| (EVENT_BADBATTERY	== ademco_event)
			|| (EVENT_SOLARDISTURB	== ademco_event)
			|| (EVENT_DISCONNECT	== ademco_event)
			|| (EVENT_RECONNECT		== ademco_event)
			|| (EVENT_SERIAL485DIS	== ademco_event)
			|| (EVENT_SERIAL485CONN == ademco_event);
	}*/

	static const char* GetAdemcoEventString(ADEMCO_EVENT ademco_event)
	{
		switch (ademco_event) {
			case EVENT_ARM:			return "ARM";		break;
			case EVENT_BURGLAR:		return "BURGLAR";	break;
			case EVENT_DISARM:		return "DISARM";	break;
			case EVENT_DURESS:		return "DURESS";	break;
			case EVENT_EMERGENCY:	return "EMERGENCY";	break;
			case EVENT_FIRE:		return "FIRE";		break;
			case EVENT_GAS:			return "GAS";		break;
			case EVENT_HALFARM:		return "HALFARM";	break;
			case EVENT_TEMPER:		return "TEMPER";	break;
			case EVENT_WATER:		return "WATER";		break;
			case EVENT_LOWBATTERY:	return "LOWBATTERY";	break;
			case EVENT_BADBATTERY:	return "BADBATTERY";	break;
			case EVENT_SOLARDISTURB:return "SOLARDISTURB";	break;
			case EVENT_DISCONNECT:	return "DISCONNECT";		break;
			case EVENT_RECONNECT:	return "RECONNECT";		break;
			case EVENT_SERIAL485DIS:return "485DIS";	break;
			case EVENT_SERIAL485CONN:return "485CONN";	break;
			case EVENT_DOORRINGING:	return "DOORRINGING";	break;
			case EVENT_CONN_HANGUP: return "CONN_HANGUP"; break;
			case EVENT_CONN_RESUME: return "CONN_RESUME"; break;
			case EVENT_SUB_MACHINE_SENSOR_EXCEPTION: return "SUB_SENSOR_EXCEPTION"; break;
			case EVENT_SUB_MACHINE_SENSOR_RESUME: return "SUB_SENSOR_RESUME"; break;
			case EVENT_SUB_MACHINE_POWER_EXCEPTION: return "SUB_POWER_EXCEPTION"; break;
			case EVENT_SUB_MACHINE_POWER_RESUME: return "SUB_POWER_RESUME"; break;
			default: return "null"; break;
		}
	}

	typedef enum EventLevel
	{
		EVENT_LEVEL_NULL,
		EVENT_LEVEL_STATUS,
		EVENT_LEVEL_EXCEPTION_RESUME,	// ��ɫ����
		EVENT_LEVEL_EXCEPTION,			// ��ɫ����
		EVENT_LEVEL_ALARM,				// ��ɫ����
	}EventLevel;

	static COLORREF GetEventLevelColor(EventLevel level)
	{
		switch (level) {
			//case ademco::EVENT_LEVEL_NULL:
			//case ademco::EVENT_LEVEL_STATUS:
			//	return 0x00FFFFFF; // ��ɫ
			//	break;
			case ademco::EVENT_LEVEL_EXCEPTION_RESUME:
				return RGB(0xFF, 0xFF, 0x80); // ��ɫ
				break;
			case ademco::EVENT_LEVEL_EXCEPTION:
				return RGB(0xFF, 0x80, 0x00); // ��ɫ
				break;
			case ademco::EVENT_LEVEL_ALARM:
				return RGB(0xFF, 0x00, 0x00); // ��ɫ
				break;
			default:
				return RGB(0xFF, 0xFF, 0xFF); // ��ɫ
				break;
		}
	}

	static EventLevel GetEventLevel(ADEMCO_EVENT ademco_event)
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

	static ADEMCO_EVENT GetExceptionEventByResumeEvent(ADEMCO_EVENT resume_event)
	{
		switch (resume_event) {
			case EVENT_RECONNECT: return EVENT_DISCONNECT; break;
			case EVENT_SERIAL485CONN: return EVENT_SERIAL485DIS; break;
			case EVENT_SUB_MACHINE_SENSOR_RESUME:return EVENT_SUB_MACHINE_SENSOR_EXCEPTION; break;
			case EVENT_SUB_MACHINE_POWER_RESUME:return EVENT_SUB_MACHINE_POWER_EXCEPTION; break;
			default:return EVENT_INVALID_EVENT; break;
		}
	}

#pragma endregion

	typedef struct AdemcoEvent
	{
		int _event;
		int _zone;
		int _sub_zone;
		time_t _time;

		AdemcoEvent() : _event(0), _zone(0), _sub_zone(0), _time(0) {}

		AdemcoEvent(int ademco_event, int zone, int sub_zone, const time_t& event_time)
			: _event(ademco_event), _zone(zone), _sub_zone(sub_zone), _time(event_time)
		{}

		AdemcoEvent(const AdemcoEvent& rhs)
			: _event(rhs._event), _zone(rhs._zone), _sub_zone(rhs._sub_zone), _time(rhs._time)
		{}

		AdemcoEvent& operator=(const AdemcoEvent& rhs)
		{
			_event = rhs._event;
			_zone = rhs._zone;
			_sub_zone = rhs._sub_zone;
			_time = rhs._time;
			return *this;
		}

		bool operator == (const AdemcoEvent& rhs)
		{
			return (_event == rhs._event)
				&& (_zone == rhs._zone)
				&& (_sub_zone == rhs._sub_zone);
		}
	}AdemcoEvent;

	//static AdemcoEvent g_clearMsgEvent();

	typedef void(_stdcall *AdemcoEventCB)(void* udata, const AdemcoEvent* ademcoEvent);

#define IMPLEMENT_ADEMCO_EVENT_CALL_BACK(class_name, function_name) \
	static void _stdcall function_name(void* udata, const AdemcoEvent* ademcoEvent) \
	{ \
		class_name* object = reinterpret_cast<class_name*>(udata); assert(object); \
		object->function_name##Result(ademcoEvent); \
	}


	

};
