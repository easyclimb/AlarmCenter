#pragma once

//#include "C:/Global/JTL/vector/vector.h"
#include <list>

namespace ado { class CADODatabase; };

namespace core { 

static const int MAX_MACHINE = 10000;
static const int MAX_MACHINE_ZONE = 1000;

class CMapInfo;
class CZoneInfo;
class CAlarmMachine; 
//class CGroupInfo;

//typedef std::list<CAlarmMachine*> CAlarmMachineList;
//typedef std::list<CGroupInfo*> CCGroupInfoList;
//class CDetectorLib;
class CAlarmMachineManager
{
public:
	~CAlarmMachineManager();
private:
	std::list<CAlarmMachine*> m_listAlarmMachine;
	std::list<CAlarmMachine*>::iterator m_curMachinePos;
	//CGroupInfo* m_rootGroupInfo;
	//CCGroupInfoList m_listGroupInfo;
	ado::CADODatabase* m_pDatabase;
	wchar_t m_csr_acctW[64];
	char m_csr_acctA[64];
	CWnd* m_pPrevCallDisarmWnd;
	int m_prevCallDisarmAdemcoID;
	//CDetectorLib* m_detectorLib;
protected:
	// functions declared below must be called sequencially.
	void InitDB();
	void InitDetectorLib();
	void LoadDetectorLibFromDB();
	void LoadZonePropertyInfoFromDB();
	void LoadGroupInfoFromDB();
	void LoadAlarmMachineFromDB();

	// functions below are called by the functions declared above.
	void LoadMapInfoFromDB(CAlarmMachine* machine);
	void LoadNoZoneMapInfoFromDB(CAlarmMachine* machine);
	//void LoadZoneInfoFromDB(CAlarmMachine* machine);
	void LoadZoneInfoFromDB(CMapInfo* mapInfo);
	void LoadDetectorInfoFromDB(CZoneInfo* zone);
	
public:
	BOOL RemoteControlAlarmMachine(const CAlarmMachine* machine, 
								   int ademco_event, CWnd* pWnd);
	void DisarmPasswdWrong(int ademco_id);
	const wchar_t* GetCsrAcctW() const;
	const char* GetCsrAcctA() const;
	void SetCsrAcct(const char* csr_acct);
	void SetCsrAcct(const wchar_t* csr_acct);
	int GetMachineCount() const;
	BOOL GetMachine(int ademco_id, CAlarmMachine*& machine);
	BOOL GetFirstMachine(CAlarmMachine*& machine);
	BOOL GetNextMachine(CAlarmMachine*& machine);

	//CGroupInfo* GetRootGroupInfo() const { return m_rootGroupInfo; }
	//void GetChildGroupInfoList(int group_id, CCGroupInfoList& list);
	//CGroupInfo* GetGroupInfo(int group_id);
	//void GetChildMachineList(int group_id, CAlarmMachineList& list);

	BOOL CheckMachine(int ademco_id, const char* device_id, int zone);
	BOOL CheckIfMachineAcctUnique(const char* device_id);
	void MachineOnline(int ademco_id, BOOL online = TRUE);
	void MachineEventHandler(int ademco_id, int ademco_event, int zone, const time_t& event_time);
	BOOL DistributeAdemcoID(int& ademco_id);
	BOOL AddMachine(int ademco_id, const char* device_id, const wchar_t* alias);
	BOOL DeleteMachine(CAlarmMachine* machine);
	// 2015年2月12日 21:54:36 
	// 进入编辑模式后，使所有主机进入 buffer mode
	void EnterEditMode();
	void LeaveEditMode();

	BOOL ExecuteSql(const CString& query);
	int AddAutoIndexTableReturnID(const CString& query);
private:
	DECLARE_UNCOPYABLE(CAlarmMachineManager)
	DECLARE_SINGLETON(CAlarmMachineManager)
};

NAMESPACE_END
