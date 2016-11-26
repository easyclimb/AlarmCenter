#include "stdafx.h"
#include "MapInfo.h"
#include "DetectorInfo.h"
#include "AlarmMachineManager.h"

#include <iterator>

namespace core {

map_info::map_info()
	: /*__class_name("class name is map_info")
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


map_info::~map_info()
{
	clear_alarm_text_list();
	//_noZoneDetectorList.clear();
}


void map_info::GetAllInterfaceInfo(std::list<detector_bind_interface_ptr>& list)
{
	std::copy(_interfaceList.begin(), _interfaceList.end(), std::back_inserter(list));
}

//
//void map_info::SetInversionControlCallBack(const CMapViewWeakPtr& wnd, OnInversionControlMapCB cb)
//{ 
//	//_signal.connect(std::bind(wnd, cb));
//	//_wnd = wnd; _cb = cb;
//}
//

void map_info::InversionControl(inversion_control_map_command icmc, const alarm_text_ptr& at)
{
	AUTO_LOG_FUNCTION;
	if ((ICMC_ADD_ALARM_TEXT == icmc) && at) {
		std::lock_guard<std::mutex> lock(_lock4AlarmTextList);
		ademco::EventLevel level = ademco::GetEventLevel(at->_event);
		if (ademco::EVENT_LEVEL_EXCEPTION_RESUME == level) {
			auto exception_event = ademco::GetExceptionEventByResumeEvent(at->_event);
			auto iter = _alarmTextList.begin();
			while (iter != _alarmTextList.end()) {
				alarm_text_ptr& old = *iter;
				if (exception_event == old->_event) {
					//if (_cb && !_wnd.expired()) { _cb(_wnd.lock(), std::make_shared<icmc_buffer>(ICMC_DEL_ALARM_TEXT, old)); }
					notify_observers(std::make_shared<icmc_buffer>(ICMC_DEL_ALARM_TEXT, old));
					_alarmTextList.erase(iter);
					_alarming = _alarmTextList.size() > 0;
					return;
				}
				iter++;
			}
		}
		_alarming = true;
		//if (_cb && !_wnd.expired()) { _cb(_wnd.lock(), std::make_shared<icmc_buffer>(ICMC_ADD_ALARM_TEXT, at)); }
		notify_observers(std::make_shared<icmc_buffer>(ICMC_ADD_ALARM_TEXT, at));
		_alarmTextList.push_back(at);
	} else if(ICMC_CLR_ALARM_TEXT == icmc){
		_alarming = false;
		//if (_cb && !_wnd.expired()) { _cb(_wnd.lock(), std::make_shared<icmc_buffer>(ICMC_CLR_ALARM_TEXT, nullptr)); }
		notify_observers(std::make_shared<icmc_buffer>(ICMC_CLR_ALARM_TEXT, at));
		clear_alarm_text_list();
	} else {
		//if (_cb && !_wnd.expired()) { _cb(_wnd.lock(), std::make_shared<icmc_buffer>(icmc, nullptr)); }
		notify_observers(std::make_shared<icmc_buffer>(icmc, at));
	}
}

//
//void map_info::TraverseAlarmText(const CMapViewWeakPtr& wnd, OnInversionControlMapCB cb)
//{
//	_lock4AlarmTextList.Lock();
//	for (auto at : _alarmTextList) {
//		cb(wnd.lock(), std::make_shared<icmc_buffer>(ICMC_ADD_ALARM_TEXT, at));
//	}
//	_lock4AlarmTextList.UnLock();
//}

void map_info::TraverseAlarmText(const observer_ptr& obj)
{
	std::lock_guard<std::mutex> lock(_lock4AlarmTextList);
	std::shared_ptr<observer_type> obs(obj.lock());
	if (obs) {
		for (auto at : _alarmTextList) {
			obs->on_update(std::make_shared<icmc_buffer>(ICMC_ADD_ALARM_TEXT, at));
		}
	}
}


void map_info::clear_alarm_text_list()
{
	std::lock_guard<std::mutex> lock(_lock4AlarmTextList);
	_alarmTextList.clear();
}


};

