#pragma once
#include <list>

namespace core {

	typedef struct AlarmText {
		int _zone;
		int _subzone;
		int _event;
		std::string _txt;
		AlarmText() : _zone(0), _subzone(0), _event(0), _txt() {}
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
	//std::list<CZoneInfo*>::iterator _curZoneListIter;
public:
	CMapInfo();
	~CMapInfo();
	void AddZone(CZoneInfo* zone) { _zoneList.push_back(zone); }
	//CZoneInfo* GetFirstZoneInfo();
	//CZoneInfo* GetNextZoneInfo();
	CZoneInfo* GetZoneInfo(int zone);
	void GetAllZoneInfo(std::list<CZoneInfo*>& list);

	DEALARE_GETTER_SETTER_INT(_id);
	//DEALARE_GETTER_SETTER_INT(_type);
	void set_type(int type) { _type = Integer2MapType(type); }
	MapType get_type() const { return _type; }

	DEALARE_GETTER_SETTER_INT(_machine_id);
	DECLARE_GETTER_SETTER_STRING(_alias);
	DECLARE_GETTER_SETTER_STRING(_path);

	
protected:
	static MapType Integer2MapType(int type) {
		if (type == MAP_SUB_MACHINE) { return MAP_SUB_MACHINE; }
		else { return MAP_MACHINE; }
	}
	
	DECLARE_OBSERVER(OnNewAlarmTextCB, AlarmText*);
	DECLARE_UNCOPYABLE(CMapInfo)
};

NAMESPACE_END
