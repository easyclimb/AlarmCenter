#pragma once

#include <list>
#include "core.h"

namespace core {

class CGroupInfo;
typedef std::list<CGroupInfo*> CGroupInfoList;
typedef void(_stdcall *OnlineCountChangedCB)(void* data, int place_holder);

class CGroupInfo
{
	// friend class CGroupManager;
	// friend class CAlarmMachineManager;
private:
	int _id;
	int _parent_id;
	CString _name;

	int _child_group_count;
	int _descendant_machine_count;
	int _online_descendant_machine_count;

	CGroupInfo* _parent_group;
	std::list<CGroupInfo*> _child_groups;
	std::list<CAlarmMachinePtr> _child_machines;
protected:
	void UpdateChildGroupCount(bool bAdd = true);
	void UpdateChildMachineCount(bool bAdd = true);
	
public:
	CGroupInfo();
	~CGroupInfo();

	void UpdateOnlineDescendantMachineCount(bool bAdd = true);

	bool IsRootItem() const { return (_parent_group == nullptr); }
	bool IsDescendantGroup(CGroupInfo* group);
// protected:
	bool AddChildGroup(CGroupInfo* group);
	bool RemoveChildGroup(CGroupInfo* group);
	void GetChildGroups(CGroupInfoList& list);
	void GetDescendantGroups(CGroupInfoList& list);

	bool AddChildMachine(CAlarmMachinePtr machine);
	bool RemoveChildMachine(CAlarmMachinePtr machine);
	void GetChildMachines(CAlarmMachineList& list);
	void GetDescendantMachines(CAlarmMachineList& list);
	void ClearAlarmMsgOfDescendantAlarmingMachine();

	CGroupInfo* GetGroupInfo(int group_id);
	CGroupInfo* GetFirstChildGroupInfo() { return _child_groups.size() > 0 ? _child_groups.front() : nullptr; }

	// really db oper
	CGroupInfo* ExecuteAddChildGroup(const wchar_t* name);
	BOOL ExecuteRename(const wchar_t* name);
	BOOL ExecuteDeleteChildGroup(CGroupInfo* group);
	BOOL ExecuteMove2Group(CGroupInfo* group);

	DECLARE_GETTER_SETTER_INT(_id);
	DECLARE_GETTER_SETTER_INT(_parent_id);
	DECLARE_GETTER_SETTER_INT(_child_group_count);
	DECLARE_GETTER_SETTER_INT(_descendant_machine_count);
	DECLARE_GETTER(int, _online_descendant_machine_count);
	DECLARE_GETTER_SETTER_STRING(_name);
	DECLARE_GETTER(CGroupInfo*, _parent_group);
	DECLARE_SETTER_NONE_CONST(CGroupInfo*, _parent_group);

	DECLARE_OBSERVER(OnlineCountChangedCB, int)
};


class CGroupManager 
{
	friend class CAlarmMachineManager;
private:
	//std::list<CGroupInfo*> _groupList;
	CGroupInfo _tree;
	//std::list<CGroupInfo*> _groupList;
	//std::list<CAlarmMachinePtr> _machineList;
	//void ResolvGroupList();
public:
	CGroupInfo* GetRootGroupInfo() { return &_tree; }
	CGroupInfo* GetGroupInfo(int group_id);
public:
	~CGroupManager();
	DECLARE_UNCOPYABLE(CGroupManager)
	DECLARE_SINGLETON(CGroupManager)
};




NAMESPACE_END

