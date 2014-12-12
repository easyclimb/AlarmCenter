#pragma once
class CAlarmMachine
{
private:
	int _id;
	int _ademco_id;
	char _device_id[64];
	CString _device_idW;

public:
	CAlarmMachine();
	~CAlarmMachine();

	int GetID() const
	{
		return _id;
	}

	void SetID(int id)
	{
		_id = id;
	}

	int GetAdemcoID() const
	{
		return _ademco_id;
	}

	void SetAdemcoID(int ademco_id)
	{
		_ademco_id = ademco_id;
	}

	const char* GetDeviceIDA() const
	{
		return _device_id;
	}

	const CString& GetDeviceIDW() const
	{
		return _device_idW;
	}

	void SetDeviceID(const CString& device_id)
	{
		_device_idW = device_id;
		USES_CONVERSION;
		strcpy_s(_device_id, W2A(_device_idW));
	}

	void SetDeviceID(const char* device_id)
	{
		strcpy_s(_device_id, device_id);
		USES_CONVERSION;
		_device_idW = A2W(device_id);
	}
};

