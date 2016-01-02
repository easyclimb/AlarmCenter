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
	CWndWeakPtr _wnd;
	OnInversionControlMapCB _cb;
	bool _alarming;
	CDetectorInfoList _noZoneDetectorList;
	CDetectorBindInterfacePtr _activeInterface;
public:
	CMapInfo();
	~CMapInfo();
	void AddInterface(CDetectorBindInterfacePtr pInterface) { _interfaceList.push_back(pInterface); }
	void RemoveInterface(CDetectorBindInterfacePtr pInterface) { _interfaceList.remove(pInterface); }
	void GetAllInterfaceInfo(std::list<CDetectorBindInterfacePtr>& list);
	CDetectorBindInterfacePtr GetActiveInterfaceInfo() { return _activeInterface; }
	void SetActiveInterfaceInfo(CDetectorBindInterfacePtr pInterface) { _activeInterface = pInterface; }

	DECLARE_GETTER_SETTER_INT(_id);
	void set_type(int type) { _type = Integer2MapType(type); }
	MapType get_type() const { return _type; }

	// 2015年3月20日 16:33:54 保存已经与地图绑定，但未与防区绑定的探头
	void AddNoZoneDetectorInfo(CDetectorInfoPtr detInfo) { _noZoneDetectorList.push_back(detInfo); }
	void GetNoZoneDetectorInfo(CDetectorInfoList& list);
	void RemoveNoZoneDetectorInfo(CDetectorInfoPtr detInfo) { _noZoneDetectorList.remove(detInfo); }

	bool execute_delete_no_zone_detector_info(CDetectorInfoPtr detInfo);

	DECLARE_GETTER_SETTER_INT(_machine_id);
	DECLARE_GETTER_SETTER_STRING(_alias);
	DECLARE_GETTER_SETTER_STRING(_path);
	DECLARE_GETTER(bool, _alarming);

	void SetInversionControlCallBack(CWndWeakPtr wnd, OnInversionControlMapCB cb);
	//void AddNewAlarmText(AlarmText* at);
	void TraverseAlarmText(CWndWeakPtr wnd, OnInversionControlMapCB cb);

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
