#pragma once
#include <list>
#include "core.h"

namespace core {


enum MapType {
	MAP_MACHINE,
	MAP_SUB_MACHINE,
};



class CMapInfo
{
	//const char *__class_name;
private:
	int _id;
	MapType _type;
	int _machine_id;
	CString _alias;
	CString _path;
	std::list<CDetectorBindInterfacePtr> _interfaceList;
	std::list<AlarmTextPtr> _alarmTextList;
	CLock _lock4AlarmTextList;
	CMapViewWeakPtr _wnd;
	OnInversionControlMapCB _cb;
	bool _alarming;
	//CDetectorInfoList _noZoneDetectorList;
	CDetectorBindInterfacePtr _activeInterface;
public:
	CMapInfo();
	~CMapInfo();
	void AddInterface(const CDetectorBindInterfacePtr& pInterface) { _interfaceList.push_back(pInterface); }
	void RemoveInterface(const CDetectorBindInterfacePtr& pInterface) { _interfaceList.remove(pInterface); }
	void GetAllInterfaceInfo(std::list<CDetectorBindInterfacePtr>& list);
	CDetectorBindInterfacePtr GetActiveInterfaceInfo() { return _activeInterface; }
	void SetActiveInterfaceInfo(const CDetectorBindInterfacePtr& pInterface) { _activeInterface = pInterface; }

	DECLARE_GETTER_SETTER_INT(_id);
	void set_type(int type) { _type = Integer2MapType(type); }
	MapType get_type() const { return _type; }

	DECLARE_GETTER_SETTER_INT(_machine_id);
	DECLARE_GETTER_SETTER_STRING(_alias);
	DECLARE_GETTER_SETTER_STRING(_path);
	DECLARE_GETTER(bool, _alarming);

	void SetInversionControlCallBack(const CMapViewWeakPtr& wnd, OnInversionControlMapCB cb);
	void TraverseAlarmText(const CMapViewWeakPtr& wnd, OnInversionControlMapCB cb);

	// 2015年3月20日 17:20:03 增加反向控制mapView实体的命令
	void InversionControl(InversionControlMapCommand icmc, AlarmTextPtr at = nullptr);

protected:
	static MapType Integer2MapType(int type) {
		if (type == MAP_SUB_MACHINE) { return MAP_SUB_MACHINE; }
		else { return MAP_MACHINE; }
	}

	void clear_alarm_text_list();
	
	DECLARE_UNCOPYABLE(CMapInfo)
};

NAMESPACE_END
