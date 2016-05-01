#include "stdafx.h"
#include "GroupInfo.h"
#include "AlarmMachine.h"
#include "AlarmMachineManager.h"
#include <iterator>
#include "json/json.h"
#include <fstream>

namespace core {

	namespace detail {
		const char* sectionSortDescendantMachineWay = "sortDescendantMachineWay";

		auto sort_machine_list = [](CAlarmMachineList& list, sort_machine_way way) {
			auto cmp_func = [&way](const CAlarmMachinePtr& m1, const CAlarmMachinePtr& m2) {
				switch (way) {
				case core::sort_by_name:
					return m1->get_machine_name().CompareNoCase(m2->get_machine_name()) <= 0;
					break;
				case core::sort_by_on_offline:
					if (m1->get_online() == m2->get_online()) {
						return m1->get_ademco_id() < m2->get_ademco_id();
					} else if (m1->get_online()) {
						return true;
					} else {
						return false;
					}
					break;
				case core::sort_by_arm_disarm:
					if (m1->get_machine_status() == m2->get_machine_status()) {
						return m1->get_ademco_id() < m2->get_ademco_id();
					} else {
						return m1->get_machine_status() < m2->get_machine_status();
					}
					break;
				case core::sort_by_event_level:
					if (m1->get_highestEventLevel() == m2->get_highestEventLevel()) {
						return m1->get_ademco_id() < m2->get_ademco_id();
					} else {
						return m1->get_highestEventLevel() > m2->get_highestEventLevel();
					}
					break;

				case core::sort_by_ademco_id:
				default:
					return m1->get_ademco_id() < m2->get_ademco_id();
					break;
				}

			};

			list.sort(cmp_func);
		};


		auto filter_machine_list = [](CAlarmMachineList& src, CAlarmMachineList& dst, filter_machine_way way) {
			dst.clear();
			
			switch (way) {
			case core::filter_by_all:
				std::copy(src.begin(), src.end(), std::back_inserter(dst));
				break;
			case core::filter_by_online:
				for (auto machine : src) {
					if (machine->get_online()) {
						dst.push_back(machine);
					}
				}
				break;
			case core::filter_by_offline:
				for (auto machine : src) {
					if (!machine->get_online()) {
						dst.push_back(machine);
					}
				}
				break;
			case core::filter_by_arm:
				for (auto machine : src) {
					if (machine->get_machine_status() == core::MACHINE_ARM || machine->get_machine_status() == core::MACHINE_HALFARM) {
						dst.push_back(machine);
					}
				}
				break;
			case core::filter_by_disarm:
				for (auto machine : src) {
					if (machine->get_machine_status() == core::MACHINE_DISARM) {
						dst.push_back(machine);
					}
				}
				break;
			case core::filter_by_event:
				for (auto machine : src) {
					if (machine->get_highestEventLevel() > EVENT_LEVEL_STATUS) {
						dst.push_back(machine);
					}
				}
				break;
			default:
				break;
			}

			
		};
	};

