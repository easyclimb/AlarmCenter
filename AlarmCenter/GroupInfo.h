#pragma once

#include <list>
#include "core.h"

namespace core {

typedef enum sort_machine_way {
	sort_by_ademco_id,
	sort_by_name,
	sort_by_on_offline,
	sort_by_arm_disarm,
	sort_by_event_level,
}sort_machine_way;

inline sort_machine_way Integer2SortMachineWay(int way) {
	switch (way) {
	case core::sort_by_ademco_id:
	default: 
		return sort_by_ademco_id;
		break;
	case core::sort_by_name:return sort_by_name;
		break;
	case core::sort_by_on_offline:return sort_by_on_offline;
		break;
	case core::sort_by_arm_disarm:return sort_by_arm_disarm;
		break;
	case core::sort_by_event_level:return sort_by_event_level;
		break;
	}
}

typedef enum filter_machine_way {
	filter_by_all,
	filter_by_online,
	filter_by_offline,
	filter_by_arm,
	filter_by_disarm,
	filter_by_event,
}filter_machine_way;


class group_info : public std::enable_shared_from_this<group_info>, public dp::observable<int>
{
	// friend class group_manager;
	// friend class alarm_machine_manager;
private:
	int _id;
	int _parent_id;
	CString group_name_;

	//int _child_group_count;
	int _descendant_machine_count;
	int _online_descendant_machine_count;
	int _alarming_descendant_machine_count;

	group_info_weak_ptr _parent_group;
	group_info_list _child_groups;
	alarm_machine_list _child_machines;
	//alarm_machine_list filtered_machines_;

	filter_machine_way cur_filter_way_ = filter_by_all;
protected:
	//void UpdateChildGroupCount(bool bAdd = true);
	void UpdateChildMachineCount(bool bAdd = true);
	//void UpdateFilteredMachines();
public:

	

	group_info();
	~group_info();

	void set_cur_filter_way(filter_machine_way way);
	filter_machine_way get_cur_filter_way() const { return cur_filter_way_; }

	void SortDescendantMachines(sort_machine_way way);
	void SortDescendantGroupsByName();

	void UpdateOnlineDescendantMachineCount(bool bAdd = true);
	void UpdateAlarmingDescendantMachineCount(bool bAdd = true);

	bool IsRootItem() const { return (_parent_group.expired()); }
	bool IsDescendantGroup(const core::group_info_ptr& group);
// protected:
	bool AddChildGroup(const core::group_info_ptr& group);
	bool RemoveChildGroup(const core::group_info_ptr& group);
	void GetChildGroups(group_info_list& list);
	void GetDescendantGroups(group_info_list& list);

	bool AddChildMachine(const core::alarm_machine_ptr& machine);
	bool RemoveChildMachine(const core::alarm_machine_ptr& machine);
	void GetChildMachines(alarm_machine_list& list);
	void GetFilteredChildMachines(alarm_machine_list& list, filter_machine_way filter);
	void GetDescendantMachines(alarm_machine_list& list);
	void GetFilteredDescendantMachines(alarm_machine_list& list, filter_machine_way filter);
	void ClearAlarmMsgOfDescendantAlarmingMachine();

	core::group_info_ptr GetGroupInfo(int group_id);
	core::group_info_ptr GetFirstChildGroupInfo() { return _child_groups.empty() ? nullptr : _child_groups.front(); }

	// really db oper
	core::group_info_ptr ExecuteAddChildGroup(const wchar_t* name);
	BOOL ExecuteRename(const wchar_t* name);
	BOOL ExecuteDeleteChildGroup(const core::group_info_ptr& group);
	BOOL ExecuteMove2Group(const core::group_info_ptr& group);

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



	core::group_info_ptr get_parent_group() const { return _parent_group.lock(); }
	DECLARE_SETTER_NONE_CONST(core::group_info_ptr, _parent_group);

};


class group_manager : public dp::singleton<group_manager>
{
	friend class alarm_machine_manager;
private:
	//std::list<const core::group_info_ptr&> _groupList;
	core::group_info_ptr _tree = std::make_shared<group_info>();
	sort_machine_way cur_sort_machine_way_ = sort_by_ademco_id;

	std::wstring cfg_path_ = L"";
	//std::list<const core::group_info_ptr&> _groupList;
	//std::list<alarm_machine_ptr> _machineList;
	//void ResolvGroupList();

protected:
	group_manager();
public:
	core::group_info_ptr GetRootGroupInfo() { return _tree; }
	core::group_info_ptr GetGroupInfo(int group_id);
	//void SortByName();

protected:
	void init();
	bool load();
	bool save();

public:
	~group_manager();
	sort_machine_way get_cur_sort_machine_way() const { return cur_sort_machine_way_; }
	void set_cur_sort_machine_way(sort_machine_way way);
	//DECLARE_SINGLETON(group_manager)
};



};


