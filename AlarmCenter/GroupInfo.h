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
	CString group_name_;

	//int _child_group_count;
	int _descendant_machine_count;
	int _online_descendant_machine_count;
	int _alarming_descendant_machine_count;

	CGroupInfoWeakPtr _parent_group;
	CGroupInfoList _child_groups;
	std::list<CAlarmMachinePtr> _child_machines;
protected:
	//void UpdateChildGroupCount(bool bAdd = true);
	void UpdateChildMachineCount(bool bAdd = true);
	
public:
	CGroupInfo();
	~CGroupInfo();

	void SortDescendantGroupsByName();

	void UpdateOnlineDescendantMachineCount(bool bAdd = true);
	void UpdateAlarmingDescendantMachineCount(bool bAdd = true);

	bool IsRootItem() const { return (_parent_group.expired()); }
	bool IsDescendantGroup(const core::CGroupInfoPtr& group);
// protected:
	bool AddChildGroup(const core::CGroupInfoPtr& group);
	bool RemoveChildGroup(const core::CGroupInfoPtr& group);
	void GetChildGroups(CGroupInfoList& list);
	void GetDescendantGroups(CGroupInfoList& list);

	bool AddChildMachine(const core::CAlarmMachinePtr& machine);
	bool RemoveChildMachine(const core::CAlarmMachinePtr& machine);
	void GetChildMachines(CAlarmMachineList& list);
	void GetDescendantMachines(CAlarmMachineList& list);
	void ClearAlarmMsgOfDescendantAlarmingMachine();

	core::CGroupInfoPtr GetGroupInfo(int group_id);
	core::CGroupInfoPtr GetFirstChildGroupInfo() { return _child_groups.empty() ? nullptr : _child_groups.front(); }

	// really db oper
	core::CGroupInfoPtr ExecuteAddChildGroup(const wchar_t* name);
	BOOL ExecuteRename(const wchar_t* name);
	BOOL ExecuteDeleteChildGroup(const core::CGroupInfoPtr& group);
	BOOL ExecuteMove2Group(const core::CGroupInfoPtr& group);

	int get_child_group_count() const { return _child_groups.size(); }

	DECLARE_GETTER_SETTER_INT(_id);
	DECLARE_GETTER_SETTER_INT(_parent_id);
	//DECLARE_GETTER_SETTER_INT(_child_group_count);
	DECLARE_GETTER_SETTER_INT(_descendant_machine_count);
	DECLARE_GETTER_SETTER_INT(_online_descendant_machine_count);
	DECLARE_GETTER_SETTER_INT(_alarming_descendant_machine_count);
	//DECLARE_GETTER_SETTER_STRING(_name);
	CString get_group_name() const { return group_name_; }
	void set_group_name(const CString& name) { group_name_ = name; }
	CString get_formatted_group_name() const { CString txt; txt.Format(L"(%d)%s", _id, group_name_); return txt; }

	core::CGroupInfoPtr get_parent_group() const { return _parent_group.lock(); }
	DECLARE_SETTER_NONE_CONST(core::CGroupInfoPtr, _parent_group);

};


class CGroupManager 
{
	friend class CAlarmMachineManager;
private:
	//std::list<const core::CGroupInfoPtr&> _groupList;
	core::CGroupInfoPtr _tree = std::make_shared<CGroupInfo>();
	//std::list<const core::CGroupInfoPtr&> _groupList;
	//std::list<CAlarmMachinePtr> _machineList;
	//void ResolvGroupList();
public:
	core::CGroupInfoPtr GetRootGroupInfo() { return _tree; }
	core::CGroupInfoPtr GetGroupInfo(int group_id);
	//void SortByName();
public:
	~CGroupManager();
	DECLARE_UNCOPYABLE(CGroupManager)
	DECLARE_SINGLETON(CGroupManager)
};




NAMESPACE_END

