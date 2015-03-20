#pragma once
#include <list>

namespace core {

typedef struct AlarmText {
	int _zone;
	int _subzone;
	int _event;
	CString _txt;
	AlarmText() : _zone(0), _subzone(0), _event(0), _txt(_T("")) {}
	AlarmText& operator=(const AlarmText& rhs) {
		_zone = rhs._zone;
		_subzone = rhs._subzone;
		_event = rhs._event;
		_txt = rhs._txt;
		return *this;
	}
}AlarmText;

// 反向控制地图实体命令
enum InversionControlCommand {
	ICC_SHOW,			// 显示地图
	ICC_ADD_ALARM_TEXT, // 添加报警文字并显示(需附加参数AlarmText)
	ICC_CLR_ALARM_TEXT, // 清除报警文字
	ICC_RENAME,			// 重命名
	ICC_CHANGE_IMAGE,	// 更换图片
	ICC_DESTROY,		// 释放对自己的引用
};

typedef void(__stdcall *OnInversionControlCB)(void* udata, 
											  InversionControlCommand icc,
											  const AlarmText* at);

enum MapType {
	MAP_MACHINE,
	MAP_SUB_MACHINE,
};

class CZoneInfo;
class CDetectorInfo;
typedef std::list<CDetectorInfo*> CDetectorInfoList;
typedef std::list<CDetectorInfo*>::iterator CDetectorInfoListIter;

class CMapInfo
{
	const char *__class_name;
private:
	int _id;
	MapType _type;
	int _machine_id;
	wchar_t* _alias;
	wchar_t* _path;
	std::list<CZoneInfo*> _zoneList;
	std::list<AlarmText*> _alarmTextList;
	CLock _lock4AlarmTextList;
	void* _udata;
	OnInversionControlCB _cb;
	bool _alarming;
	CDetectorInfoList _noZoneDetectorList;
public:
	CMapInfo();
	~CMapInfo();
	void AddZone(CZoneInfo* zone) { _zoneList.push_back(zone); }
	void RemoveZone(CZoneInfo* zone) { _zoneList.remove(zone); }
	void GetAllZoneInfo(std::list<CZoneInfo*>& list);

	DECLARE_GETTER_SETTER_INT(_id);
	void set_type(int type) { _type = Integer2MapType(type); }
	MapType get_type() const { return _type; }

	// 2015年3月20日 16:33:54 保存已经与地图绑定，但未与防区绑定的探头
	void AddNoZoneDetectorInfo(CDetectorInfo* detInfo) { _noZoneDetectorList.push_back(detInfo); }
	void GetNoZoneDetectorInfo(CDetectorInfoList& list);

	DECLARE_GETTER_SETTER_INT(_machine_id);
	DECLARE_GETTER_SETTER_STRING(_alias);
	DECLARE_GETTER_SETTER_STRING(_path);
	DECLARE_GETTER(bool, _alarming);

	void SetInversionControlCallBack(void* udata, OnInversionControlCB cb);
	//void AddNewAlarmText(AlarmText* at);
	void TraverseAlarmText(void* udata, OnInversionControlCB cb);

	// 2015年3月20日 17:20:03 增加反向控制mapView实体的命令
	void InversionControl(InversionControlCommand icc, AlarmText* at = NULL);

protected:
	static MapType Integer2MapType(int type) {
		if (type == MAP_SUB_MACHINE) { return MAP_SUB_MACHINE; }
		else { return MAP_MACHINE; }
	}

	void clear_alarm_text_list();
	
	DECLARE_UNCOPYABLE(CMapInfo)
};

NAMESPACE_END
