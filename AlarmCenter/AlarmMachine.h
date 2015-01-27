#pragma once
#include <list>
#include <algorithm>

namespace core {

typedef void(_stdcall *MachineStatusCB)(void* udata, MachineStatus status);

class CMapInfo;
class CZoneInfo;

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
	MachineStatus _status;
	//MachineStatusCB _statusCb;
	//void* _udata;
	std::list<CMapInfo*> _mapList;
	std::list<CZoneInfo*> _zoneList;
	std::list<MachineStatusCallbackInfo*> _observerList;
public:
	CAlarmMachine();
	~CAlarmMachine();

	void AddMap(CMapInfo* map) { _mapList.push_back(map); }
	void AddZone(CZoneInfo* zone) { _zoneList.push_back(zone); }

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

	void SetDeviceID(const wchar_t* device_id) {
		wcscpy_s(_device_idW, device_id);
		USES_CONVERSION;
		strcpy_s(_device_id, W2A(_device_idW));
	}

	void SetDeviceID(const char* device_id)	{
		strcpy_s(_device_id, device_id);
		USES_CONVERSION;
		wcscpy_s(_device_idW, A2W(device_id));
	}

	const wchar_t* GetAlias() const { return _alias; }

	void SetAlias(const wchar_t* alias) { 
		if (alias) {
			int len = wcslen(alias);
			if (_alias) { delete[] _alias; }
			_alias = new wchar_t[len + 1];
			wcscpy_s(_alias, len + 1, alias);
		} else {
			if (_alias) { delete[] _alias; }
			_alias = new wchar_t[1];
			_alias[0] = 0;
		}
	}

	void SetStatus(MachineStatus status) {
		if (_status != status) {
			_status = status;
			NotifyObservers();
		}
	}
};

typedef std::list<CAlarmMachine*> CAlarmMachineList;
typedef std::list<CAlarmMachine*>::iterator CAlarmMachineListIter;

NAMESPACE_END
