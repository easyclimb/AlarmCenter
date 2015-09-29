#include "stdafx.h"
#include "GroupInfo.h"
#include "AlarmMachine.h"
#include "AlarmMachineManager.h"
#include <iterator>

namespace core {

CGroupInfo::CGroupInfo()
	: _id(0), _parent_id(0), _name(NULL)
	, _child_group_count(0)
	, _descendant_machine_count(0)
	, _online_descendant_machine_count(0)
	, _parent_group(NULL)
{
	_name = new wchar_t[1];
	_name[0] = 0;
}


CGroupInfo::~CGroupInfo()
{
	SAFEDELETEARR(_name);

	for (auto child_group : _child_groups) {
		delete child_group;
	}
	_child_groups.clear();
}


void CGroupInfo::UpdateChildGroupCount(bool bAdd)
{
	bAdd ? (_child_group_count++) : (_child_group_count--);
	if (_parent_group) {
		_parent_group->UpdateChildGroupCount(bAdd);
	}
}


void CGroupInfo::UpdateChildMachineCount(bool bAdd)
{
	bAdd ? (_descendant_machine_count++) : (_descendant_machine_count--);
	if (_parent_group) {
		_parent_group->UpdateChildMachineCount(bAdd);
	}
}


bool CGroupInfo::IsDescendantGroup(CGroupInfo* group)
{
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
		UpdateChildGroupCount();
		return true;
	}

	for (auto child_group : _child_groups) {
		if (child_group->AddChildGroup(group)) {
			return true;
		}
	}

	return false;
}


bool CGroupInfo::RemoveChildGroup(CGroupInfo* group)
{
	if (_id == group->get_parent_id()) {
		_child_groups.remove(group);
		_child_group_count--;
		return true;
	}

	for (auto child_group : _child_groups) {
		if (child_group->RemoveChildGroup(group)) {
			return true;
		}
	}

	return false;
}


// 获取所有儿子分组
void CGroupInfo::GetChildGroups(CGroupInfoList& list)
{
	std::copy(_child_groups.begin(), _child_groups.end(), std::back_inserter(list));
}


// 获取所有后代分组(包括儿子分组)
void CGroupInfo::GetDescendantGroups(CGroupInfoList& list)
{
	for (auto child_group : _child_groups) {
		child_group->GetDescendantGroups(list);
	}

	GetChildGroups(list);
}


bool CGroupInfo::AddChildMachine(CAlarmMachine* machine)
{
	AUTO_LOG_FUNCTION;
	if (_id == machine->get_group_id()) {
		_child_machines.push_back(machine);
		UpdateChildMachineCount();
		return true;
	}

	for (auto child_group : _child_groups) {
		if (child_group->AddChildMachine(machine)) {
			return true;
		}
	}
	return false;
}


bool CGroupInfo::RemoveChildMachine(CAlarmMachine* machine)
{
	if (_id == machine->get_group_id()) {
		_child_machines.remove(machine);
		UpdateChildMachineCount(false);
		return true;
	}

	for (auto child_group : _child_groups) {
		if (child_group->RemoveChildMachine(machine)) {
			return true;
		}
	}
	return false;
}


// 获取儿子主机
void CGroupInfo::GetChildMachines(CAlarmMachineList& list)
{
	std::copy(_child_machines.begin(), _child_machines.end(), std::back_inserter(list));
}


// 获取所有主机，包括儿子主机与后代主机
void CGroupInfo::GetDescendantMachines(CAlarmMachineList& list)
{
	for (auto child_group : _child_groups) {
		child_group->GetDescendantMachines(list);
	}

	GetChildMachines(list);
}


CGroupInfo* CGroupInfo::GetGroupInfo(int group_id)
{
	if (_id == group_id)
		return this;

	for (auto child_group : _child_groups) {
		CGroupInfo* target = child_group->GetGroupInfo(group_id);
		if (target) {
			return target;
		}
	}

	return NULL;
}


