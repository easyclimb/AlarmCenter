#pragma once

//#include "C:/Global/JTL/vector/vector.h"
#include <list>
#include <map>
#include "core.h"
namespace ado { class CDbOper; };

namespace core { 

static const int MAX_MACHINE = 1000000;

class alarm_machine_manager : private boost::noncopyable
{
public:
	~alarm_machine_manager();
private:
	
	alarm_machine_map m_machineMap;
	map_info_map m_mapInfoMap;
	detector_info_list m_detectorList;
	std::map<std::pair<int, int>, camera_info_list> m_cameraMap;
	std::map<int, camera_info_ptr> m_cameraIdMap;
	std::shared_ptr<ado::CDbOper> m_db;
	CWnd* m_pPrevCallDisarmWnd;
	int m_prevCallDisarmAdemcoID;
	int m_prevCallDisarmGG;
	int m_prevCallDisarmZoneValue;
protected:
	// functions declared below must be called sequencially.
	void InitCsrInfo();
	void InitDB();
	void InitDetectorLib();
	void LoadDetectorLibFromDB();
	void LoadGroupInfoFromDB();
	void LoadAlarmMachineFromDB(void* udata, LoadDBProgressCB cb);
#ifdef _DEBUG_TestLoadAlarmMachineFromDB
	void TestLoadAlarmMachineFromDB(void* udata, LoadDBProgressCB cb);
#endif

	// functions below are called by the functions declared above.
	void LoadMapInfoFromDB(const core::alarm_machine_ptr& machine);
	void LoadZoneInfoFromDB(const core::alarm_machine_ptr& machine, void* udata, LoadDBProgressCB cb);
	detector_info_ptr LoadDetectorInfoFromDB(int id);
	void LoadSubMachineInfoFromDB(const zone_info_ptr& zone);
	void LoadSubZoneInfoOfSubMachineFromDB(const core::alarm_machine_ptr& subMachine);
	void LoadCameraInfoFromDB();
	void LoadServiceInfoFromDB();

	static DWORD WINAPI ThreadCheckSubMachine(LPVOID lp);
	typedef struct CHECKER_PARAM{
		alarm_machine_manager* mgr;
		int ademco_id;
		int zone_value;
	}CHECKER_PARAM;
	HANDLE m_hThread;
	HANDLE m_hEventExit;
	HANDLE m_hEventOotebm;
	std::mutex m_lock4Machines;
	
public:

	detector_info_ptr GetDetectorInfo(int id);
	void DeleteDetector(const detector_info_ptr& detector) { m_detectorList.remove(detector); }
	void AddDetector(const detector_info_ptr& detector) { m_detectorList.push_back(detector); }
	void AddMapInfo(const core::map_info_ptr& mapInfo);
	void DeleteMapInfo(const core::map_info_ptr& mapInfo);

	void ResolveCameraInfo(int device_id, int productor);
	void DeleteCameraInfo(const camera_info_ptr& camera);
	void DeleteCameraInfo(int device_id, int productor);
	void AddCameraInfo(const camera_info_ptr& camera);
	camera_info_ptr GetCameraInfo(int id) { return m_cameraIdMap[id]; }

	map_info_ptr GetMapInfoById(int id);
	void LoadFromDB(void* udata = nullptr, LoadDBProgressCB cb = nullptr);
	BOOL RemoteControlAlarmMachine(const alarm_machine_ptr& machine,
								   int ademco_event, int gg, int zone, 
								   const ademco::char_array_ptr& xdata = nullptr, 
								   const ademco::char_array_ptr& cmd = nullptr,
								   ademco::EventSource path = ademco::ES_UNKNOWN,
								   CWnd* pWnd = nullptr);
	void DisarmPasswdWrong(int ademco_id);
	int GetMachineCount() const;
	alarm_machine_ptr GetMachine(int ademco_id);


	BOOL CheckIsValidMachine(int ademco_id, /*const char* device_id, */int zone);
	BOOL CheckIfMachineAdemcoIdCanUse(int ademco_id);
	void MachineOnline(ademco::EventSource source, int ademco_id, BOOL online = TRUE, const char* ipv4 = nullptr, 
					   net::server::CClientDataPtr = nullptr, remote_control_command_conn_call_back cb = nullptr);
	void MachineEventHandler(ademco::EventSource source, int ademco_id, int ademco_event, int zone,
							 int subzone, const time_t& timestamp,
							 const time_t& recv_time,
							 const ademco::char_array_ptr& xdata = nullptr
							 );
	BOOL DistributeAdemcoID(int& ademco_id);
	BOOL AddMachine(const core::alarm_machine_ptr& machine);
	BOOL DeleteMachine(const core::alarm_machine_ptr& machine);
	BOOL DeleteSubMachine(const zone_info_ptr& zoneInfo);
	// 2015年2月12日 21:54:36 
	// 进入编辑模式后，使所有主机进入 buffer mode
	BOOL EnterBufferMode();
	BOOL LeaveBufferMode();

	BOOL ExecuteSql(const CString& query);
	int AddAutoIndexTableReturnID(const CString& query);
	static void __stdcall OnOtherCallEnterBufferMode(void* udata);
	void DeleteVideoBindInfoByZoneInfo(const zone_info_ptr& zoneInfo);
private:
	DECLARE_SINGLETON(alarm_machine_manager)
	
};

};

