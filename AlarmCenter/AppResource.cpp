#include "stdafx.h"
#include "AppResource.h"
#include "resource.h"
#include "ademco_event.h"

IMPLEMENT_SINGLETON(CAppResource)

HICON CAppResource::m_hIconArm = NULL;
HICON CAppResource::m_hIconDisarm = NULL;
HICON CAppResource::m_hIconNetOk = NULL;
HICON CAppResource::m_hIconNetFailed = NULL;
HICON CAppResource::m_hIconEmergency = NULL;

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
	eventSubSensorException.LoadStringW(IDS_STRING_SUB_MACHINE_SENSOR_EXCEPTION);
	eventSubSensorResume.LoadStringW(IDS_STRING_SUB_MACHINE_SENSOR_RESUME);
	eventSubPowerException.LoadStringW(IDS_STRING_SUB_MACHINE_POWER_EXCEPTION);
	eventSubPowerResume.LoadStringW(IDS_STRING_SUB_MACHINE_POWER_RESUME);
	eventUnknown.LoadStringW(IDS_STRING_UNKNOWNEVENT);

	m_hIconArm = (HICON)::LoadImage(AfxGetApp()->m_hInstance,
									MAKEINTRESOURCE(IDI_ICON_ARM),
									IMAGE_ICON, 32, 32,
									LR_DEFAULTCOLOR);

	m_hIconDisarm = (HICON)::LoadImage(AfxGetApp()->m_hInstance,
									   MAKEINTRESOURCE(IDI_ICON_DISARM),
									   IMAGE_ICON, 32, 32,
									   LR_DEFAULTCOLOR);

	m_hIconNetOk = (HICON)::LoadImage(AfxGetApp()->m_hInstance,
									  MAKEINTRESOURCE(IDI_ICON_NETOK),
									  IMAGE_ICON, 32, 32,
									  LR_DEFAULTCOLOR);

	m_hIconNetFailed = (HICON)::LoadImage(AfxGetApp()->m_hInstance,
										  MAKEINTRESOURCE(IDI_ICON_NETFAIL),
										  IMAGE_ICON, 32, 32,
										  LR_DEFAULTCOLOR);

	m_hIconEmergency = (HICON)::LoadImage(AfxGetApp()->m_hInstance,
										  MAKEINTRESOURCE(IDI_ICON_EMERGENCY),
										  IMAGE_ICON, 32, 32,
										  LR_DEFAULTCOLOR);
}


CAppResource::~CAppResource()
{
	if (m_hIconArm) { DeleteObject(m_hIconArm); }
	if (m_hIconDisarm) { DeleteObject(m_hIconDisarm); }
	if (m_hIconNetOk) { DeleteObject(m_hIconNetOk); }
	if (m_hIconNetFailed) { DeleteObject(m_hIconNetFailed); }
}


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
		case ademco::EVENT_SUB_MACHINE_SENSOR_EXCEPTION:
			return eventSubSensorException;
			break;
		case ademco::EVENT_SUB_MACHINE_SENSOR_RESUME:
			return eventSubSensorResume;
			break; 
		case ademco::EVENT_SUB_MACHINE_POWER_EXCEPTION:
			return eventSubPowerException;
			break; 
		case ademco::EVENT_SUB_MACHINE_POWER_RESUME:
			return eventSubPowerResume;
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
