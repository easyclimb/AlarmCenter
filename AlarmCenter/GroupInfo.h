#pragma once

#include <list>
#include "core.h"
#include "observer.h"

namespace core {

typedef void(_stdcall *OnlineCountChangedCB)(void* data, int place_holder);

class CGroupInfo : public std::enable_shared_from_this<CGroupInfo>, public dp::observable<int>
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

	CGroupInfoWeakPtr _parent_group;
	CGroupInfoList _child_groups;
	std::list<CAlarmMachinePtr> _child_machines;
protected:
	void UpdateChildGroupCount(bool bAdd = true);
	void UpdateChildMachineCount(bool bAdd = true);
	
public:
	CGroupInfo();
	~CGroupInfo();

	void UpdateOnlineDescendantMachineCount(bool bAdd = true);

	bool IsRootItem() const { return (_parent_group.expired()); }
	bool IsDescendantGroup(CGroupInfoPtr group);
// protected:
	bool AddChildGroup(CGroupInfoPtr group);
	bool RemoveChildGroup(CGroupInfoPtr group);
	void GetChildGroups(CGroupInfoList& list);
	void GetDescendantGroups(CGroupInfoList& list);

	bool AddChildMachine(CAlarmMachinePtr machine);
	bool RemoveChildMachine(CAlarmMachinePtr machine);
	void GetChildMachines(CAlarmMachineList& list);
	void GetDescendantMachines(CAlarmMachineList& list);
	void ClearAlarmMsgOfDescendantAlarmingMachine();

	CGroupInfoPtr GetGroupInfo(int group_id);
	CGroupInfoPtr GetFirstChildGroupInfo() { return _child_groups.empty() ? nullptr : _child_groups.front(); }

	// really db oper
	CGroupInfoPtr ExecuteAddChildGroup(const wchar_t* name);
	BOOL ExecuteRename(const wchar_t* name);
	BOOL ExecuteDeleteChildGroup(CGroupInfoPtr group);
	BOOL ExecuteMove2Group(CGroupInfoPtr group);

	DECLARE_GETTER_SETTER_INT(_id);
	DECLARE_GETTER_SETTER_INT(_parent_id);
	DECLARE_GETTER_SETTER_INT(_child_group_count);
	DECLARE_GETTER_SETTER_INT(_descendant_machine_count);
	DECLARE_GETTER(int, _online_descendant_machine_count);
	DECLARE_GETTER_SETTER_STRING(_name);
	CGroupInfoPtr get_parent_group() const { return _parent_group.lock(); }
	DECLARE_SETTER_NONE_CONST(CGroupInfoPtr, _parent_group);

};


class CGroupManager 
{
	friend class CAlarmMachineManager;
private:
	//std::list<CGroupInfoPtr> _groupList;
	CGroupInfoPtr _tree = std::make_shared<CGroupInfo>();
	//std::list<CGroupInfoPtr> _groupList;
	//std::list<CAlarmMachinePtr> _machineList;
	//void ResolvGroupList();
public:
	CGroupInfoPtr GetRootGroupInfo() { return _tree; }
	CGroupInfoPtr GetGroupInfo(int group_id);
public:
	~CGroupManager();
	DECLARE_UNCOPYABLE(CGroupManager)
	DECLARE_SINGLETON(CGroupManager)
};




NAMESPACE_END

