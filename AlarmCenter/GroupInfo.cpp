#include "stdafx.h"
#include "GroupInfo.h"



namespace core {

CGroupInfo::CGroupInfo()
	: _id(0), _parent_id(0), _name(NULL)
{
	_name = new wchar_t[1];
	_name[0] = 0;
}


CGroupInfo::~CGroupInfo()
{
	SAFEDELETEARR(_name);
}





NAMESPACE_END
