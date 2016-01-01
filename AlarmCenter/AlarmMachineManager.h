#pragma once

//#include "C:/Global/JTL/vector/vector.h"
#include <list>
#include <map>
#include "core.h"
namespace ado { class CDbOper; };

namespace core { 

#define USE_ARRAY

static const int MAX_MACHINE = 10000;

typedef struct ProgressEx {
	int progress;
	int value;
	int total;
	ProgressEx* subProgress;
	ProgressEx() : progress(0), value(0), total(0), subProgress(nullptr) {}
}ProgressEx;
typedef void(__stdcall *LoadDBProgressCB)(void* udata, bool bmain, const ProgressEx* progress);


class CDetectorInfo;
class CMapInfo;
class CZoneInfo;
class CCameraInfo;
class CAlarmMachine; 
typedef CAlarmMachine* PMachine;
//class CSubMachineInfo;
//class CGroupInfo;

//typedef std::list<CAlarmMachine*> CAlarmMachineList;
//typedef std::list<CGroupInfo*> CCGroupInfoList;
//class CDetectorLib;
class CAlarmMachineManager
{
public:
	~CAlarmMachineManager();
private:
	
#ifdef USE_ARRAY
	PMachine m_alarmMachines[MAX_MACHINE];
	int m_curMachinePos;
	int m_validMachineCount;
#else
	std::list<CAlarmMachine*> m_listAlarmMachine;
	std::list<CAlarmMachine*>::iterator m_curMachinePos;
#endif
	core::CMapInfoList m_mapList;
	//CGroupInfo* m_rootGroupInfo;
	//CCGroupInfoList m_listGroupInfo;
	//ado::CADODatabase* m_pDatabase;
	ado::CDbOper* m_db;
	//wchar_t m_csr_acctW[64];
	//char m_csr_acctA[64];
	CWnd* m_pPrevCallDisarmWnd;
	int m_prevCallDisarmAdemcoID;
	int m_prevCallDisarmGG;
	int m_prevCallDisarmZoneValue;
	//CDetectorLib* m_detectorLib;
protected:
	// functions declared below must be called sequencially.
	void InitCsrInfo();
	void InitDB();
	void InitDetectorLib();
	void LoadDetectorLibFromDB();
	//void LoadZonePropertyInfoFromDB();
	void LoadGroupInfoFromDB();
	void LoadAlarmMachineFromDB(void* udata, LoadDBProgressCB cb);
#ifdef _DEBUG_TestLoadAlarmMachineFromDB
	void TestLoadAlarmMachineFromDB(void* udata, LoadDBProgressCB cb);
#endif

	// functions below are called by the functions declared above.
	void LoadMapInfoFromDB(CAlarmMachine* machine);
	void LoadNoZoneHasMapDetectorInfoFromDB(CMapInfoPtr mapInfo);
	void LoadZoneInfoFromDB(CAlarmMachine* machine, void* udata, LoadDBProgressCB cb, ProgressEx* progress);
	CDetectorInfo* LoadDetectorInfoFromDB(int id);
	void LoadSubMachineInfoFromDB(CZoneInfo* zone);
	void LoadSubZoneInfoOfSubMachineFromDB(CAlarmMachine* subMachine);
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
	std::map<std::pair<int, int>, std::list<CCameraInfo*>> m_cameraMap;
public:
	void AddMapInfo(CMapInfoPtr mapInfo) { m_mapList.push_back(mapInfo); }
	void DeleteMapInfo(CMapInfoPtr mapInfo) { m_mapList.remove(mapInfo); }
	void ResolveCameraInfo(int device_id, int productor);
	void DeleteCameraInfo(CCameraInfo* camera);
	void DeleteCameraInfo(int device_id, int productor);
	void AddCameraInfo(CCameraInfo* camera);
	CMapInfoPtr GetMapInfoById(int id);
	void LoadFromDB(void* udata = nullptr, LoadDBProgressCB cb = nullptr);
	BOOL RemoteControlAlarmMachine(const CAlarmMachine* machine,
								   int ademco_event, int gg, int zone, 
								   const char* xdata, int xdata_len, CWnd* pWnd);
	void DisarmPasswdWrong(int ademco_id);
	/*const wchar_t* GetCsrAcctW() const;
	const char* GetCsrAcctA() const;
	void SetCsrAcct(const char* csr_acct);
	void SetCsrAcct(const wchar_t* csr_acct);*/
	int GetMachineCount() const;
	BOOL GetMachine(int ademco_id, CAlarmMachine*& machine);
	BOOL GetFirstMachine(CAlarmMachine*& machine);
	BOOL GetNextMachine(CAlarmMachine*& machine);

	//CGroupInfo* GetRootGroupInfo() const { return m_rootGroupInfo; }
	//void GetChildGroupInfoList(int group_id, CCGroupInfoList& list);
	//CGroupInfo* GetGroupInfo(int group_id);
	//void GetChildMachineList(int group_id, CAlarmMachineList& list);

	BOOL CheckIsValidMachine(int ademco_id, /*const char* device_id, */int zone);
	//BOOL CheckIfMachineAcctAlreadyInuse(const char* device_id);
	//BOOL CheckIfMachineAcctAlreadyInuse(const wchar_t* device_id);
	BOOL CheckIfMachineAdemcoIdCanUse(int ademco_id);
	void MachineOnline(ademco::EventSource resource, int ademco_id, BOOL online = TRUE, const char* ipv4 = nullptr, void* udata = nullptr, RemoteControlCommandConnCB cb = nullptr);
	void MachineEventHandler(ademco::EventSource resource, int ademco_id, int ademco_event, int zone, 
							 int subzone, const time_t& timestamp,
							 const time_t& recv_time,
							 const std::vector<char>& xdata
							 );
	BOOL DistributeAdemcoID(int& ademco_id);
	//BOOL AddMachine(int ademco_id, const char* device_id, const wchar_t* alias);
	BOOL AddMachine(CAlarmMachine* machine);
	BOOL DeleteMachine(CAlarmMachine* machine);
	BOOL DeleteSubMachine(CZoneInfo* zoneInfo);
	// 2015年2月12日 21:54:36 
	// 进入编辑模式后，使所有主机进入 buffer mode
	void EnterEditMode();
	void LeaveEditMode();

	BOOL ExecuteSql(const CString& query);
	int AddAutoIndexTableReturnID(const CString& query);
	static void __stdcall OnOtherCallEnterBufferMode(void* udata);
	void DeleteVideoBindInfoByZoneInfo(core::CZoneInfo* zoneInfo);
private:
	DECLARE_UNCOPYABLE(CAlarmMachineManager)
	DECLARE_SINGLETON(CAlarmMachineManager)
	
};

NAMESPACE_END
