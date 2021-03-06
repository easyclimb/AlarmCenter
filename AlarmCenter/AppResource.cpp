#include "stdafx.h"
#include "AlarmCenter.h"
#include "ademco_event.h"

//IMPLEMENT_SINGLETON(CAppResource)

HICON CAppResource::m_hIconArm = nullptr;
HICON CAppResource::m_hIconHalfarm = nullptr;
HICON CAppResource::m_hIconDisarm = nullptr;
HICON CAppResource::m_hIconNetOk = nullptr;
HICON CAppResource::m_hIconNetFailed = nullptr;
HICON CAppResource::m_hIconEmergency = nullptr;

HICON CAppResource::m_hIcon_Offline_Arm = nullptr;
HICON CAppResource::m_hIcon_Offline_Halfarm = nullptr;
HICON CAppResource::m_hIcon_Offline_Disarm = nullptr;
HICON CAppResource::m_hIcon_Online_Arm = nullptr;
HICON CAppResource::m_hIcon_Online_Halfarm = nullptr;
HICON CAppResource::m_hIcon_Online_Disarm = nullptr;
HICON CAppResource::m_hIcon_Offline_Arm_Hassubmachine = nullptr;
HICON CAppResource::m_hIcon_Offline_Halfarm_Hassubmachine = nullptr;
HICON CAppResource::m_hIcon_Offline_Disarm_Hassubmachine = nullptr;
HICON CAppResource::m_hIcon_Online_Arm_Hassubmachine = nullptr;
HICON CAppResource::m_hIcon_Online_Halfarm_Hassubmachine = nullptr;
HICON CAppResource::m_hIcon_Online_Disarm_Hassubmachine = nullptr;
HICON CAppResource::m_hIcon_Gsm_Arm = nullptr;
HICON CAppResource::m_hIcon_Gsm_Halfarm = nullptr;
HICON CAppResource::m_hIcon_Gsm_Disarm = nullptr;

namespace detail {
	HICON HICONFromCBitmap(CBitmap& bitmap)
	{
		BITMAP bmp;
		bitmap.GetBitmap(&bmp);

		HBITMAP hbmMask = ::CreateCompatibleBitmap(::GetDC(nullptr),
												   bmp.bmWidth, bmp.bmHeight);

		ICONINFO ii = { 0 };
		ii.fIcon = TRUE;
		ii.hbmColor = bitmap;
		ii.hbmMask = hbmMask;

		HICON hIcon = ::CreateIconIndirect(&ii);
		::DeleteObject(hbmMask);

		return hIcon;
	}
};

