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
//class CDetectorLib;
class CAlarmMachineManager
{
public:
	~CAlarmMachineManager();
private:
	std::list<CAlarmMachine*> m_listAlarmMachine;
	std::list<CAlarmMachine*>::iterator m_curMachinePos;
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
	BOOL CheckMachine(int ademco_id, const char* device_id, int zone);
	BOOL CheckMachine(const char* device_id);
	void MachineOnline(int ademco_id, BOOL online = TRUE);
	void MachineEventHandler(int ademco_id, int ademco_event, int zone, const time_t& event_time);
	BOOL DistributeAdemcoID(int& ademco_id);
	BOOL AddMachine(int ademco_id, const char* device_id, const wchar_t* alias);

private:
	DECLARE_UNCOPYABLE(CAlarmMachineManager)
	DECLARE_SINGLETON(CAlarmMachineManager)
};

NAMESPACE_END
