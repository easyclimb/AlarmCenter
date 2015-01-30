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
	DECLARE_UNCOPYABLE(CAlarmMachineManager)
public:
	static CAlarmMachineManager* GetInstance();
	~CAlarmMachineManager();
protected:
	CAlarmMachineManager();
private:
	static CLock m_lock;
	std::list<CAlarmMachine*> m_listAlarmMachine;
	std::list<CAlarmMachine*>::iterator m_curMachinePos;
	ado::CADODatabase* m_pDatabase;
	wchar_t m_csr_acct[64];
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
	void LoadZoneInfoFromDB(CMapInfo* mapInfo);
	void LoadDetectorInfoFromDB(CZoneInfo* zone);
	
public:
	const wchar_t* GetCsrAcct() const;
	int GetMachineCount() const;
	BOOL GetMachine(int ademco_id, CAlarmMachine*& machine);
	BOOL GetFirstMachine(CAlarmMachine*& machine);
	BOOL GetNextMachine(CAlarmMachine*& machine);
	BOOL CheckMachine(int ademco_id, const wchar_t* device_id, int zone);
	BOOL CheckMachine(const wchar_t* device_id);
	void MachineOnline(int ademco_id, BOOL online = TRUE);
	void MachineEventHandler(int ademco_id, int ademco_event, int zone);
	BOOL DistributeAdemcoID(int& ademco_id);
	BOOL AddMachine(int ademco_id, const wchar_t* device_id, const wchar_t* alias);
};

NAMESPACE_END
