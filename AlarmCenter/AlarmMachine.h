#pragma once
#include <list>
#include <algorithm>


namespace core {
	
class CZoneInfo;

typedef void(_stdcall *TraverseZoneOfMapCB)(void* udata, CZoneInfo* zone);

using namespace ademco;
class CMapInfo;
class CAlarmMachine
{
private:
	int _id;
	int _ademco_id;
	int _group_id;
	bool _banned;
	char _device_id[64];
	wchar_t _device_idW[64];
	wchar_t* _alias;
	wchar_t* _contact;
	wchar_t* _address;
	wchar_t* _phone;
	wchar_t* _phone_bk;
	bool _online;
	bool _armed;
	CMapInfo* _noZoneMap;
	std::list<CMapInfo*> _mapList;
	std::list<CMapInfo*>::iterator _curMapListIter;
	std::list<ademco::AdemcoEvent*> _ademcoEventList;
	CLock _lock4AdemcoEventList;
protected:
	CZoneInfo* GetZoneInfo(int zone_id);
public:
	CAlarmMachine();
	~CAlarmMachine();
	bool IsOnline() const { return _online; }
	bool IsArmed() const { return _armed; }
	void clear_ademco_event_list();

	void AddMap(CMapInfo* map) { _mapList.push_back(map); }
	CMapInfo* GetFirstMap();
	CMapInfo* GetNextMap();
	void SetNoZoneMap(CMapInfo* map) { _noZoneMap = map; }
	CMapInfo* GetNoZoneMap() { return _noZoneMap; }
	
	void SetAdemcoEvent(int zone, int ademco_event, const time_t& event_time);
	void TraverseAdmecoEventList(void* udata, ademco::AdemcoEventCB cb);

	const char* GetDeviceIDA() const { return _device_id; }
	const wchar_t* GetDeviceIDW() const { return _device_idW; }

	void set_device_id(const wchar_t* device_id);
	void set_device_id(const char* device_id);

	DEALARE_GETTER_SETTER_INT(_id);
	DEALARE_GETTER_SETTER_INT(_ademco_id); 
	DEALARE_GETTER_SETTER_INT(_group_id);
	DEALARE_GETTER_SETTER(bool, _banned);

	DECLARE_GETTER_SETTER_STRING(_alias);
	DECLARE_GETTER_SETTER_STRING(_contact);
	DECLARE_GETTER_SETTER_STRING(_address);
	DECLARE_GETTER_SETTER_STRING(_phone);
	DECLARE_GETTER_SETTER_STRING(_phone_bk);

	DECLARE_OBSERVER(AdemcoEventCB, AdemcoEvent*)
	DECLARE_UNCOPYABLE(CAlarmMachine)
};

NAMESPACE_END
