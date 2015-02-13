#pragma once

#include <list>

namespace core {

class CGroupInfo;
class CAlarmMachine;

typedef std::list<CGroupInfo*> CCGroupInfoList;
typedef std::list<CGroupInfo*>::iterator CGroupInfoListIter;

typedef std::list<CAlarmMachine*> CAlarmMachineList;
typedef std::list<CAlarmMachine*>::iterator CAlarmMachineListIter;

class CGroupInfo
{
	// friend class CGroupManager;
	// friend class CAlarmMachineManager;
private:
	int _id;
	int _parent_id;
	wchar_t* _name;

	int _machine_count;

	CGroupInfo* _parent_group;
	std::list<CGroupInfo*> _child_groups;
	std::list<CAlarmMachine*> _child_machines;
protected:
	void UpdateCount(bool bAdd = true);
public:
	CGroupInfo();
	~CGroupInfo();

	bool IsRootItem() const { return (_parent_group == NULL); }
	bool IsDescendantGroup(CGroupInfo* group);
// protected:
	bool AddChildGroup(CGroupInfo* group);
	void RemoveChildGroup(CGroupInfo* group);
	void GetChildGroups(CCGroupInfoList& list);
	void GetDescendantGroups(CCGroupInfoList& list);

	bool AddChildMachine(CAlarmMachine* machine);
	void DeleteChildMachine(CAlarmMachine* machine);
	void GetChildMachines(CAlarmMachineList& list);
	void GetDescendantMachines(CAlarmMachineList& list);

	CGroupInfo* GetGroupInfo(int group_id);
	CGroupInfo* GetParentGroupInfo() const { LOG_FUNCTION_AUTO; return _parent_group; }

	// really db oper
	CGroupInfo* ExecuteAddChildGroup(const wchar_t* name);
	BOOL ExecuteRename(const wchar_t* name);
	BOOL ExecuteDeleteChildGroup(CGroupInfo* group);
	BOOL ExecuteMove2Group(CGroupInfo* group);

	DEALARE_GETTER_SETTER_INT(_id);
	DEALARE_GETTER_SETTER_INT(_parent_id);
	DEALARE_GETTER_SETTER_INT(_machine_count);
	DECLARE_GETTER_SETTER_STRING(_name);
	DEALARE_GETTER_SETTER(CGroupInfo*, _parent_group);
};


class CGroupManager 
{
	friend class CAlarmMachineManager;
private:
	//std::list<CGroupInfo*> _groupList;
	CGroupInfo _tree;
	//std::list<CGroupInfo*> _groupList;
	//std::list<CAlarmMachine*> _machineList;
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

