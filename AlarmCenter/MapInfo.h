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

	typedef void(__stdcall *OnNewAlarmTextCB)(void* udata, const AlarmText* at);


enum MapType {
	MAP_MACHINE,
	MAP_SUB_MACHINE,
};

class CZoneInfo;
class CMapInfo
{
private:
	int _id;
	MapType _type;
	int _machine_id;
	wchar_t* _alias;
	wchar_t* _path;
	std::list<CZoneInfo*> _zoneList;
	std::list<AlarmText*> _alarmTextList;
	CLock _lock4AlarmTextList;
	//std::list<CZoneInfo*>::iterator _curZoneListIter;
	void* _udata;
	OnNewAlarmTextCB _cb;
	bool _alarming;
public:
	CMapInfo();
	~CMapInfo();
	void AddZone(CZoneInfo* zone) { _zoneList.push_back(zone); }
	//CZoneInfo* GetFirstZoneInfo();
	//CZoneInfo* GetNextZoneInfo();
	CZoneInfo* GetZoneInfo(int zone);
	void GetAllZoneInfo(std::list<CZoneInfo*>& list);

	DECLARE_GETTER_SETTER_INT(_id);
	//DECLARE_GETTER_SETTER_INT(_type);
	void set_type(int type) { _type = Integer2MapType(type); }
	MapType get_type() const { return _type; }

	DECLARE_GETTER_SETTER_INT(_machine_id);
	DECLARE_GETTER_SETTER_STRING(_alias);
	DECLARE_GETTER_SETTER_STRING(_path);
	DECLARE_GETTER(bool, _alarming);
	void SetNewAlarmTextCallBack(void* udata, OnNewAlarmTextCB cb) { _udata = udata; _cb = cb;	}

	void AddNewAlarmText(AlarmText* at) { 
		if (at) {
			_alarming = true;
			if (_cb) { _cb(_udata, at); delete at; } 
			else {
				_lock4AlarmTextList.Lock();
				_alarmTextList.push_back(at); 
				_lock4AlarmTextList.UnLock();
			}
		} else {
			_alarming = false;
			if (_cb) { _cb(_udata, at); } 
			clear_alarm_text_list();
		}
	}

	void TraverseAlarmText(void* udata, OnNewAlarmTextCB cb) {
		_lock4AlarmTextList.Lock();
		std::list<AlarmText*>::iterator iter = _alarmTextList.begin();
		while (iter != _alarmTextList.end()) {
			AlarmText* at = *iter++; cb(udata, at); delete at; }
		_alarmTextList.clear();
		_lock4AlarmTextList.UnLock();
	}

protected:
	static MapType Integer2MapType(int type) {
		if (type == MAP_SUB_MACHINE) { return MAP_SUB_MACHINE; }
		else { return MAP_MACHINE; }
	}

	void clear_alarm_text_list() {
		_lock4AlarmTextList.Lock();
		std::list<AlarmText*>::iterator iter = _alarmTextList.begin();
		while (iter != _alarmTextList.end()) {
			AlarmText* at = *iter++;	delete at;
		}
		_alarmTextList.clear();
		_lock4AlarmTextList.UnLock();
	}
	
	DECLARE_UNCOPYABLE(CMapInfo)
};

NAMESPACE_END
