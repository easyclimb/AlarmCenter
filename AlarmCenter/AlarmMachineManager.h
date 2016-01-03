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
	CMapInfoList m_mapList;
	CDetectorInfoList m_detectorList;
	std::map<std::pair<int, int>, CCameraInfoList> m_cameraMap;
	std::map<int, CCameraInfoPtr> m_cameraIdMap;
	ado::CDbOper* m_db;
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
	void LoadMapInfoFromDB(CAlarmMachinePtr machine);
	//void LoadNoZoneHasMapDetectorInfoFromDB(CMapInfoPtr mapInfo);
	void LoadZoneInfoFromDB(CAlarmMachinePtr machine, void* udata, LoadDBProgressCB cb);
	CDetectorInfoPtr LoadDetectorInfoFromDB(int id);
	void LoadSubMachineInfoFromDB(CZoneInfoPtr zone);
	void LoadSubZoneInfoOfSubMachineFromDB(CAlarmMachinePtr subMachine);
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
	CLock m_lock4Machines;
	
public:

	CDetectorInfoPtr GetDetectorInfo(int id);
	void DeleteDetector(CDetectorInfoPtr detector) { m_detectorList.remove(detector); }
	void AddDetector(CDetectorInfoPtr detector) { m_detectorList.push_back(detector); }
	void AddMapInfo(CMapInfoPtr mapInfo) { m_mapList.push_back(mapInfo); }
	void DeleteMapInfo(CMapInfoPtr mapInfo) { m_mapList.remove(mapInfo); }

	void ResolveCameraInfo(int device_id, int productor);
	void DeleteCameraInfo(CCameraInfoPtr camera);
	void DeleteCameraInfo(int device_id, int productor);
	void AddCameraInfo(CCameraInfoPtr camera);
	CCameraInfoPtr GetCameraInfo(int id) { return m_cameraIdMap[id]; }

	CMapInfoPtr GetMapInfoById(int id);
	void LoadFromDB(void* udata = nullptr, LoadDBProgressCB cb = nullptr);
	BOOL RemoteControlAlarmMachine(const CAlarmMachinePtr machine,
								   int ademco_event, int gg, int zone, 
								   const char* xdata, int xdata_len, CWnd* pWnd);
	void DisarmPasswdWrong(int ademco_id);
	int GetMachineCount() const;
	CAlarmMachinePtr GetMachine(int ademco_id);


	BOOL CheckIsValidMachine(int ademco_id, /*const char* device_id, */int zone);
	BOOL CheckIfMachineAdemcoIdCanUse(int ademco_id);
	void MachineOnline(ademco::EventSource resource, int ademco_id, BOOL online = TRUE, const char* ipv4 = nullptr, 
					   net::server::CClientDataPtr = nullptr, RemoteControlCommandConnCB cb = nullptr);
	void MachineEventHandler(ademco::EventSource resource, int ademco_id, int ademco_event, int zone, 
							 int subzone, const time_t& timestamp,
							 const time_t& recv_time,
							 const std::vector<char>& xdata
							 );
	BOOL DistributeAdemcoID(int& ademco_id);
	BOOL AddMachine(CAlarmMachinePtr machine);
	BOOL DeleteMachine(CAlarmMachinePtr machine);
	BOOL DeleteSubMachine(CZoneInfoPtr zoneInfo);
	// 2015年2月12日 21:54:36 
	// 进入编辑模式后，使所有主机进入 buffer mode
	void EnterEditMode();
	void LeaveEditMode();

	BOOL ExecuteSql(const CString& query);
	int AddAutoIndexTableReturnID(const CString& query);
	static void __stdcall OnOtherCallEnterBufferMode(void* udata);
	void DeleteVideoBindInfoByZoneInfo(core::CZoneInfoPtr zoneInfo);
private:
	DECLARE_UNCOPYABLE(CAlarmMachineManager)
	DECLARE_SINGLETON(CAlarmMachineManager)
	
};

NAMESPACE_END
