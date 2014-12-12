#pragma once

#include "C:/Global/JTL/vector/vector.h"

class CADODatabase;
//class CADORecordset;
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
	JTL::Vector<CAlarmMachine*> m_vectorAlarmMachine;
	CADODatabase* m_pDatabase;
protected:
	void LoadAlarmMachineFromDB();
public:
	int GetMachineCount() const;
	BOOL GetMachine(int id, CAlarmMachine*& machine);
};

