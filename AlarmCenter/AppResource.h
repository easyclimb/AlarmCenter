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
	static HICON m_hIcon_Offline_Arm;
	static HICON m_hIcon_Offline_Disarm;
	static HICON m_hIcon_Online_Arm;
	static HICON m_hIcon_Online_Disarm;
	static HICON m_hIcon_Offline_Arm_Hassubmachine;
	static HICON m_hIcon_Offline_Disarm_Hassubmachine;
	static HICON m_hIcon_Online_Arm_Hassubmachine;
	static HICON m_hIcon_Online_Disarm_Hassubmachine;
	static HICON m_hIcon_Gsm_Arm;
	static HICON m_hIcon_Gsm_Disarm;
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
	CString eventBatteryRecover;
	CString eventBadBattery;
	CString eventSolarDisturb;
	CString eventDisconnect;
	CString eventReconnect;
	CString event485Disconn;
	CString event485Reconn;
	CString eventSubSensorException;
	CString eventSubSensorResume;
	CString eventSubPowerException;
	CString eventSubPowerResume;
	CString eventUnknown;

	

	DECLARE_UNCOPYABLE(CAppResource)
	DECLARE_SINGLETON(CAppResource)
};

