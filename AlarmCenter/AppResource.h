#pragma once
class CAppResource
{
public:
	~CAppResource();
	CString AdemcoEventToString(int ademco_event);

	static HICON m_hIconArm;
	static HICON m_hIconDisarm;
	static HICON m_hIconNetOk;
	static HICON m_hIconNetFailed;
	static HICON m_hIconEmergency;
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

