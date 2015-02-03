#pragma once 

namespace ademco
{
#pragma region event_definetion
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
	};

	static bool IsExceptionEvent(int ademco_event)
	{
		return (EVENT_LOWBATTERY	== ademco_event)
			|| (EVENT_BADBATTERY	== ademco_event)
			|| (EVENT_SOLARDISTURB	== ademco_event)
			|| (EVENT_DISCONNECT	== ademco_event)
			|| (EVENT_RECONNECT		== ademco_event);
	}
#pragma endregion

	typedef struct AdemcoEvent
	{
		int _zone;
		int _ademco_event;
		time_t _time;

		AdemcoEvent() : _zone(0), _ademco_event(0), _time(0) {}

		AdemcoEvent(int zone, int ademco_event, const time_t& event_time)
			: _zone(zone), _ademco_event(ademco_event), _time(event_time)
		{}

		AdemcoEvent& operator=(const AdemcoEvent& rhs)
		{
			_zone = rhs._zone;
			_ademco_event = rhs._ademco_event;
			_time = rhs._time;
			return *this;
		}
	}AdemcoEvent;


	typedef void(_stdcall *AdemcoEventCB)(void* udata,
										  int zone,
										  int ademco_event,
										  const time_t& event_time);

#define IMPLEMENT_ADEMCO_EVENT_CALL_BACK(class_name, function_name) \
	static void _stdcall function_name(void* udata, \
										int zone, \
										int ademco_event, \
										const time_t& event_time) \
	{ \
		class_name* object = reinterpret_cast<class_name*>(udata); \
		assert(object); \
		object->function_name##Result(zone, ademco_event, event_time); \
	}



};
