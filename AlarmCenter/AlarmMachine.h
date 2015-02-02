#pragma once
#include <list>
#include <algorithm>

namespace core {

typedef void(_stdcall *AdemcoEventCB)(void* udata, int zone, int ademco_event);

class CMapInfo;
class CAlarmMachine
{
	DECLARE_UNCOPYABLE(CAlarmMachine)
	typedef struct AdemcoEventCallbackInfo
	{
		DECLARE_UNCOPYABLE(AdemcoEventCallbackInfo)
		AdemcoEventCallbackInfo() {}
	public:
		AdemcoEventCB _on_result;
		void* _udata;
		AdemcoEventCallbackInfo(AdemcoEventCB on_result, void* udata)
			: _on_result(on_result), _udata(udata) {}
	}MachineStatusCallbackInfo;
private:
	int _id;
	int _ademco_id;
	char _device_id[64];
	wchar_t _device_idW[64];
	wchar_t* _alias;
	wchar_t* _contact;
	wchar_t* _address;
	wchar_t* _phone;
	wchar_t* _phone_bk;
	int _ademco_zone;
	int _ademco_event;
	bool _online;
	//AdemcoEventCB _statusCb;
	//void* _udata;
	std::list<CMapInfo*> _mapList;
	std::list<MachineStatusCallbackInfo*> _observerList;
public:
	CAlarmMachine();
	~CAlarmMachine();
	bool IsOnline() const { return _online; }

	void AddMap(CMapInfo* map) { _mapList.push_back(map); }
	CMapInfo* GetFirstMap() const;
	bool HasMap() const { return _mapList.size() > 0; }

	//bool operator > (const CAlarmMachine* machine) { return _ademco_id > machine->_ademco_id; }
	//bool operator < (const CAlarmMachine* machine) { return _ademco_id < machine->_ademco_id; }
	//bool operator == (const CAlarmMachine* machine) { return _ademco_id == machine->_ademco_id; }
	
	void RegisterObserver(void* udata, AdemcoEventCB cb);
	void UnregisterObserver(void* udata);
	void NotifyObservers();

	void SetAdemcoEvent(int zone, int status);
	int GetStatus() const { return _ademco_event; }

	const char* GetDeviceIDA() const { return _device_id; }
	const wchar_t* GetDeviceIDW() const { return _device_idW; }

	void set_device_id(const wchar_t* device_id);
	void set_device_id(const char* device_id);

	DEALARE_GETTER_SETTER_INT(_id);
	DEALARE_GETTER_SETTER_INT(_ademco_id);

	DECLARE_GETTER_SETTER_STRING(_alias);
	DECLARE_GETTER_SETTER_STRING(_contact);
	DECLARE_GETTER_SETTER_STRING(_address);
	DECLARE_GETTER_SETTER_STRING(_phone);
	DECLARE_GETTER_SETTER_STRING(_phone_bk);

	
};

NAMESPACE_END
