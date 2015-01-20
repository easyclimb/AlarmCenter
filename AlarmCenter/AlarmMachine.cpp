#include "stdafx.h"
#include "AlarmMachine.h"

namespace core {

CAlarmMachine::CAlarmMachine()
{
	memset(this, 0, sizeof(this));
}


CAlarmMachine::~CAlarmMachine()
{
	if (_alias) {
		delete[] _alias;
	}
}








NAMESPACE_END