	using namespace detail;

//IMPLEMENT_OBSERVER(CGroupInfo)

CGroupInfo::CGroupInfo()
	: _id(0)
	, _parent_id(0)
	, group_name_()
	//, _child_group_count(0)
	, _descendant_machine_count(0)
	, _online_descendant_machine_count(0)
	, _alarming_descendant_machine_count(0)
	, _parent_group()
{	
}


CGroupInfo::~CGroupInfo()
{
	_child_groups.clear();
}

//
//void CGroupInfo::UpdateChildGroupCount(bool bAdd)
//{
//	bAdd ? (_child_group_count++) : (_child_group_count--);
//	if (!_parent_group.expired()) {
//		_parent_group.lock()->UpdateChildGroupCount(bAdd);
//	}
//}


void CGroupInfo::UpdateChildMachineCount(bool bAdd)
{
	bAdd ? (_descendant_machine_count++) : (_descendant_machine_count--);
	if (!_parent_group.expired()) {
		_parent_group.lock()->UpdateChildMachineCount(bAdd);
	}
}


void CGroupInfo::UpdateOnlineDescendantMachineCount(bool bAdd)
{
	bAdd ? (_online_descendant_machine_count++) : (_online_descendant_machine_count--);
	if (!_parent_group.expired()) {
		_parent_group.lock()->UpdateOnlineDescendantMachineCount(bAdd);
	} else {
		//NotifyObservers(_online_descendant_machine_count); // only root group can call it.
		notify_observers(_online_descendant_machine_count);
	}
}


void CGroupInfo::UpdateAlarmingDescendantMachineCount(bool bAdd)
{
	bAdd ? (_alarming_descendant_machine_count++) : (_alarming_descendant_machine_count--);
	if (!_parent_group.expired()) {
		_parent_group.lock()->UpdateAlarmingDescendantMachineCount(bAdd);
	} else {
		//NotifyObservers(_online_descendant_machine_count); // only root group can call it.
		notify_observers(_alarming_descendant_machine_count);
	}
}


bool CGroupInfo::IsDescendantGroup(const core::CGroupInfoPtr& group)
{
	core::CGroupInfoPtr parent_group = group->get_parent_group();
	while (parent_group) {
		if (parent_group.get() == this) { return true; }
		parent_group = parent_group->get_parent_group();
	}

	return false;
}


bool CGroupInfo::AddChildGroup(const core::CGroupInfoPtr& group)
{
	if (_id == group->get_parent_id()) {
		if (group->get_descendant_machine_count() > 0) {
			_descendant_machine_count += group->get_descendant_machine_count();
		}
		bool b_need_notify_observers = false;
		if (group->get_online_descendant_machine_count() > 0) {
			if (!IsDescendantGroup(group)) {
				_online_descendant_machine_count += group->get_online_descendant_machine_count();
				if (_parent_group.expired()) { // root
					b_need_notify_observers = true;
				}
			}
		}
		if (group->get_alarming_descendant_machine_count() > 0) {
			if (!IsDescendantGroup(group)) {
				_alarming_descendant_machine_count += group->get_alarming_descendant_machine_count();
				if (_parent_group.expired()) { // root
					b_need_notify_observers = true;
				}
			}
		}
		if (b_need_notify_observers) {
			notify_observers(0);
		}
		
		group->set_parent_group(shared_from_this());
		_child_groups.push_back(group);
		//UpdateChildGroupCount();
		return true;
	}

	for (auto child_group : _child_groups) {
		if (child_group->AddChildGroup(group)) {
			return true;
		}
	}

	return false;
}


bool CGroupInfo::RemoveChildGroup(const core::CGroupInfoPtr& group)
{
	if (_id == group->get_parent_id()) {
		_child_groups.remove(group);
		//_child_group_count--;
		if (group->get_descendant_machine_count() > 0) {
			_descendant_machine_count -= group->get_descendant_machine_count();
		}
		if (group->get_online_descendant_machine_count() > 0) {
			if (!_parent_group.expired()) { // not root
				_online_descendant_machine_count -= group->get_online_descendant_machine_count();
			}
		}
		if (group->get_alarming_descendant_machine_count() > 0) {
			if (!_parent_group.expired()) { // not root
				_alarming_descendant_machine_count -= group->get_alarming_descendant_machine_count();
			}
		}
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


bool CGroupInfo::AddChildMachine(const core::CAlarmMachinePtr& machine)
{
	AUTO_LOG_FUNCTION;
	if (_id == machine->get_group_id()) {
		_child_machines.push_back(machine);
		filter_machine_list(_child_machines, filtered_machines_, cur_filter_way_);
		UpdateChildMachineCount();
		if (machine->get_online())
			UpdateOnlineDescendantMachineCount();
		return true;
	}

	for (auto child_group : _child_groups) {
		if (child_group->AddChildMachine(machine)) {
			return true;
		}
	}
	return false;
}


bool CGroupInfo::RemoveChildMachine(const core::CAlarmMachinePtr& machine)
{
	if (_id == machine->get_group_id()) {
		_child_machines.remove(machine);
		filter_machine_list(_child_machines, filtered_machines_, cur_filter_way_);
		UpdateChildMachineCount(false);
		if (machine->get_online())
			UpdateOnlineDescendantMachineCount(false);
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


void CGroupInfo::GetFilteredChildMachines(CAlarmMachineList& list)
{
	std::copy(filtered_machines_.begin(), filtered_machines_.end(), std::back_inserter(list));
}


// 获取所有主机，包括儿子主机与后代主机
void CGroupInfo::GetDescendantMachines(CAlarmMachineList& list)
{
	for (auto child_group : _child_groups) {
		child_group->GetDescendantMachines(list);
	}

	GetChildMachines(list);

	sort_machine_list(list, CGroupManager::GetInstance()->get_cur_sort_machine_way());
}


void CGroupInfo::GetFilteredDescendantMachines(CAlarmMachineList& list)
{
	for (auto child_group : _child_groups) {
		child_group->GetFilteredDescendantMachines(list);
	}

	GetFilteredChildMachines(list);

	sort_machine_list(list, CGroupManager::GetInstance()->get_cur_sort_machine_way());
}


//void CGroupInfo::UpdateFilteredMachines()
//{
//	filter_machine_list(_child_machines, filtered_machines_, cur_filter_way_);
//}


void CGroupInfo::ClearAlarmMsgOfDescendantAlarmingMachine()
{
	for (auto child_group : _child_groups) {
		child_group->ClearAlarmMsgOfDescendantAlarmingMachine();
	}

	for (auto child_machine : _child_machines) {
		if (child_machine->get_alarming()) {
			child_machine->clear_ademco_event_list();
		}
	}

	//if (_parent_group.expired()) { // root
	//	notify_observers(0);
	//}
}


core::CGroupInfoPtr CGroupInfo::GetGroupInfo(int group_id)
{
	if (_id == group_id)
		return shared_from_this();

	for (auto child_group : _child_groups) {
		const core::CGroupInfoPtr& target = child_group->GetGroupInfo(group_id);
		if (target) {
			return target;
		}
	}

	return nullptr;
}


core::CGroupInfoPtr CGroupInfo::ExecuteAddChildGroup(const wchar_t* name)
{
	CAlarmMachineManager* mgr = CAlarmMachineManager::GetInstance();
	CString query;
	query.Format(L"insert into GroupInfo ([parent_id], [group_name]) values(%d,'%s')",
				 _id, name);
	int id = mgr->AddAutoIndexTableReturnID(query);
	if (-1 != id) {
		auto group = std::make_shared<CGroupInfo>();
		group->set_id(id);
		group->set_parent_id(_id);
		group->set_parent_group(shared_from_this());
		group->set_group_name(name);
		group->set_descendant_machine_count(0);
		_child_groups.push_back(group);
		return group;
	}
	return nullptr;
}


BOOL CGroupInfo::ExecuteRename(const wchar_t* name)
{
	AUTO_LOG_FUNCTION;
	CAlarmMachineManager* mgr = CAlarmMachineManager::GetInstance();
	CString query;
	query.Format(L"update GroupInfo set group_name='%s' where id=%d",
				 name, _id);
	if (mgr->ExecuteSql(query)) {
		set_group_name(name);
		return TRUE;
	}
	return FALSE;
}


BOOL CGroupInfo::ExecuteDeleteChildGroup(const core::CGroupInfoPtr& group)
{
	AUTO_LOG_FUNCTION;
	ASSERT(group);
	CAlarmMachineManager* mgr = CAlarmMachineManager::GetInstance();
	CString query;
	query.Format(L"delete from GroupInfo where id=%d", group->get_id());
	do {
		if (!mgr->ExecuteSql(query))
			break;

		auto dummy = group; // aquire a use count
		_child_groups.remove(group);
		//_child_group_count--;
		bool b_need_notify_observers = false;
		if (group->get_online_descendant_machine_count() > 0) {
			_online_descendant_machine_count -= group->get_online_descendant_machine_count();
			if (_parent_group.expired()) { // root
				b_need_notify_observers = true;
			}
		}
		if (group->get_alarming_descendant_machine_count() > 0) {
			_alarming_descendant_machine_count -= group->get_alarming_descendant_machine_count();
			if (_parent_group.expired()) { // root
				b_need_notify_observers = true;
			}
		}
		if (b_need_notify_observers) {
			notify_observers(0);
		}

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
				child->set_parent_group(shared_from_this());
				child->set_parent_id(this->_id);
				_child_groups.push_back(child);
				//_child_group_count++;
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
			filter_machine_list(_child_machines, filtered_machines_, cur_filter_way_);
		}
		
		dummy = nullptr; // release the use count, actually it will cause real release
		return TRUE;
	} while (0);
	return FALSE;
}


BOOL CGroupInfo::ExecuteMove2Group(const core::CGroupInfoPtr& group)
{
	AUTO_LOG_FUNCTION;
	ASSERT(group);
	CAlarmMachineManager* mgr = CAlarmMachineManager::GetInstance();
	CString query;
	query.Format(L"update GroupInfo set parent_id=%d where id=%d", group->get_id(), _id);
	do {
		if (!mgr->ExecuteSql(query))
			break;

		const core::CGroupInfoPtr& oldParent = get_parent_group();
		oldParent->RemoveChildGroup(shared_from_this());
		set_parent_group(group);
		set_parent_id(group->get_id());
		group->AddChildGroup(shared_from_this());

		return true;
	} while (0);

	return FALSE;
}


void CGroupInfo::SortDescendantGroupsByName()
{
	for (auto child_group : _child_groups) {
		child_group->SortDescendantGroupsByName();
	}

	auto cmp_func = [](const CGroupInfoPtr& g1, const CGroupInfoPtr& g2) {
		return g1->get_group_name().CompareNoCase(g2->get_group_name()) <= 0;
	};
	_child_groups.sort(cmp_func);
}


void CGroupInfo::SortDescendantMachines(sort_machine_way way)
{
	for (auto child_group : _child_groups) {
		child_group->SortDescendantMachines(way);
	}
	
	sort_machine_list(_child_machines, way);
	filter_machine_list(_child_machines, filtered_machines_, cur_filter_way_);
}


void CGroupInfo::set_cur_filter_way(filter_machine_way filter)
{
	if (cur_filter_way_ != filter) {
		cur_filter_way_ = filter;
		filter_machine_list(_child_machines, filtered_machines_, filter);

		for (auto child_group : _child_groups) {
			child_group->set_cur_filter_way(filter);
		}
	}
}


/*******************CGroupManager************************/




IMPLEMENT_SINGLETON(CGroupManager)
CGroupManager::CGroupManager() 
{
	cfg_path_ = get_exe_path();
	cfg_path_ += L"\\data\\config\\group.json";
	load();
}


CGroupManager::~CGroupManager()
{}


core::CGroupInfoPtr CGroupManager::GetGroupInfo(int group_id)
{
	AUTO_LOG_FUNCTION;
	return _tree->GetGroupInfo(group_id);
}


void CGroupManager::set_cur_sort_machine_way(sort_machine_way way)
{
	if (cur_sort_machine_way_ != way) {
		cur_sort_machine_way_ = way;
		save();
	}
	_tree->SortDescendantMachines(cur_sort_machine_way_);
}


void CGroupManager::init()
{
	cur_sort_machine_way_ = sort_by_ademco_id;
}


bool CGroupManager::load()
{
	using namespace Json;
	do {
		std::ifstream in(cfg_path_); if (!in) break;
		Reader reader;
		Value value;
		if (!reader.parse(in, value)) break;

		auto way = value[sectionSortDescendantMachineWay].asUInt();
		cur_sort_machine_way_ = Integer2SortMachineWay(way);

	} while (false);

	return save();
}


bool CGroupManager::save()
{
	using namespace detail;
	std::ofstream out(cfg_path_); if (!out)return false;
	Json::Value value;

	value[sectionSortDescendantMachineWay] = cur_sort_machine_way_;

	Json::StyledWriter writer;
	out << writer.write(value);
	out.close();

	return true;
}




NAMESPACE_END
