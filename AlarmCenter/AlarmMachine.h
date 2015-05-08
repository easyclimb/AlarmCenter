#pragma once
#include <list>
#include "core.h"

namespace core {

static const int MAX_MACHINE_ZONE = 1000;

typedef enum MachineType {
	MT_UNKNOWN = 0,
	MT_WIFI,		// wifi����
	MT_NETMOD,		// ������ģ��Ĺ�������
	MT_GPRS,		// gprs����
	MT_MAX,
}MachineType;

static MachineType Integer2MachineType(int type)
{
	switch (type) {
		case MT_WIFI:	return MT_WIFI;		break;
		case MT_NETMOD:	return MT_NETMOD;	break;
		case MT_GPRS: 	return MT_GPRS;		break;
		default:		return MT_UNKNOWN;	break;
	}
}
	
class CZoneInfo;
typedef CZoneInfo* PZone;
typedef std::list<CZoneInfo*> CZoneInfoList;
typedef std::list<CZoneInfo*>::iterator CZoneInfoListIter;
class CMapInfo;
typedef std::list<CMapInfo*> CMapInfoList;
typedef std::list<CMapInfo*>::iterator CMapInfoListIter;
//typedef void(__stdcall *TraverseZoneOfMapCB)(void* udata, CZoneInfo* zone);

typedef void(__stdcall *OnOtherTryEnterBufferMode)(void* udata);
typedef struct OnOtherTryEnterBufferModeObj
{
private:
	OnOtherTryEnterBufferMode _cb;
	void* _udata;
public:
	OnOtherTryEnterBufferModeObj() : _cb(NULL), _udata(NULL) {}
	bool valid() const { return _cb && _udata; }
	void call() { if (valid())_cb(_udata); }
	void update(OnOtherTryEnterBufferMode cb, void* udata) { _cb = cb; _udata = udata; }
	void reset() { _cb = NULL; _udata = NULL; }
}OnOtherTryEnterBufferModeObj;

using namespace ademco;

class CAlarmMachine
{
private:
	int _id;
	int _ademco_id;
	int _group_id;
	MachineType _machine_type;
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
	bool _alarming;
	bool _has_alarming_direct_zone;
	bool _buffer_mode;
	bool _is_submachine;
	bool _has_video;
	int _submachine_zone;
	int _submachine_count;
	CMapInfo* _unbindZoneMap;
	CMapInfoList _mapList;
	CMapInfoListIter _curMapListIter;
	std::list<ademco::AdemcoEvent*> _ademcoEventList;
	std::list<ademco::AdemcoEvent*> _ademcoEventFilter;
	CLock _lock4AdemcoEventList;
	PZone _zoneArray[MAX_MACHINE_ZONE];
	CZoneInfoList _validZoneList;
	ConnHangupObj _connHangupObj;
	EventLevel _highestEventLevel;
	long _alarmingSubMachineCount;
	time_t _lastActionTime;
	bool _bChecking;
	OnOtherTryEnterBufferModeObj _ootebmOjb;
	COleDateTime _expire_time;
protected:
	void HandleAdemcoEvent(const ademco::AdemcoEvent* ademcoEvent, BOOL bDeleteAfterHandled = TRUE);
	void inc_alarmingSubMachineCount();
	void dec_alarmingSubMachineCount();
	void set_highestEventLevel(EventLevel level);
	void NotifySubmachines(const ademco::AdemcoEvent* ademcoEvent);
	void HandleRetrieveResult(const ademco::AdemcoEvent* ademcoEvent);
	void UpdateLastActionTime() { AUTO_LOG_FUNCTION; LOG(L"subMachine %03d, %s", _submachine_zone, _alias); _lastActionTime = time(NULL); }
public:
	CAlarmMachine();
	~CAlarmMachine();
	//bool IsOnline() const { return _online; }
	//bool IsArmed() const { return _armed; }
	void clear_ademco_event_list();

	// 2015��5��6�� 15:58:07 �ֻ�����16Сʱ���Զ�������
	time_t GetLastActionTime() const { return _lastActionTime; }

	// 2015��4��22�� 16:55:04 ��ť��ɫ��ء��ֱ�������зֻ���Ϣ�����������ť��ɫ
	EventLevel get_highestEventLevel() const { return _highestEventLevel; }
	long get_alarmingSubMachineCount() const { return _alarmingSubMachineCount; }

	// 2015��4��16�� 15:45:06 ��·�������
	void SetConnHangupCallback(void* udata, ConnHangupCB cb) { _connHangupObj.udata = udata; _connHangupObj.cb = cb; }