CGroupInfo* CGroupInfo::ExecuteAddChildGroup(const wchar_t* name)
{
	CAlarmMachineManager* mgr = CAlarmMachineManager::GetInstance();
	CString query;
	query.Format(L"insert into GroupInfo ([parent_id], [group_name]) values(%d,'%s')",
				 _id, name);
	int id = mgr->AddAutoIndexTableReturnID(query);
	if (-1 != id) {
		CGroupInfo* group = new CGroupInfo();
		group->set_id(id);
		group->set_parent_id(_id);
		group->set_parent_group(this);
		group->set_name(name);
		group->set_descendant_machine_count(0);
		_child_groups.push_back(group);
		return group;
	}
	return NULL;
}


BOOL CGroupInfo::ExecuteRename(const wchar_t* name)
{
	AUTO_LOG_FUNCTION;
	CAlarmMachineManager* mgr = CAlarmMachineManager::GetInstance();
	CString query;
	query.Format(L"update GroupInfo set group_name='%s' where id=%d",
				 name, _id);
	if (mgr->ExecuteSql(query)) {
		set_name(name);
		return TRUE;
	}
	return FALSE;
}


BOOL CGroupInfo::ExecuteDeleteChildGroup(CGroupInfo* group)
{
	AUTO_LOG_FUNCTION;
	ASSERT(group);
	CAlarmMachineManager* mgr = CAlarmMachineManager::GetInstance();
	CString query;
	query.Format(L"delete from GroupInfo where id=%d", group->get_id());
	do {
		if (!mgr->ExecuteSql(query))
			break;

		_child_groups.remove(group);
		_child_group_count--;

		// 处置该分组有子分组或子主机的情况
		if (group->get_child_group_count() > 0) {
			query.Format(L"update GroupInfo set parent_id=%d where parent_id=%d", 
						 this->_id, group->get_id());
			if (!mgr->ExecuteSql(query))
				break;

			CGroupInfoList groupList;
			group->GetChildGroups(groupList);
			group->_child_groups.clear();
			for (auto child : groupList) {
				child->set_parent_group(this);
				child->set_parent_id(this->_id);
				_child_groups.push_back(child);
				_child_group_count++;
			}
		}

		if (group->get_descendant_machine_count() > 0) {
			query.Format(L"update AlarmMachine set group_id=%d where group_id=%d",
						 this->_id, group->get_id());
			if (!mgr->ExecuteSql(query))
				break;

			CAlarmMachineList machineList;
			group->GetChildMachines(machineList);
			group->_child_machines.clear();
			for (auto machine : machineList) {
				machine->set_group_id(this->_id);
				_child_machines.push_back(machine);
			}
		}
		
		delete group;
		return TRUE;
	} while (0);
	return FALSE;
}


BOOL CGroupInfo::ExecuteMove2Group(CGroupInfo* group)
{
	AUTO_LOG_FUNCTION;
	ASSERT(group);
	CAlarmMachineManager* mgr = CAlarmMachineManager::GetInstance();
	CString query;
	query.Format(L"update GroupInfo set parent_id=%d where id=%d", group->get_id(), _id);
	do {
		if (!mgr->ExecuteSql(query))
			break;

		CGroupInfo* oldParent = get_parent_group();
		oldParent->RemoveChildGroup(this);
		set_parent_group(group);
		set_parent_id(group->get_id());
		group->AddChildGroup(this);

		return true;
	} while (0);

	return FALSE;
}





/*******************CGroupManager************************/

IMPLEMENT_SINGLETON(CGroupManager)
CGroupManager::CGroupManager()
{}


CGroupManager::~CGroupManager()
{}


CGroupInfo* CGroupManager::GetGroupInfo(int group_id)
{
	AUTO_LOG_FUNCTION;
	return _tree.GetGroupInfo(group_id);
}


NAMESPACE_END
