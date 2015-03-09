#pragma once

namespace core {

// max sub-zone of sub-machine
static const int MAX_SUB_MACHINE_ZONE = 100;

class CZoneInfo;

class CSubMachineInfo
{
private:
	int _id;
	int _ademco_id;
	int _zone_id;
	wchar_t* _alias;
	wchar_t* _contact;
	wchar_t* _address;
	wchar_t* _phone;
	wchar_t* _phone_bk;
	bool _armed;
	CZoneInfo* _sub_zones[MAX_SUB_MACHINE_ZONE];
public:
	CSubMachineInfo();
	~CSubMachineInfo();

	// 2015年3月4日 15:44:22 分防区操作
	void AddSubZone(CZoneInfo* zoneInfo);
	CZoneInfo* GetSubZone(int sub_zone);

	DEALARE_GETTER_SETTER_INT(_id);
	DEALARE_GETTER_SETTER_INT(_ademco_id);
	DEALARE_GETTER_SETTER_INT(_zone_id);
	DECLARE_GETTER_SETTER_STRING(_alias);
	DECLARE_GETTER_SETTER_STRING(_contact);
	DECLARE_GETTER_SETTER_STRING(_address);
	DECLARE_GETTER_SETTER_STRING(_phone);
	DECLARE_GETTER_SETTER_STRING(_phone_bk);
	DEALARE_GETTER_SETTER(bool, _armed);

	DECLARE_UNCOPYABLE(CSubMachineInfo);
};



NAMESPACE_END;

