#pragma once
#include <list>

namespace core {

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

	DECLARE_UNCOPYABLE(CMapInfo)
protected:
	static MapType Integer2MapType(int type) {
		if (type == MAP_SUB_MACHINE) { return MAP_SUB_MACHINE; }
		else { return MAP_MACHINE; }
	}
};

NAMESPACE_END
