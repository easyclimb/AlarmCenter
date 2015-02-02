#pragma once 

namespace ademco
{
	static const int EVENT_ARM = 3400;
	static const int EVENT_DISARM = 1400;
	static const int EVENT_HALFARM = 3456;
	static const int EVENT_EMERGENCY = 1120;
	static const int EVENT_BURGLAR = 1130;
	static const int EVENT_FIRE = 1110;
	static const int EVENT_DURESS = 1121;
	static const int EVENT_GAS = 1151;
	static const int EVENT_WATER = 1113;
	static const int EVENT_TEMPER = 1137;
	static const int EVENT_LOWBATTERY = 1384;
	static const int EVENT_SOLARDISTURB = 1387;
	static const int EVENT_DISCONNECT = 1381;
	static const int EVENT_SERIAL485DIS = 1485;
	static const int EVENT_SERIAL485CONN = 3485;
	static const int EVENT_DOORRINGING = 1134;

	static const int gc_AdemcoEvent[] = {
		EVENT_ARM,
		EVENT_DISARM,
		EVENT_HALFARM,
		EVENT_EMERGENCY,
		EVENT_BURGLAR,
		EVENT_FIRE,
		EVENT_DURESS,
		EVENT_GAS,
		EVENT_WATER,
		EVENT_TEMPER,
		EVENT_LOWBATTERY,
		EVENT_SOLARDISTURB,
		EVENT_DISCONNECT,
		EVENT_SERIAL485DIS,
		EVENT_SERIAL485CONN,
		EVENT_DOORRINGING,
	};

};
