#include "stdafx.h"
#include "AppResource.h"
#include "resource.h"
#include "ademco_event.h"

IMPLEMENT_SINGLETON(CAppResource)

CAppResource::CAppResource()
{
	eventArm.LoadStringW(IDS_STRING_ARM);
	eventDisArm.LoadStringW(IDS_STRING_DISARM);
	eventHalfArm.LoadStringW(IDS_STRING_HALFARM);
	eventEmergency.LoadStringW(IDS_STRING_EMERGENCY);
	eventBurglar.LoadStringW(IDS_STRING_BURGLAR);
	eventDoorring.LoadStringW(IDS_STRING_DOORRING);
	eventFire.LoadStringW(IDS_STRING_FIRE);
	eventDuress.LoadStringW(IDS_STRING_DURESS);
	eventGas.LoadStringW(IDS_STRING_GAS);
	eventWater.LoadStringW(IDS_STRING_WATER);
	eventTemper.LoadStringW(IDS_STRING_TEMPER);
	eventLowBattery.LoadStringW(IDS_STRING_LOWBATTERY);
	eventBadBattery.LoadStringW(IDS_STRING_BADBATTERY);
	eventSolarDisturb.LoadStringW(IDS_STRING_SOLARDISTURB);
	eventDisconnect.LoadStringW(IDS_STRING_DISCONNECT);
	eventReconnect.LoadStringW(IDS_STRING_RECONNECT);
	event485Disconn.LoadStringW(IDS_STRING_485DIS);
	event485Reconn.LoadStringW(IDS_STRING_485CONN);
	eventUnknown.LoadStringW(IDS_STRING_UNKNOWNEVENT);
}


CAppResource::~CAppResource()
{}


CString CAppResource::AdemcoEventToString(int ademco_event)
{
	switch (ademco_event) {
		case ademco::EVENT_ARM:
			return eventArm;
			break;
		case ademco::EVENT_DISARM:
			return eventDisArm;
			break;
		case ademco::EVENT_HALFARM:
			return eventHalfArm;
			break;
		case ademco::EVENT_EMERGENCY:
			return eventEmergency;
			break;
		case ademco::EVENT_BURGLAR:
			return eventBurglar;
			break;
		case ademco::EVENT_FIRE:
			return eventFire;
			break;
		case ademco::EVENT_DURESS:
			return eventDuress;
			break;
		case ademco::EVENT_GAS:
			return eventGas;
			break;
		case ademco::EVENT_WATER:
			return eventWater;
			break;
		case ademco::EVENT_TEMPER:
			return eventTemper;
			break;
		case ademco::EVENT_LOWBATTERY:
			return eventLowBattery;
			break;
		case ademco::EVENT_BADBATTERY:
			return eventBadBattery;
			break;
		case ademco::EVENT_SOLARDISTURB:
			return eventSolarDisturb;
			break;
		case ademco::EVENT_DISCONNECT:
			return eventDisconnect;
			break;
		case ademco::EVENT_SERIAL485DIS:
			return event485Disconn;
			break;
		case ademco::EVENT_SERIAL485CONN:
			return event485Reconn;
			break;
		case ademco::EVENT_DOORRINGING:
			return eventDoorring;
			break;
		default:
		{
			CString eventNumber;
			eventNumber.Format(L"%d", ademco_event);
			return eventUnknown + eventNumber;
			break;
		}
			
	}

}
