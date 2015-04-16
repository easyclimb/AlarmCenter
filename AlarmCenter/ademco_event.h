#pragma once 

namespace ademco
{
#pragma region event_definetion

	static const int EVENT_CLEARMSG			= 0x000FFFFF;
	static const int EVENT_SUBMACHINECNT	= EVENT_CLEARMSG + 1;

	static const int EVENT_ARM				= 3400;
	static const int EVENT_DISARM			= 1400;
	static const int EVENT_HALFARM			= 3456;
	static const int EVENT_EMERGENCY		= 1120;

	static const int EVENT_BURGLAR			= 1130;
	static const int EVENT_DOORRINGING		= 1134;
	static const int EVENT_FIRE				= 1110;
	static const int EVENT_DURESS			= 1121;
	static const int EVENT_GAS				= 1151;
	static const int EVENT_WATER			= 1113;
	static const int EVENT_TEMPER			= 1137;

	static const int EVENT_LOWBATTERY		= 1302;
	static const int EVENT_BADBATTERY		= 1311;
	static const int EVENT_SOLARDISTURB		= 1387;
	static const int EVENT_DISCONNECT		= 1381;
	static const int EVENT_RECONNECT		= 3381;

	static const int EVENT_SERIAL485DIS		= 1485;
	static const int EVENT_SERIAL485CONN	= 3485;

	static const int EVENT_CONN_HANGUP		= 1700;
	static const int EVENT_CONN_RESUME		= 3700;

	static const int gc_AdemcoEvent[] = {
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
	};

	static bool IsExceptionEvent(int ademco_event)
	{
		return (EVENT_LOWBATTERY	== ademco_event)
			|| (EVENT_BADBATTERY	== ademco_event)
			|| (EVENT_SOLARDISTURB	== ademco_event)
			|| (EVENT_DISCONNECT	== ademco_event)
			|| (EVENT_RECONNECT		== ademco_event)
			|| (EVENT_SERIAL485DIS	== ademco_event)
			|| (EVENT_SERIAL485CONN == ademco_event);
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
