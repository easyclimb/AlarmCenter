#pragma once
#include <list>

namespace core {

class CZoneInfo;
class CMapInfo
{
	DECLARE_UNCOPYABLE(CMapInfo)
private:
	int _id;
	int _type;
	int _ademco_id;
	wchar_t* _path;
	std::list<CZoneInfo*> _zoneList;
	std::list<CZoneInfo*>::iterator _curZoneListIter;
public:
	CMapInfo();
	~CMapInfo();
	void AddZone(CZoneInfo* zone) { _zoneList.push_back(zone); }
	CZoneInfo* GetFirstZoneInfo();
	CZoneInfo* GetNextZoneInfo();

	DEALARE_GETTER_SETTER_INT(_id);
	DEALARE_GETTER_SETTER_INT(_type);
	DEALARE_GETTER_SETTER_INT(_ademco_id);

	DECLARE_GETTER_SETTER_STRING(_path);
};

NAMESPACE_END
