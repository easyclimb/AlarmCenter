#pragma once
#include <list>
#include <algorithm>


namespace core {
	
class CZoneInfo;

typedef void(_stdcall *TraverseZoneOfMapCB)(void* udata, CZoneInfo* zone);
//typedef struct TraverseZoneOfMapCallbackObj {
//	TraverseZoneOfMapCB on_result;
//	void* udata;
//}TraverseZoneOfMapCallbackObj;

class CMapInfo;
class CAlarmMachine
{
	DECLARE_UNCOPYABLE(CAlarmMachine)
	typedef struct AdemcoEventCallbackInfo
	{
		DECLARE_UNCOPYABLE(AdemcoEventCallbackInfo)
		AdemcoEventCallbackInfo() {}
	public:
		ademco::AdemcoEventCB _on_result;
		void* _udata;
		AdemcoEventCallbackInfo(ademco::AdemcoEventCB on_result, void* udata)
			: _on_result(on_result), _udata(udata)
		{}
	}AdemcoEventCallbackInfo;
private:
	int _id;
	int _ademco_id;
	bool _banned;
	char _device_id[64];
	wchar_t _device_idW[64];
	wchar_t* _alias;
	wchar_t* _contact;
	wchar_t* _address;
	wchar_t* _phone;
	wchar_t* _phone_bk;
	bool _online;
	std::list<CZoneInfo*> _zoneList;
	std::list<CMapInfo*> _mapList;
	std::list<CMapInfo*>::iterator _curMapListIter;
	std::list<ademco::AdemcoEvent*> _ademcoEventList;
	std::list<AdemcoEventCallbackInfo*> _observerList;
	CLock _lock4AdemcoEventList;
protected:
	void clear_ademco_event_list();
public:
	CAlarmMachine();
	~CAlarmMachine();
	bool IsOnline() const { return _online; }

	void AddZone(CZoneInfo* zone) { _zoneList.push_back(zone); }
	void TraverseZoneOfMap(int map_id, void* udata, TraverseZoneOfMapCB cb);

	void AddMap(CMapInfo* map) { _mapList.push_back(map); }
	CMapInfo* GetFirstMap();
	CMapInfo* GetNextMap();
	bool HasMap() const { return _mapList.size() > 0; }

	//bool operator > (const CAlarmMachine* machine) { return _ademco_id > machine->_ademco_id; }
	//bool operator < (const CAlarmMachine* machine) { return _ademco_id < machine->_ademco_id; }
	//bool operator == (const CAlarmMachine* machine) { return _ademco_id == machine->_ademco_id; }
	
	void RegisterObserver(void* udata, ademco::AdemcoEventCB cb);
	void UnregisterObserver(void* udata);
	void NotifyObservers(ademco::AdemcoEvent* ademcoEvent);

	void SetAdemcoEvent(int zone, int ademco_event, const time_t& event_time);
	//int GetStatus() const { return _ademco_event; }
	void TraverseAdmecoEventList(void* udata, ademco::AdemcoEventCB cb);

	const char* GetDeviceIDA() const { return _device_id; }
	const wchar_t* GetDeviceIDW() const { return _device_idW; }

	void set_device_id(const wchar_t* device_id);
	void set_device_id(const char* device_id);

	DEALARE_GETTER_SETTER_INT(_id);
	DEALARE_GETTER_SETTER_INT(_ademco_id);
	DEALARE_GETTER_SETTER(bool, _banned);

	DECLARE_GETTER_SETTER_STRING(_alias);
	DECLARE_GETTER_SETTER_STRING(_contact);
	DECLARE_GETTER_SETTER_STRING(_address);
	DECLARE_GETTER_SETTER_STRING(_phone);
	DECLARE_GETTER_SETTER_STRING(_phone_bk);

	
};

NAMESPACE_END
