#include "stdafx.h"
#include "GroupInfo.h"
#include "AlarmMachine.h"


namespace core {

CGroupInfo::CGroupInfo()
	: _id(0), _parent_id(0), _name(NULL)
	, _machine_count(0)
	, _parent_group(NULL)
{
	_name = new wchar_t[1];
	_name[0] = 0;
}


CGroupInfo::~CGroupInfo()
{
	SAFEDELETEARR(_name);

	std::list<CGroupInfo*>::iterator iter = _child_groups.begin();
	while (iter != _child_groups.end()) {
		CGroupInfo* child_group = *iter++;
		delete child_group;
	}
	_child_groups.clear();
}


void CGroupInfo::UpdateCount(bool bAdd)
{
	bAdd ? (_machine_count++) : (_machine_count--);
	if (_parent_group) {
		_parent_group->UpdateCount(bAdd);
	}
}


bool CGroupInfo::IsDescendantGroup(CGroupInfo* group)
{
	/*if (_id == group->get_parent_id)
		return true;

	std::list<CGroupInfo*>::iterator iter = _child_groups.begin();
	while (iter != _child_groups.end()) {
		CGroupInfo* child_group = *iter++;
		if (child_group == group) {
			return true;
		}

		if (child_group->IsDescendantGroup(group)) {
			return true;
		}
	}*/

	CGroupInfo* parent_group = group->get_parent_group();
	while (parent_group) {
		if (parent_group == this) { return true; }
		parent_group = parent_group->get_parent_group();
	}

	return false;
}


bool CGroupInfo::AddChildGroup(CGroupInfo* group)
{
	if (_id == group->get_parent_id()) {
		group->set_parent_group(this);
		_child_groups.push_back(group);
		return true;
	}

	std::list<CGroupInfo*>::iterator iter = _child_groups.begin();
	while (iter != _child_groups.end()) {
		CGroupInfo* child_group = *iter++;
		if (child_group->AddChildGroup(group)) {
			return true;
		}
	}
	return false;
}


// 获取所有儿子分组
void CGroupInfo::GetChildGroups(CCGroupInfoList& list)
{
	std::list<CGroupInfo*>::iterator iter = _child_groups.begin();
	while (iter != _child_groups.end()) {
		CGroupInfo* child_group = *iter++;
		list.push_back(child_group);
	}
}


// 获取所有后代分组(包括儿子分组)
void CGroupInfo::GetDescendantGroups(CCGroupInfoList& list)
{
	std::list<CGroupInfo*>::iterator iter = _child_groups.begin();
	while (iter != _child_groups.end()) {
		CGroupInfo* child_group = *iter++;
		child_group->GetDescendantGroups(list);
	}

	GetChildGroups(list);
}


void CGroupInfo::RemoveChildGroup(CGroupInfo* group)
{

}


bool CGroupInfo::AddChildMachine(CAlarmMachine* machine)
{
	if (_id == machine->get_group_id()) {
		_child_machines.push_back(machine);
		UpdateCount();
		return true;
	}

	std::list<CGroupInfo*>::iterator iter = _child_groups.begin();
	while (iter != _child_groups.end()) {
		CGroupInfo* child_group = *iter++;
		if (child_group->AddChildMachine(machine)) {
			return true;
		}
	}
	return false;
}

// 获取儿子主机
void CGroupInfo::GetChildMachines(CAlarmMachineList& list)
{
	std::list<CAlarmMachine*>::iterator iter = _child_machines.begin();
	while (iter != _child_machines.end()) {
		CAlarmMachine* machine = *iter++;
		list.push_back(machine);
	}
}


// 获取所有主机，包括儿子主机与后代主机
void CGroupInfo::GetDescendantMachines(CAlarmMachineList& list)
{
	std::list<CGroupInfo*>::iterator iter = _child_groups.begin();
	while (iter != _child_groups.end()) {
		CGroupInfo* child_group = *iter++;
		child_group->GetDescendantMachines(list);
	}

	GetChildMachines(list);
}


void CGroupInfo::DeleteChildMachine(CAlarmMachine* machine)
{

}


CGroupInfo* CGroupInfo::GetGroupInfo(int group_id)
{
	if (_id == group_id)
		return this;

	std::list<CGroupInfo*>::iterator iter = _child_groups.begin();
	while (iter != _child_groups.end()) {
		CGroupInfo* child_group = *iter++;
		CGroupInfo* target = child_group->GetGroupInfo(group_id);
		if (target) {
			return target;
		}
	}

	return NULL;
}




/*******************CGroupManager************************/

IMPLEMENT_SINGLETON(CGroupManager)
CGroupManager::CGroupManager()
{}


CGroupManager::~CGroupManager()
{}


CGroupInfo* CGroupManager::GetGroupInfo(int group_id)
{
	LOG_FUNCTION_AUTO;
	return _tree.GetGroupInfo(group_id);
}


//void CGroupManager::ResolvGroupList()
//{
//	std::list<CGroupInfo*>::iterator iter = _groupList.begin();
//	while (iter != _groupList.end()) {
//		CGroupInfo* group = *iter;
//		if (group->get_parent_id() == _tree.get_id()) {
//			_tree.AddChildGroup(group);
//			_groupList.erase(iter);
//			iter = _groupList.begin();
//			continue;
//		}
//		iter++;
//	}
//}







NAMESPACE_END
