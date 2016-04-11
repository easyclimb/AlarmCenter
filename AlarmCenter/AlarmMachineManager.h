#pragma once

//#include "C:/Global/JTL/vector/vector.h"
#include <list>
#include <map>
#include "core.h"
namespace ado { class CDbOper; };

namespace core { 

static const int MAX_MACHINE = 10000;

class CAlarmMachineManager
{
public:
	~CAlarmMachineManager();
private:
	
	CAlarmMachineMap m_machineMap;
	CMapInfoMap m_mapInfoMap;
	CDetectorInfoList m_detectorList;
	std::map<std::pair<int, int>, CCameraInfoList> m_cameraMap;
	std::map<int, CCameraInfoPtr> m_cameraIdMap;
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
	void LoadMapInfoFromDB(const core::CAlarmMachinePtr& machine);
	void LoadZoneInfoFromDB(const core::CAlarmMachinePtr& machine, void* udata, LoadDBProgressCB cb);
	CDetectorInfoPtr LoadDetectorInfoFromDB(int id);
	void LoadSubMachineInfoFromDB(const CZoneInfoPtr& zone);
	void LoadSubZoneInfoOfSubMachineFromDB(const core::CAlarmMachinePtr& subMachine);
	void LoadCameraInfoFromDB();

	static DWORD WINAPI ThreadCheckSubMachine(LPVOID lp);
	typedef struct CHECKER_PARAM{
		CAlarmMachineManager* mgr;
		int ademco_id;
		int zone_value;
	}CHECKER_PARAM;
	HANDLE m_hThread;
	HANDLE m_hEventExit;
	HANDLE m_hEventOotebm;
	std::mutex m_lock4Machines;
	
public:

	CDetectorInfoPtr GetDetectorInfo(int id);
	void DeleteDetector(const CDetectorInfoPtr& detector) { m_detectorList.remove(detector); }
	void AddDetector(const CDetectorInfoPtr& detector) { m_detectorList.push_back(detector); }
	void AddMapInfo(const core::CMapInfoPtr& mapInfo);
	void DeleteMapInfo(const core::CMapInfoPtr& mapInfo);

	void ResolveCameraInfo(int device_id, int productor);
	void DeleteCameraInfo(const CCameraInfoPtr& camera);
	void DeleteCameraInfo(int device_id, int productor);
	void AddCameraInfo(const CCameraInfoPtr& camera);
	CCameraInfoPtr GetCameraInfo(int id) { return m_cameraIdMap[id]; }

	CMapInfoPtr GetMapInfoById(int id);
	void LoadFromDB(void* udata = nullptr, LoadDBProgressCB cb = nullptr);
	BOOL RemoteControlAlarmMachine(const CAlarmMachinePtr& machine,
								   int ademco_event, int gg, int zone, 
								   const ademco::char_array_ptr& xdata = nullptr, 
								   const ademco::char_array_ptr& cmd = nullptr,
								   ademco::EventSource path = ademco::ES_UNKNOWN,
								   CWnd* pWnd = nullptr);
	void DisarmPasswdWrong(int ademco_id);
	int GetMachineCount() const;
	CAlarmMachinePtr GetMachine(int ademco_id);


	BOOL CheckIsValidMachine(int ademco_id, /*const char* device_id, */int zone);
	BOOL CheckIfMachineAdemcoIdCanUse(int ademco_id);
	void MachineOnline(ademco::EventSource source, int ademco_id, BOOL online = TRUE, const char* ipv4 = nullptr, 
					   net::server::CClientDataPtr = nullptr, RemoteControlCommandConnCB cb = nullptr);
	void MachineEventHandler(ademco::EventSource source, int ademco_id, int ademco_event, int zone,
							 int subzone, const time_t& timestamp,
							 const time_t& recv_time,
							 const ademco::char_array_ptr& xdata = nullptr
							 );
	BOOL DistributeAdemcoID(int& ademco_id);
	BOOL AddMachine(const core::CAlarmMachinePtr& machine);
	BOOL DeleteMachine(const core::CAlarmMachinePtr& machine);
	BOOL DeleteSubMachine(const CZoneInfoPtr& zoneInfo);
	// 2015年2月12日 21:54:36 
	// 进入编辑模式后，使所有主机进入 buffer mode
	BOOL EnterBufferMode();
	BOOL LeaveBufferMode();

	BOOL ExecuteSql(const CString& query);
	int AddAutoIndexTableReturnID(const CString& query);
	static void __stdcall OnOtherCallEnterBufferMode(void* udata);
	void DeleteVideoBindInfoByZoneInfo(const CZoneInfoPtr& zoneInfo);
private:
	DECLARE_UNCOPYABLE(CAlarmMachineManager)
	DECLARE_SINGLETON(CAlarmMachineManager)
	
};

NAMESPACE_END
