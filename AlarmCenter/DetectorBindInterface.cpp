#include "stdafx.h"
#include "DetectorBindInterface.h"
#include "DetectorInfo.h"
#include "AlarmMachineManager.h"

namespace core {

detector_bind_interface::~detector_bind_interface() {
	_iczcList.clear();
}


bool detector_bind_interface::execute_update_detector_info_field(DetectorInfoField dif, int value)
{
	AUTO_LOG_FUNCTION;
	ASSERT(_detectorInfo);
	CString query, filed;
	switch (dif) {
	case DIF_X:
		filed = L"x";
		break;
	case DIF_Y:
		filed = L"y";
		break;
	case DIF_DISTANCE:
		filed = L"distance";
		break;
	case DIF_ANGLE:
		filed = L"angle";
		break;
	default:
		return false;
		break;
	}
	CString tableName;
	if (DIT_ZONE_INFO == GetInterfaceType()) {
		tableName = L"DetectorInfo";
	} else if (DIT_CAMERA_INFO == GetInterfaceType()) {
		tableName = L"DetectorInfoOfCamera";
	} else {
		assert(0); return false;
	}
	query.Format(L"update %s set %s=%d where id=%d", tableName,
				 filed, value, _detectorInfo->get_id());
	alarm_machine_manager* mgr = alarm_machine_manager::GetInstance();
	if (!mgr->ExecuteSql(query)) {
		ASSERT(0); JLOG(L"update detector info failed.\n"); return false;
	}
	return true;
}






};


