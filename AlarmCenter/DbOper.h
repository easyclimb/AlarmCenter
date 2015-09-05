#pragma once

#include <odbcinst.h>
#include <afxdb.h>
#include <comdef.h>
#include "ado2.h"

namespace ado {

class CDbOper
{
private:
	ado::CADODatabase* m_pDatabase;
protected:
	void Check();
public:
	int AddAutoIndexTableReturnID(const CString& query);
	bool Execute(const CString& sql);
	bool Open(const CString& mdbName, const CString& passwd = L"");
	ado::CADODatabase* GetDatabase() { Check(); return m_pDatabase; }
	CDbOper();
	virtual ~CDbOper();
};

NAMESPACE_END