	// 2015��3��24�� 17:45:11 �ֻ����
	void inc_submachine_count();
	void dec_submachine_count();
	int get_submachine_count() { /*AUTO_LOG_FUNCTION;*/ return _submachine_count; }

	// 2015��3��4�� 14:29:34 ��������
	void AddZone(CZoneInfo* zoneInfo);
	CZoneInfo* GetZone(int zone);
	// 2015��3��3�� 14:16:10 ��ȡ���з�����Ϣ
	void GetAllZoneInfo(CZoneInfoList& list);
	int get_zone_count() const { return _validZoneList.size(); }

	// 2015��2��25�� 15:50:16 �����������ݿ���޸Ĳ���
	bool execute_set_banned(bool banned = true);
	bool execute_set_machine_type(MachineType type);
	bool execute_set_has_video(bool has);
	bool execute_set_alias(const wchar_t* alias);
	bool execute_set_contact(const wchar_t* contact);
	bool execute_set_address(const wchar_t* address);
	bool execute_set_phone(const wchar_t* phone);
	bool execute_set_phone_bk(const wchar_t* phone_bk);
	bool execute_set_group_id(int group_id);
	bool execute_add_map(CMapInfo* mapInfo);
	bool execute_update_map_alias(CMapInfo* mapInfo, const wchar_t* alias);
	bool execute_update_map_path(CMapInfo* mapInfo, const wchar_t* path);
	bool execute_delete_map(CMapInfo* mapInfo);
	bool execute_update_expire_time(const COleDateTime& datetime);
	
	// 2015��3��16�� 16:19:27 �����������ݿ�ķ�������
	bool execute_add_zone(CZoneInfo* zoneInfo);
	bool execute_del_zone(CZoneInfo* zoneInfo);
	// 2015��2��12�� 21:34:56
	// ���༭ĳ������ʱ�����������յ������¼����Ȼ��棬�˳��༭���� notify observers.
	bool EnterBufferMode();
	bool LeaveBufferMode();
	// 2015��5��6�� 21:03:22
	// ��EnterBufferModeʱ�����ô�obj���Ա������ط�����EnterBufferModeʱLeaveBufferMode
	void SetOotebmObj(OnOtherTryEnterBufferMode cb, void* udata) { _ootebmOjb.update(cb, udata); }

	void AddMap(CMapInfo* map) { _mapList.push_back(map); }
	//CMapInfo* GetFirstMap();
	//CMapInfo* GetNextMap();
	//void SetUnbindZoneMap(CMapInfo* map) { _unbindZoneMap = map; }
	CMapInfo* GetUnbindZoneMap() { return _unbindZoneMap; }
	void GetAllMapInfo(CMapInfoList& list);
	CMapInfo* GetMapInfo(int map_id);
	
	void SetAdemcoEvent(int ademco_event, int zone, int subzone, const time_t& event_time,
						const char* xdata, int xdata_len);
	//void SetAdemcoEvent(const ademco::AdemcoEvent* ademcoEvent);
	void TraverseAdmecoEventList(void* udata, ademco::AdemcoEventCB cb);

	const char* GetDeviceIDA() const { return _device_id; }
	const wchar_t* GetDeviceIDW() const { return _device_idW; }

	void set_device_id(const wchar_t* device_id);
	void set_device_id(const char* device_id);

	MachineType get_machine_type() const { return _machine_type; }
	void set_machine_type(MachineType type) { _machine_type = type; }

	DECLARE_GETTER_SETTER_INT(_id);
	DECLARE_GETTER_SETTER_INT(_ademco_id); 
	DECLARE_GETTER_SETTER_INT(_group_id);
	DECLARE_GETTER_SETTER(bool, _banned); 
	DECLARE_GETTER_SETTER(bool, _is_submachine); 
	DECLARE_GETTER_SETTER(bool, _alarming);
	DECLARE_GETTER_SETTER(bool, _online);
	DECLARE_GETTER_SETTER(bool, _armed);
	DECLARE_GETTER_SETTER(bool, _has_video); 
	DECLARE_GETTER_SETTER(bool, _bChecking);
	DECLARE_GETTER_SETTER_INT(_submachine_zone);

	DECLARE_GETTER_SETTER_STRING(_alias);
	DECLARE_GETTER_SETTER_STRING(_contact);
	DECLARE_GETTER_SETTER_STRING(_address);
	DECLARE_GETTER_SETTER_STRING(_phone);
	DECLARE_GETTER_SETTER_STRING(_phone_bk);
	DECLARE_GETTER_SETTER(COleDateTime, _expire_time);
	DECLARE_OBSERVER(AdemcoEventCB, AdemcoEvent*);
	DECLARE_UNCOPYABLE(CAlarmMachine);
};



NAMESPACE_END
