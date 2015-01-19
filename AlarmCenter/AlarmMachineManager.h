#pragma once

//#include "C:/Global/JTL/vector/vector.h"
#include <vector>

namespace ado { class CADODatabase; };

namespace core { 
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
	std::vector<CAlarmMachine*> m_vectorAlarmMachine;
	ado::CADODatabase* m_pDatabase;
protected:
	void LoadAlarmMachineFromDB();
public:
	int GetMachineCount() const;
	BOOL GetMachine(int id, CAlarmMachine*& machine);

};

NAMESPACE_END
