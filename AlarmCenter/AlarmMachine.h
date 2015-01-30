#pragma once
#include <list>
#include <algorithm>

namespace core {

typedef void(_stdcall *MachineStatusCB)(void* udata, MachineStatus status);

class CMapInfo;
class CAlarmMachine
{
	DECLARE_UNCOPYABLE(CAlarmMachine)
	typedef struct MachineStatusCallbackInfo
	{
		DECLARE_UNCOPYABLE(MachineStatusCallbackInfo)
		MachineStatusCallbackInfo() {}
	public:
		MachineStatusCB _on_result;
		void* _udata;
		MachineStatusCallbackInfo(MachineStatusCB on_result, void* udata) 
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
	MachineStatus _status;
	bool _online;
	//MachineStatusCB _statusCb;
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
	
	void RegisterObserver(void* udata, MachineStatusCB cb);
	void UnregisterObserver(void* udata);
	void NotifyObservers();

	DEALARE_GETTER_SETTER_INT(_id);
	DEALARE_GETTER_SETTER_INT(_ademco_id);

	const char* GetDeviceIDA() const { return _device_id; }
	const wchar_t* GetDeviceIDW() const { return _device_idW; }

	void set_device_id(const wchar_t* device_id);
	void set_device_id(const char* device_id);

	DECLARE_GETTER_SETTER_STRING(_alias)
	DECLARE_GETTER_SETTER_STRING(_contact)
	DECLARE_GETTER_SETTER_STRING(_address)
	DECLARE_GETTER_SETTER_STRING(_phone)
	DECLARE_GETTER_SETTER_STRING(_phone_bk)

	void SetStatus(MachineStatus status);
	MachineStatus GetStatus() const { return _status; }
};

NAMESPACE_END
