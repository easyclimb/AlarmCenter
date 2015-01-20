#pragma once
#include <list>

namespace core {

typedef enum MachineStatus
{
	MS_OFFLINE,
	MS_ONLINE,
	MS_DISARM,
	MS_ARM,
}MachineStatus;

typedef void(_stdcall *MachineStatusCB)(void* udata, MachineStatus status);

class CAlarmMachine
{
	DECLARE_UNCOPYABLE(CAlarmMachine)
private:
	int _id;
	int _ademco_id;
	char _device_id[64];
	wchar_t _device_idW[64];
	wchar_t* _alias;
	MachineStatus _status;
	MachineStatusCB _statusCb;
public:
	CAlarmMachine();
	~CAlarmMachine();

	bool operator > (const CAlarmMachine* machine) { return _ademco_id > machine->_ademco_id; }
	bool operator < (const CAlarmMachine* machine) { return _ademco_id < machine->_ademco_id; }
	//bool operator == (const CAlarmMachine* machine) { return _ademco_id == machine->_ademco_id; }
	
	void SetMachineStatusCb(MachineStatusCB cb) { _statusCb = cb; }

	int GetID() const { return _id;	}

	void SetID(int id) { _id = id; }

	int GetAdemcoID() const { return _ademco_id; }

	void SetAdemcoID(int ademco_id) { _ademco_id = ademco_id; }

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
		if (NULL == alias) return;
		int len = wcslen(alias);
		if (len > 0) {
			if (_alias) delete _alias;
			_alias = new wchar_t[len + 1];
			wcscpy_s(_alias, len + 1, alias);
		}
	}
};

typedef std::list<CAlarmMachine*> CAlarmMachineList;
typedef std::list<CAlarmMachine*>::iterator CAlarmMachineListIter;

NAMESPACE_END
