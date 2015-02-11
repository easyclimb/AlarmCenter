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
// protected:
	bool AddChildGroup(CGroupInfo* group);
	void RemoveChildGroup(CGroupInfo* group);
	void GetChildGroups(CCGroupInfoList& list);

	bool AddChildMachine(CAlarmMachine* machine);
	void DeleteChildMachine(CAlarmMachine* machine);
	void GetChildMachines(CAlarmMachineList& list);
	void GetAllChildMachines(CAlarmMachineList& list);

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
public:
	~CGroupManager();
	DECLARE_UNCOPYABLE(CGroupManager)
	DECLARE_SINGLETON(CGroupManager)
};




NAMESPACE_END

