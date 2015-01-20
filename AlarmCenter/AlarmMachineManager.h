#pragma once

//#include "C:/Global/JTL/vector/vector.h"
#include <list>

namespace ado { class CADODatabase; };

namespace core { 

static const int MAX_MACHINE = 10000;
static const int MAX_MACHINE_ZONE = 1000;

class CAlarmMachine; 
class CAlarmMachineManager
{
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
protected:
	void LoadAlarmMachineFromDB();
public:
	int GetMachineCount() const;
	BOOL GetFirstMachine(CAlarmMachine*& machine);
	BOOL GetNextMachine(CAlarmMachine*& machine);
	BOOL CheckMachine(int ademco_id, const wchar_t* device_id, int zone);
};

NAMESPACE_END
