#pragma once
class CAppResource
{
public:
	~CAppResource();
	CString AdemcoEventToString(int ademco_event);
private:
	CString eventArm;
	CString eventDisArm;
	CString eventHalfArm;
	CString eventEmergency;
	CString eventBurglar;
	CString eventDoorring;
	CString eventFire;
	CString eventDuress;
	CString eventGas;
	CString eventWater;
	CString eventTemper;
	CString eventLowBattery;
	CString eventBadBattery;
	CString eventSolarDisturb;
	CString eventDisconnect;
	CString eventReconnect;
	CString event485Disconn;
	CString event485Reconn;
	CString eventUnknown;

	DECLARE_UNCOPYABLE(CAppResource)
	DECLARE_SINGLETON(CAppResource)
};