CAppResource::CAppResource()
{
	//InitStringResource();
	eventArm = TR(IDS_STRING_ARM);
	eventDisArm = TR(IDS_STRING_DISARM);
	eventHalfArm = TR(IDS_STRING_HALFARM);
	eventEmergency = TR(IDS_STRING_EMERGENCY);
	eventBurglar = TR(IDS_STRING_BURGLAR);
	eventDoorring = TR(IDS_STRING_DOORRING);
	eventFire = TR(IDS_STRING_FIRE);
	eventDuress = TR(IDS_STRING_DURESS);
	eventGas = TR(IDS_STRING_GAS);
	eventWater = TR(IDS_STRING_WATER);
	eventTemper = TR(IDS_STRING_TEMPER);
	eventZoneTemper = TR(IDS_STRING_ZONE_TEMPER);

	eventLowBattery = TR(IDS_STRING_LOWBATTERY);
	eventBatteryRecover = TR(IDS_STRING_BATTERY_RECOVER);
	eventBadBattery = TR(IDS_STRING_BADBATTERY);
	eventSolarDisturb = TR(IDS_STRING_SOLARDISTURB);
	eventDisconnect = TR(IDS_STRING_DISCONNECT);
	eventReconnect = TR(IDS_STRING_RECONNECT);
	eventBatteryException = TR(IDS_STRING_ZONE_BATTERY_EXCEPTION);
	eventBatteryExceptionRecover = TR(IDS_STRING_ZONE_BATTERY_EXCEPTION_RECOVER);
	eventOtherException = TR(IDS_STRING_ZONE_OTHER_EXCEPTION);
	eventOtherExceptionRecover = TR(IDS_STRING_ZONE_OTHER_EXCEPTION_RECOVER);

	event485Disconn = TR(IDS_STRING_485DIS);
	event485Reconn = TR(IDS_STRING_485CONN);
	eventSubSensorException = TR(IDS_STRING_SUB_MACHINE_SENSOR_EXCEPTION);
	eventSubSensorResume = TR(IDS_STRING_SUB_MACHINE_SENSOR_RESUME);
	eventSubPowerException = TR(IDS_STRING_SUB_MACHINE_POWER_EXCEPTION);
	eventSubPowerResume = TR(IDS_STRING_SUB_MACHINE_POWER_RESUME);
	eventUnknown = TR(IDS_STRING_UNKNOWNEVENT);


	m_hIconArm = (HICON)::LoadImage(AfxGetApp()->m_hInstance,
									MAKEINTRESOURCE(IDI_ICON_ARM),
									IMAGE_ICON, 32, 32,
									LR_DEFAULTCOLOR);
	
	m_hIconHalfarm = (HICON)::LoadImage(AfxGetApp()->m_hInstance,
									MAKEINTRESOURCE(IDI_ICON_HALFARM),
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

	using namespace detail;
	CBitmap bmp;
	bmp.LoadBitmapW(IDB_BITMAP_OFFLINE_ARM);
	m_hIcon_Offline_Arm = HICONFromCBitmap(bmp);
	bmp.DeleteObject();
	
	bmp.LoadBitmapW(IDB_BITMAP_OFFLINE_HALFARM);
	m_hIcon_Offline_Halfarm = HICONFromCBitmap(bmp);
	bmp.DeleteObject();

	bmp.LoadBitmapW(IDB_BITMAP_OFFLINE_DISARM);
	m_hIcon_Offline_Disarm = HICONFromCBitmap(bmp);
	bmp.DeleteObject();

	bmp.LoadBitmapW(IDB_BITMAP_ONLINE_ARM);
	m_hIcon_Online_Arm = HICONFromCBitmap(bmp);
	bmp.DeleteObject();
	
	bmp.LoadBitmapW(IDB_BITMAP_ONLINE_HALFARM);
	m_hIcon_Online_Halfarm = HICONFromCBitmap(bmp);
	bmp.DeleteObject();

	bmp.LoadBitmapW(IDB_BITMAP_ONLINE_DISARM);
	m_hIcon_Online_Disarm = HICONFromCBitmap(bmp);
	bmp.DeleteObject();

	bmp.LoadBitmapW(IDB_BITMAP_OFFLINE_ARM_HSM);
	m_hIcon_Offline_Arm_Hassubmachine = HICONFromCBitmap(bmp);
	bmp.DeleteObject();
	
	bmp.LoadBitmapW(IDB_BITMAP_OFFLINE_HALFARM_HSM);
	m_hIcon_Offline_Halfarm_Hassubmachine = HICONFromCBitmap(bmp);
	bmp.DeleteObject();

	bmp.LoadBitmapW(IDB_BITMAP_OFFLINE_DISARM_HSM);
	m_hIcon_Offline_Disarm_Hassubmachine = HICONFromCBitmap(bmp);
	bmp.DeleteObject();

	bmp.LoadBitmapW(IDB_BITMAP_ONLINE_HALFARM_HSM);
	m_hIcon_Online_Halfarm_Hassubmachine = HICONFromCBitmap(bmp);
	bmp.DeleteObject();

	bmp.LoadBitmapW(IDB_BITMAP_ONLINE_ARM_HSM);
	m_hIcon_Online_Arm_Hassubmachine = HICONFromCBitmap(bmp);
	bmp.DeleteObject();

	bmp.LoadBitmapW(IDB_BITMAP_ONLINE_DISARM_HSM);
	m_hIcon_Online_Disarm_Hassubmachine = HICONFromCBitmap(bmp);
	bmp.DeleteObject();
	
	bmp.LoadBitmapW(IDB_BITMAP_GSM_ARM);
	m_hIcon_Gsm_Arm = HICONFromCBitmap(bmp);
	bmp.DeleteObject();
	
	bmp.LoadBitmapW(IDB_BITMAP_GSM_HALFARM);
	m_hIcon_Gsm_Halfarm = HICONFromCBitmap(bmp);
	bmp.DeleteObject();

	bmp.LoadBitmapW(IDB_BITMAP_GSM_DISARM);
	m_hIcon_Gsm_Disarm = HICONFromCBitmap(bmp);
	bmp.DeleteObject();

	
}


CAppResource::~CAppResource()
{
#define DELETE_OBJECT(obj) if (obj) { DeleteObject(obj); obj = nullptr;}

	DELETE_OBJECT(m_hIconArm);
	DELETE_OBJECT(m_hIconDisarm);
	DELETE_OBJECT(m_hIconNetOk);
	DELETE_OBJECT(m_hIconNetFailed);
	DELETE_OBJECT(m_hIcon_Offline_Arm);
	DELETE_OBJECT(m_hIcon_Offline_Disarm);
	DELETE_OBJECT(m_hIcon_Online_Arm);
	DELETE_OBJECT(m_hIcon_Online_Disarm);
	DELETE_OBJECT(m_hIcon_Offline_Arm_Hassubmachine);
	DELETE_OBJECT(m_hIcon_Offline_Disarm_Hassubmachine);
	DELETE_OBJECT(m_hIcon_Online_Arm_Hassubmachine);
	DELETE_OBJECT(m_hIcon_Online_Disarm_Hassubmachine);
	DELETE_OBJECT(m_hIcon_Gsm_Arm);
	DELETE_OBJECT(m_hIcon_Gsm_Disarm);

	//m_strResourceMap.clear();
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
		case ademco::EVENT_ZONE_TEMPER:
			return eventZoneTemper;
			break;

		case ademco::EVENT_LOWBATTERY:
			return eventLowBattery;
			break;
		case ademco::EVENT_BATTERY_RECOVER:
			return eventBatteryRecover;
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
		case ademco::EVENT_RECONNECT:
			return eventReconnect;
			break;
		case ademco::EVENT_BATTERY_EXCEPTION:
			return eventBatteryException;
			break;
		case ademco::EVENT_BATTERY_EXCEPTION_RECOVER:
			return eventBatteryExceptionRecover;
			break;
		case ademco::EVENT_OTHER_EXCEPTION:
			return eventOtherException;
			break;
		case ademco::EVENT_OTHER_EXCEPTION_RECOVER:
			return eventOtherExceptionRecover;
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

//
//void CAppResource::InitStringResource()
//{
//	AUTO_LOG_FUNCTION;
//	for (unsigned int i = 101; i <= 600; i++) {
//		CString str;
//		if(str.LoadStringW(i))
//			if(!str.IsEmpty())
//				m_strResourceMap[i] = str;
//		//JLOG(str);
//	}
//}
//
//
//CString CAppResource::GetString(unsigned int res_id)
//{
//	std::lock_guard<std::mutex> lock(m_mutex);
//	auto iter = m_strResourceMap.find(res_id);
//	if (iter != m_strResourceMap.end()) {
//		return iter->second;
//	} else {
//		CString ret; ret.Format(L"invalid string id: %d", res_id);
//		return ret;
//	}
//}


CString CAppResource::MachineStatusToString(core::machine_status ms)
{
	switch (ms)
	{
	case core::MACHINE_ARM:
		return TR(IDS_STRING_ARM);
		break;
	case core::MACHINE_HALFARM:
		return TR(IDS_STRING_HALFARM);
		break;
	case core::MACHINE_DISARM:
	case core::MACHINE_STATUS_UNKNOWN:
	default:
		return TR(IDS_STRING_DISARM);
		break;
	}
}

CString CAppResource::MachineTypeToString(core::machine_type mt)
{
	switch (mt) {
		case core::MT_WIFI:
			return TR(IDS_STRING_MT_WIFI);
			break;
		case core::MT_NETMOD:
			return TR(IDS_STRING_MT_NETMOD);
			break;
		case core::MT_IMPRESSED_GPRS_MACHINE_2050:
			return TR(IDS_STRING_MT_GPRS);
			break;
		case core::MT_LCD:
			return TR(IDS_STRING_MT_LCD);
			break;
		case core::MT_WIRE:
			return TR(IDS_STRING_MT_WIRE);
			break;
		default:
			return TR(IDS_STRING_UNKNOWN_MT);
			break;
	}
}
