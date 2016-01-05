#include "stdafx.h"
#include "MapInfo.h"
#include "DetectorInfo.h"
#include "AlarmMachineManager.h"

#include <iterator>

namespace core {

CMapInfo::CMapInfo()
	: /*__class_name("class name is CMapInfo")
	, */_id(0)
	, _type(MAP_MACHINE)
	, _machine_id(0)
	, _path()
	//, _wnd()
	//, _cb(nullptr)
	, _alarming(false)
	, _activeInterface(nullptr)
{
}


CMapInfo::~CMapInfo()
{
	clear_alarm_text_list();
	//_noZoneDetectorList.clear();
}


void CMapInfo::GetAllInterfaceInfo(std::list<CDetectorBindInterfacePtr>& list)
{
	std::copy(_interfaceList.begin(), _interfaceList.end(), std::back_inserter(list));
}

//
//void CMapInfo::SetInversionControlCallBack(const CMapViewWeakPtr& wnd, OnInversionControlMapCB cb)
//{ 
//	//_signal.connect(std::bind(wnd, cb));
//	//_wnd = wnd; _cb = cb;
//}
//

void CMapInfo::InversionControl(InversionControlMapCommand icmc, const AlarmTextPtr& at)
{
	AUTO_LOG_FUNCTION;
	if ((ICMC_ADD_ALARM_TEXT == icmc) && at) {
		_lock4AlarmTextList.Lock();
		ademco::EventLevel level = ademco::GetEventLevel(at->_event);
		if (ademco::EVENT_LEVEL_EXCEPTION_RESUME == level) {
			ADEMCO_EVENT exception_event = ademco::GetExceptionEventByResumeEvent(at->_event);
			auto iter = _alarmTextList.begin();
			while (iter != _alarmTextList.end()) {
				AlarmTextPtr& old = *iter;
				if (exception_event == old->_event) {
					//if (_cb && !_wnd.expired()) { _cb(_wnd.lock(), std::make_shared<IcmcBuffer>(ICMC_DEL_ALARM_TEXT, old)); }
					notify_observers(std::make_shared<IcmcBuffer>(ICMC_DEL_ALARM_TEXT, old));
					_alarmTextList.erase(iter);
					_alarming = _alarmTextList.size() > 0;
					_lock4AlarmTextList.UnLock();
					return;
				}
				iter++;
			}
		}
		_alarming = true;
		//if (_cb && !_wnd.expired()) { _cb(_wnd.lock(), std::make_shared<IcmcBuffer>(ICMC_ADD_ALARM_TEXT, at)); }
		notify_observers(std::make_shared<IcmcBuffer>(ICMC_ADD_ALARM_TEXT, at));
		_alarmTextList.push_back(at);
		_lock4AlarmTextList.UnLock();
	} else if(ICMC_CLR_ALARM_TEXT == icmc){
		_alarming = false;
		//if (_cb && !_wnd.expired()) { _cb(_wnd.lock(), std::make_shared<IcmcBuffer>(ICMC_CLR_ALARM_TEXT, nullptr)); }
		notify_observers(std::make_shared<IcmcBuffer>(ICMC_CLR_ALARM_TEXT, at));
		clear_alarm_text_list();
	} else {
		//if (_cb && !_wnd.expired()) { _cb(_wnd.lock(), std::make_shared<IcmcBuffer>(icmc, nullptr)); }
		notify_observers(std::make_shared<IcmcBuffer>(icmc, at));
	}
}

//
//void CMapInfo::TraverseAlarmText(const CMapViewWeakPtr& wnd, OnInversionControlMapCB cb)
//{
//	_lock4AlarmTextList.Lock();
//	for (auto at : _alarmTextList) {
//		cb(wnd.lock(), std::make_shared<IcmcBuffer>(ICMC_ADD_ALARM_TEXT, at));
//	}
//	_lock4AlarmTextList.UnLock();
//}

void CMapInfo::TraverseAlarmText(const observer_ptr& obj)
{
	_lock4AlarmTextList.Lock();
	std::shared_ptr<observer_type> obs(obj.lock());
	if (obs) {
		for (auto at : _alarmTextList) {
			obs->on_update(std::make_shared<IcmcBuffer>(ICMC_ADD_ALARM_TEXT, at));
		}
	}
	_lock4AlarmTextList.UnLock();
}


void CMapInfo::clear_alarm_text_list()
{
	_lock4AlarmTextList.Lock();
	_alarmTextList.clear();
	_lock4AlarmTextList.UnLock();
}


NAMESPACE_END
