#pragma once
#include <list>
#include "core.h"
//#include "signal_slot.h"

namespace core {


enum map_type {
	MAP_MACHINE,
	MAP_SUB_MACHINE,
};



class map_info : public dp::observable<icmc_buffer_ptr>
{
	//const char *__class_name;
private:
	int _id;
	map_type _type;
	int _machine_id;
	CString _alias;
	CString _path;
	std::list<detector_bind_interface_ptr> _interfaceList;
	std::list<alarm_text_ptr> _alarmTextList;
	std::mutex _lock4AlarmTextList;
	//CMapViewWeakPtr _wnd;
	//OnInversionControlMapCB _cb;
	bool _alarming;
	//detector_info_list _noZoneDetectorList;
	detector_bind_interface_ptr _activeInterface;

	//util::signal<void(const CMapViewWeakPtr& wnd, const core::icmc_buffer_ptr&)> _signal;
public:
	map_info();
	~map_info();
	void AddInterface(const detector_bind_interface_ptr& pInterface) { _interfaceList.push_back(pInterface); }
	void RemoveInterface(const detector_bind_interface_ptr& pInterface) { _interfaceList.remove(pInterface); }
	void GetAllInterfaceInfo(std::list<detector_bind_interface_ptr>& list);
	detector_bind_interface_ptr GetActiveInterfaceInfo() { return _activeInterface; }
	void SetActiveInterfaceInfo(const detector_bind_interface_ptr& pInterface) { _activeInterface = pInterface; }

	DECLARE_GETTER_SETTER_INT(_id);
	void set_type(int type) { _type = Integer2MapType(type); }
	map_type get_type() const { return _type; }

	DECLARE_GETTER_SETTER_INT(_machine_id);
	DECLARE_GETTER_SETTER_STRING(_alias);
	DECLARE_GETTER_SETTER_STRING(_path);
	DECLARE_GETTER(bool, _alarming);

	//void SetInversionControlCallBack(const CMapViewWeakPtr& wnd, OnInversionControlMapCB cb);
	//void TraverseAlarmText(const CMapViewWeakPtr& wnd, OnInversionControlMapCB cb);
	void TraverseAlarmText(const observer_ptr& obj);

	// 2015年3月20日 17:20:03 增加反向控制mapView实体的命令
	void InversionControl(inversion_control_map_command icmc, const alarm_text_ptr& at = nullptr);

protected:
	static map_type Integer2MapType(int type) {
		if (type == MAP_SUB_MACHINE) { return MAP_SUB_MACHINE; }
		else { return MAP_MACHINE; }
	}

	void clear_alarm_text_list();
	
};
};

