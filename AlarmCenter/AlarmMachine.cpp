#include "stdafx.h"
#include "AlarmCenter.h"
#include "AlarmMachine.h"
#include "ZoneInfo.h"
#include "DetectorInfo.h"
#include "MapInfo.h"
#include "ademco_event.h"
#include "HistoryRecord.h"
#include "AppResource.h"
#include "UserInfo.h"
#include "AlarmMachineManager.h"
#include "GroupInfo.h"
//#include "SubMachineInfo.h"
#include "SoundPlayer.h"
#include <algorithm>
#include <iterator>
#include <fstream>
#include "Gsm.h"
#include "BaiduMapViewerDlg.h"
#include "CameraInfo.h"
#include "AlarmCenterDlg.h"
//#include "DbOper.h"
#include "sqlitecpp/SQLiteCpp.h"
#include "json/json.h"
#include "ConfigHelper.h"

using namespace ademco;
namespace core {

namespace detail
{
#ifdef _DEBUG
static const int CHECK_EXPIRE_GAP_TIME = 6 * 1000; // check machine if expire in every 6 seconds.
#else
static const int CHECK_EXPIRE_GAP_TIME = 60 * 1000; // check machine if expire in every minutes.
#endif


}

using namespace detail;

///////////////////////////// consumer_manager implement //////////////////////////

//IMPLEMENT_OBSERVER(alarm_machine)
IMPLEMENT_SINGLETON(consumer_manager)

consumer_manager::consumer_manager()
{
	AUTO_LOG_FUNCTION;
	//USES_CONVERSION;
	using namespace SQLite;
	auto path = get_config_path() + "\\service.db3";
	db_ = std::make_shared<Database>(path, SQLITE_OPEN_READWRITE | SQLITE_OPEN_CREATE);
	assert(db_);
	if (!db_) { return; }

	try {
		// check if db empty
		{
			Statement query(*db_, "select name from sqlite_master where type='table'");
			if (!query.executeStep()) {
				// init tables
				db_->exec("drop table if exists table_consumer_type");
				db_->exec("drop table if exists consumers");
				db_->exec("create table table_consumer_type (id integer primary key AUTOINCREMENT, type_name text)");
				db_->exec("create table table_consumers (id integer primary key AUTOINCREMENT, ademco_id integer, zone_value integer, type_id integer, receivable_amount integer, paid_amount integer, remind_time text)");

				// init some default consumer types
				CString sql;
				sql.Format(L"insert into table_consumer_type values(NULL, \"%s\")", GetStringFromAppResource(IDS_STRING_CONSUMER_T_HOME));
				db_->exec(utf8::w2a((LPCTSTR)sql));
				sql.Format(L"insert into table_consumer_type values(NULL, \"%s\")", GetStringFromAppResource(IDS_STRING_CONSUMER_T_SHOP));
				db_->exec(utf8::w2a((LPCTSTR)sql));
				sql.Format(L"insert into table_consumer_type values(NULL, \"%s\")", GetStringFromAppResource(IDS_STRING_CONSUMER_T_OFFICE));
				db_->exec(utf8::w2a((LPCTSTR)sql));

			} else {
				std::string name = query.getColumn(0);
				JLOGA(name.c_str());
				while (query.executeStep()) {
					name = query.getColumn(0).getText();
					JLOGA(name.c_str());
				}
			}
		}

		Statement query(*db_, "select * from table_consumer_type");

		while (query.executeStep()) {
			int id = static_cast<int>(query.getColumn(0));
			const char* name = query.getColumn(1);
			auto wname = utf8::a2w(name);
			add_type(id, wname.c_str());
		}
	} catch (std::exception& e) {
		JLOGA(e.what());
	}
}


consumer_list consumer_manager::load_consumers() const
{
	using namespace SQLite;
	consumer_list list;
	Statement query(*db_, "select * from table_consumers");

	int id, ademco_id, zone_value, type_id, receivable_amount, paid_amount;
	std::string remind_time;
	while (query.executeStep()) {
		int ndx = 0;
		id = query.getColumn(ndx++);
		ademco_id = query.getColumn(ndx++);
		zone_value = query.getColumn(ndx++);
		type_id = query.getColumn(ndx++);
		receivable_amount = query.getColumn(ndx++);
		paid_amount = query.getColumn(ndx++);
		remind_time = query.getColumn(ndx++).getText();
		auto consumer_type = get_consumer_type_by_id(type_id);
		if (consumer_type) {
			auto a_consumer = std::make_shared<consumer>(id, ademco_id, zone_value, consumer_type, receivable_amount, paid_amount,
														 string_to_time_point(remind_time));
			list.push_back(a_consumer);
		}
	}		

	return list;
}


consumer_ptr consumer_manager::execute_add_consumer(int ademco_id, int zone_value, const consumer_type_ptr& type,
													int receivable_amount, int paid_amount, const std::chrono::system_clock::time_point& remind_time)
{
	using namespace SQLite;
	assert(type); if (!type) {
		return nullptr;
	}
	CString sql; 
	sql.Format(L"insert into table_consumers ([ademco_id],[zone_value],[type_id],[receivable_amount],[paid_amount],[remind_time]) values(%d,%d,%d,%d,%d,'%s')",
			   ademco_id, zone_value, type->id, receivable_amount, paid_amount, time_point_to_wstring(remind_time).c_str());
	db_->exec(utf8::w2a((LPCTSTR)sql)); 
	int id =static_cast<int>(db_->getLastInsertRowid());
	return std::make_shared<consumer>(id, ademco_id, zone_value, type, receivable_amount, paid_amount, remind_time);
}


bool consumer_manager::execute_delete_consumer(const consumer_ptr& consumer)
{
	CString sql;
	sql.Format(L"delete from table_consumers where id=%d", consumer->id);
	if (db_->exec(utf8::w2a((LPCTSTR)sql)) > 0) {
		return true;
	}
	return false;
}


bool consumer_manager::execute_update_consumer(const consumer_ptr& consumer)
{
	CString sql;
	sql.Format(L"update table_consumers set type_id=%d,receivable_amount=%d,paid_amount=%d,remind_time='%s' where id=%d", 
			   consumer->type->id, consumer->receivable_amount, consumer->paid_amount, 
			   time_point_to_wstring(consumer->remind_time).c_str(), consumer->id);
	return db_->exec(utf8::w2a((LPCTSTR)sql)) > 0;
}


consumer_manager::~consumer_manager()
{
	
}


bool consumer_manager::execute_add_type(int& id, const CString& type_name)
{
	CString sql;
	sql.Format(L"insert into table_consumer_type ([type_name]) values('%s')", type_name);
	db_->exec(utf8::w2a((LPCTSTR)sql));
	id = static_cast<int>(db_->getLastInsertRowid());
	add_type(id, (LPCTSTR)type_name);
	return true;
	
}


bool consumer_manager::execute_rename(int id, const CString& new_name)
{
	CString sql;
	sql.Format(L"update table_consumer_type set type_name='%s' where id=%d", new_name, id);
	if (db_->exec(utf8::w2a((LPCTSTR)sql)) == 1) {
		consumer_type_map_[id]->name = (LPCTSTR)new_name;
		return true;
	}

	return false;
}
/////////////////// end of consumer / consumer_manager implement ////////////////////

/////////////////// alarm machine implement /////////////////////////////////////////
	
alarm_machine::alarm_machine()
	: _id(0)
	, _ademco_id(0)
	, _group_id(0)
	, _machine_status(MACHINE_STATUS_UNKNOWN)
	, _alarming(false)
	, _has_alarming_direct_zone(false)
	, _buffer_mode(false)
	, _is_submachine(false)
	, _submachine_zone(0)
	, _submachine_count(0)
	, _unbindZoneMap(nullptr)
	, _highestEventLevel(EVENT_LEVEL_STATUS)
	, _alarmingSubMachineCount(0)
	, _lastActionTime(time(nullptr))
	, _bChecking(false)
	, expire_time_()
	, _last_time_check_if_expire(0)
	, _coor()
	, _zoomLevel(14)
	, _auto_show_map_when_start_alarming(true)
	, _privatePacket(nullptr)
{
	memset(_ipv4, 0, sizeof(_ipv4));

	_unbindZoneMap = std::make_shared<map_info>();
	_unbindZoneMap->set_id(-1);
	CString fmAlias;
	fmAlias = GetStringFromAppResource(IDS_STRING_NOZONEMAP);
	_unbindZoneMap->set_alias(fmAlias);

}


alarm_machine::~alarm_machine()
{
	auto t = time(nullptr);
	auto ademcoEvent = std::make_shared<AdemcoEvent>(ES_UNKNOWN, EVENT_IM_GONNA_DIE, 0, 0, t, t);
	notify_observers(ademcoEvent);
}


void alarm_machine::SetPrivatePacket(const ademco::PrivatePacket* privatePacket)
{
	if (privatePacket == nullptr) {
		_privatePacket = nullptr;;
		return;
	}

	if (_privatePacket == nullptr)
		_privatePacket = std::make_shared<PrivatePacket>();
	_privatePacket->Copy(privatePacket);
}


const ademco::PrivatePacketPtr alarm_machine::GetPrivatePacket() const
{
	return _privatePacket;
}


bool alarm_machine::execute_set_auto_show_map_when_start_alarming(bool b)
{
	auto mgr = alarm_machine_manager::GetInstance();
	CString sql;
	if (_is_submachine) {
		sql.Format(L"update table_sub_machine set auto_show_map_when_alarm=%d where id=%d", b, _id);
	} else {
		sql.Format(L"update table_machine set auto_show_map_when_alarm=%d where id=%d", b, _id);
	}

	if (mgr->ExecuteSql(sql)) {
		_auto_show_map_when_start_alarming = b;
		return true;
	}
	return false;
}


bool alarm_machine::execute_set_zoomLevel(int zoomLevel)
{
	if (19 < zoomLevel || zoomLevel < 1) zoomLevel = 14;
	auto mgr = alarm_machine_manager::GetInstance();
	CString sql;
	if (_is_submachine) {
		sql.Format(L"update table_sub_machine set map_zoom_level=%d where id=%d", zoomLevel, _id);
	} else {
		sql.Format(L"update table_machine set map_zoom_level=%d where id=%d", zoomLevel, _id);
	}

	if (mgr->ExecuteSql(sql)) {
		_zoomLevel = zoomLevel;
		return true;
	}
	return false;
}


void alarm_machine::clear_ademco_event_list()
{
	if (!_alarming) return;
	std::lock_guard<std::recursive_mutex> lock(_lock4AdemcoEventList);
	_alarming = false;
	_has_alarming_direct_zone = false;
	_highestEventLevel = EVENT_LEVEL_STATUS;
	_alarmingSubMachineCount = 0;
	_ademcoEventList.clear();

	auto t = time(nullptr);
	auto ademcoEvent = std::make_shared<AdemcoEvent>(ES_UNKNOWN, EVENT_CLEARMSG, 0, 0, t, t);
	notify_observers(ademcoEvent);
	if (_unbindZoneMap) {
		_unbindZoneMap->InversionControl(ICMC_CLR_ALARM_TEXT);
	}

	auto zoneIter = _zoneMap.begin();
	while (zoneIter != _zoneMap.end()) {
		if (!zoneIter->second) {
			zoneIter = _zoneMap.erase(zoneIter);
			continue;
		}
		map_info_ptr mapInfo = zoneIter->second->GetMapInfo();
		if (mapInfo.get()) {
			mapInfo->InversionControl(ICMC_CLR_ALARM_TEXT);
		}
		if (zoneIter->second->get_type() == ZT_SUB_MACHINE) {
			alarm_machine_ptr subMachine = zoneIter->second->GetSubMachineInfo();
			if (subMachine && subMachine->get_alarming()) {
				subMachine->clear_ademco_event_list();
			}
		}
		zoneIter->second->HandleAdemcoEvent(ademcoEvent);
		zoneIter++;
	}

	for (auto mapInfo : _mapList) {
		mapInfo->InversionControl(ICMC_CLR_ALARM_TEXT);
	}

	// add a record
	CString srecord, suser, sfm, sop, spost;
	suser = GetStringFromAppResource(IDS_STRING_USER);
	sfm = GetStringFromAppResource(IDS_STRING_LOCAL_OP);
	sop = GetStringFromAppResource(IDS_STRING_CLR_MSG);
	auto user = user_manager::GetInstance()->GetCurUserInfo();
	srecord.Format(L"%s(ID:%d,%s)%s:%s", suser,
				   user->get_user_id(), user->get_user_name(),
				   sfm, sop);
	if (_is_submachine) {
		alarm_machine_manager* mgr = alarm_machine_manager::GetInstance();
		alarm_machine_ptr parent_machine = mgr->GetMachine(_ademco_id);
		if (parent_machine) {
			parent_machine->dec_alarmingSubMachineCount();
		}
	}

	spost = get_formatted_name();
	srecord += spost;
	history_record_manager::GetInstance()->InsertRecord(get_ademco_id(),
												_is_submachine ? _submachine_zone : 0,
												srecord, time(nullptr),
												RECORD_LEVEL_USERCONTROL);

	if (!_is_submachine) {
		group_manager* groupMgr = group_manager::GetInstance();
		group_info_ptr group = groupMgr->GetGroupInfo(_group_id);
		group->UpdateAlarmingDescendantMachineCount(false);

		CWinApp* app = AfxGetApp(); ASSERT(app);
		auto wnd = static_cast<CAlarmCenterDlg*>(app->GetMainWnd()); ASSERT(wnd);
		wnd->MachineDisalarm(shared_from_this());
	}
}


bool alarm_machine::EnterBufferMode()
{ 
	_ootebmOjb.call();
	if (_lock4AdemcoEventList.try_lock()) {
		std::lock_guard<std::recursive_mutex> lock(_lock4AdemcoEventList, std::adopt_lock);
		_buffer_mode = true;
		return true;
	}
	return false;
}


bool alarm_machine::LeaveBufferMode()
{
	if (_lock4AdemcoEventList.try_lock()) {
		std::lock_guard<std::recursive_mutex> lock(_lock4AdemcoEventList, std::adopt_lock);
		for (auto map : _mapList) {
			map->InversionControl(ICMC_MODE_NORMAL);
		}

		for (auto ademcoEvent: _ademcoEventList) {
			HandleAdemcoEvent(ademcoEvent);
		}
		_ademcoEventList.clear();
		_buffer_mode = false;
		return true;
	}
	return false;
}


void alarm_machine::TraverseAdmecoEventList(const observer_ptr& obj)
{
	std::lock_guard<std::recursive_mutex> lock(_lock4AdemcoEventList);
	std::shared_ptr<observer_type> obs(obj.lock());
	if (obs) {
		for (auto ademcoEvent : _ademcoEventList) {
			obs->on_update(ademcoEvent);
		}
	}
}


map_info_ptr alarm_machine::GetMapInfo(int map_id)
{
	for (auto mapInfo : _mapList) {
		if (mapInfo->get_id() == map_id)
			return mapInfo;
	}
	return nullptr;
}


void alarm_machine::HandleAdemcoEvent(const ademco::AdemcoEventPtr& ademcoEvent)
{
	AUTO_LOG_FUNCTION;

	// check reminder/expire
	if (GetTickCount() - _last_time_check_if_expire > CHECK_EXPIRE_GAP_TIME) {

		{
			auto now = std::chrono::system_clock::now();
			auto diff = consumer_->remind_time - now;
			if (std::chrono::duration_cast<std::chrono::minutes>(diff).count() <= 0) {
				PostMessageToMainWnd(WM_REMINDER_TIME_UP, _ademco_id, _submachine_zone);
			}
		}

		if (get_left_service_time_in_minutes() <= 0) {
			CString rec;
			int zoneValue = 0;
			if (_is_submachine) {
				zoneValue = _submachine_zone;
			}

			rec = get_formatted_name() + L" " + GetStringFromAppResource(IDS_STRING_EXPIRE);
			history_record_manager::GetInstance()->InsertRecord(_ademco_id, zoneValue, rec, 
														ademcoEvent->_recv_time, 
														RECORD_LEVEL_EXCEPTION);
			PostMessageToMainWnd(WM_SERVICE_TIME_UP, _ademco_id, _submachine_zone);
		}
		_last_time_check_if_expire = GetTickCount();
	}

	// handle ademco event
	if (!_is_submachine) {
		if (_banned) {
			return;
		}
#pragma region define val
		bool bMachineStatus = false;
		bool bOnofflineStatus = false;
		CString fmEvent, fmNull, record, fmZone, fmHangup, fmResume;
		fmNull = GetStringFromAppResource(IDS_STRING_NULL);
		fmZone = GetStringFromAppResource(IDS_STRING_ZONE);
		fmHangup = GetStringFromAppResource(IDS_STRING_CONN_HANGUP);
		fmResume = GetStringFromAppResource(IDS_STRING_CONN_RESUME);
		bool online = true;
		machine_status machine_status = MACHINE_DISARM;
		zone_info_ptr zone = GetZone(ademcoEvent->_zone);
		alarm_machine_ptr subMachine = nullptr;
		
		if (zone) {
			subMachine = zone->GetSubMachineInfo();
			
		}
#pragma endregion

#pragma region switch event
		switch (ademcoEvent->_event) {
			case ademco::EVENT_LINK_TEST:
				NotifySubmachines(ademcoEvent);
				return;
				break;
			case ademco::EVENT_SIGNAL_STRENGTH_CHANGED:
			{
				char sig = ademcoEvent->_xdata->at(0);
				int strength = ((sig >> 4) & 0x0F) * 10 + (sig & 0x0F);
				real_signal_strength_ = strength;
				JLOG(L"主机信号强度, ademco_id %06d, signal_strength %d", _ademco_id, strength);
				auto signal_strength = Integer2SignalStrength(strength);
				if (signal_strength != signal_strength_) {
					signal_strength_ = signal_strength;
					notify_observers(ademcoEvent);
				}
				return;
			}
				break;
			case ademco::EVENT_OFFLINE:
				bOnofflineStatus = true; 
				_rcccObj.reset();
				bMachineStatus = true; 
				online = false; 
				break;
			case ademco::EVENT_ONLINE: 
				bOnofflineStatus = true; 
				bMachineStatus = true; 
				break;
			case ademco::EVENT_CONN_HANGUP:
				if (_rcccObj.valid()) { _rcccObj.cb(_rcccObj.udata, RCCC_HANGUP); }
				record = get_formatted_name() + L" " + fmHangup;
				history_record_manager::GetInstance()->InsertRecord(_ademco_id, -1, record, 
															ademcoEvent->_recv_time,
															RECORD_LEVEL_STATUS);
				return;
				break;
			case ademco::EVENT_CONN_RESUME:
				if (_rcccObj.valid()) { _rcccObj.cb(_rcccObj.udata, RCCC_RESUME); }
				record = get_formatted_name() + L" " + fmResume;
				history_record_manager::GetInstance()->InsertRecord(_ademco_id, -1, record,
															ademcoEvent->_recv_time,
															RECORD_LEVEL_STATUS);
				return;
				break;
			case ademco::EVENT_RETRIEVE_ZONE_OR_SUB_MACHINE:
			case ademco::EVENT_ENTER_SET_MODE:
			case ademco::EVENT_STOP_RETRIEVE:
				HandleRetrieveResult(ademcoEvent);
				return;
				break;
			case ademco::EVENT_QUERY_SUB_MACHINE:
				return;
				break;
			case ademco::EVENT_I_AM_NET_MODULE:
				execute_set_machine_type(MT_NETMOD);
				notify_observers(ademcoEvent);
				NotifySubmachines(ademcoEvent);
				return;
				break;
			case EVENT_I_AM_EXPRESSED_GPRS_2050_MACHINE:
				execute_set_machine_type(MT_IMPRESSED_GPRS_MACHINE_2050);
				notify_observers(ademcoEvent);
				NotifySubmachines(ademcoEvent);
				return;
				break;
			case EVENT_PHONE_USER_CANCLE_ALARM:
				fmEvent = GetStringFromAppResource(IDS_STRING_PHONE_USER_CANCLE_ALARM);
				record = get_formatted_name() + L" " + fmEvent;
				history_record_manager::GetInstance()->InsertRecord(_ademco_id, -1, record,
					ademcoEvent->_recv_time,
					RECORD_LEVEL_STATUS);
				return;
				break;
			case ademco::EVENT_DISARM_PWD_ERR:
				alarm_machine_manager::GetInstance()->DisarmPasswdWrong(_ademco_id);
				return;
				break;
			case ademco::EVENT_DISARM: bMachineStatus = true; machine_status = MACHINE_DISARM; fmEvent = GetStringFromAppResource(IDS_STRING_DISARM);
				break;
			case ademco::EVENT_HALFARM: bMachineStatus = true; machine_status = MACHINE_HALFARM; fmEvent = GetStringFromAppResource(IDS_STRING_HALFARM);
				break;
			case ademco::EVENT_ARM: bMachineStatus = true; machine_status = MACHINE_ARM; fmEvent = GetStringFromAppResource(IDS_STRING_ARM);
				break;
			case ademco::EVENT_RECONNECT:
			case ademco::EVENT_SERIAL485CONN:
			case ademco::EVENT_SUB_MACHINE_SENSOR_RESUME:
			case ademco::EVENT_SUB_MACHINE_POWER_RESUME:
			case ademco::EVENT_BATTERY_EXCEPTION_RECOVER:
			case ademco::EVENT_OTHER_EXCEPTION_RECOVER:
				bMachineStatus = false;
				break;
			case ademco::EVENT_EMERGENCY:
			case ademco::EVENT_BADBATTERY:
			case ademco::EVENT_LOWBATTERY:
			//	bMachineStatus = true;
			case ademco::EVENT_BURGLAR:
			case ademco::EVENT_TEMPER:
			case ademco::EVENT_ZONE_TEMPER:
			case ademco::EVENT_DISCONNECT:
			case ademco::EVENT_SOLARDISTURB:
			case ademco::EVENT_SUB_MACHINE_SENSOR_EXCEPTION:
			case ademco::EVENT_SUB_MACHINE_POWER_EXCEPTION:
			case ademco::EVENT_BATTERY_EXCEPTION:
			case ademco::EVENT_OTHER_EXCEPTION:
				sound_manager::GetInstance()->LoopPlay(sound_manager::SI_BUGLAR);
				break;
			case ademco::EVENT_SERIAL485DIS:
				sound_manager::GetInstance()->LoopPlay(sound_manager::SI_OFFLINE);
				break;
			case ademco::EVENT_DOORRINGING:
				sound_manager::GetInstance()->LoopPlay(sound_manager::SI_DOORRING);
				break;
			case ademco::EVENT_FIRE:
				sound_manager::GetInstance()->LoopPlay(sound_manager::SI_FIRE);
				break;
			case ademco::EVENT_GAS:
				sound_manager::GetInstance()->LoopPlay(sound_manager::SI_GAS);
				break;
			case ademco::EVENT_DURESS:
				sound_manager::GetInstance()->LoopPlay(sound_manager::SI_PLEASE_HELP);
				break;
			case ademco::EVENT_WATER:
				sound_manager::GetInstance()->LoopPlay(sound_manager::SI_WATER);
				break;
			default: bMachineStatus = false;
				break;
		}
#pragma endregion

		// define AdemcoDataSegment for sending sms
		AdemcoDataSegment dataSegment;
		dataSegment.Make(_ademco_id, ademcoEvent->_sub_zone, ademcoEvent->_event, ademcoEvent->_zone);

		if (bMachineStatus) {	// status of machine
			bool bStatusChanged = false;
#pragma region online or armed
			if ((ademcoEvent->_zone == 0) && (ademcoEvent->_sub_zone == INDEX_ZONE)) {
				if (bOnofflineStatus) {
					bool old_online = get_online();
					switch (ademcoEvent->_source)
					{
					case ES_TCP_CLIENT:
						if (_online_by_direct_mode != online) {
							_online_by_direct_mode = online;
						}
						break;
					case ES_TCP_SERVER1:
						if (_online_by_transmit_mode1 != online) {
							_online_by_transmit_mode1 = online;
						}
						break;

					case ES_TCP_SERVER2:
						if (_online_by_transmit_mode2 != online) {
							_online_by_transmit_mode2 = online;
						}
						break;
					default:
						break;
					}

					if (old_online != get_online()) {
						group_manager* groupMgr = group_manager::GetInstance();
						group_info_ptr group = groupMgr->GetGroupInfo(_group_id);
						group->UpdateOnlineDescendantMachineCount(get_online());
						if (get_online()) {
							fmEvent = GetStringFromAppResource(IDS_STRING_ONLINE);
#if LOOP_PLAY_OFFLINE_SOUND
							sound_manager::GetInstance()->DecOffLineMachineNum();
#endif
						} else {
							fmEvent = GetStringFromAppResource(IDS_STRING_OFFLINE);
#if LOOP_PLAY_OFFLINE_SOUND
							sound_manager::GetInstance()->IncOffLineMachineNum();
#else
							sound_manager::GetInstance()->PlayOnce(sound_manager::SI_OFFLINE);
#endif
						}
					} else {
						return;
					}
				}

				if (!bOnofflineStatus && (_machine_status != machine_status)) {
					bStatusChanged = true;
					execute_set_machine_status(machine_status);
				}
			}
#pragma endregion

#pragma region status event

			gsm_manager* gsm = gsm_manager::GetInstance();
			if (ademcoEvent->_zone == 0) { // netmachine
				// 2015-06-05 16:35:49 submachine on/off line status follow machine on/off line status
				if (bOnofflineStatus && !_is_submachine) {
					SetAllSubMachineOnOffLine(online);
				} 
			} else { // submachine
				if (subMachine) {
					//subMachine->_online = online;
					if (!bOnofflineStatus) {
						if (!subMachine->get_online()) {
							subMachine->set_online(true);
						}
						if (subMachine->get_machine_status() != machine_status)
							bStatusChanged = true;

						if (subMachine->execute_set_machine_status(machine_status)) {
							subMachine->SetAdemcoEvent(ademcoEvent->_source,
													   ademcoEvent->_event,
													   ademcoEvent->_zone,
													   ademcoEvent->_sub_zone, 
													   ademcoEvent->_timestamp,
													   ademcoEvent->_recv_time,
													   ademcoEvent->_xdata
													   );
						}
					}
						
					if (bStatusChanged) {
						sms_config cfg = subMachine->get_sms_cfg();
						if (!subMachine->get_phone().IsEmpty()) {
							if (cfg.report_status) {
								gsm->SendSms(subMachine->get_phone(), &dataSegment, fmEvent);
							}
						}

						if (!subMachine->get_phone_bk().IsEmpty()) {
							if (cfg.report_status_bk) {
								gsm->SendSms(subMachine->get_phone_bk(), &dataSegment, fmEvent);
							}
						}
					}
					
				}
			}

			record.Format(L"%s %s", get_formatted_name(), fmEvent);

			if (!bOnofflineStatus && bStatusChanged) {
				if (!_phone.IsEmpty()) {
					if (_sms_cfg.report_status) {
						gsm->SendSms(_phone, &dataSegment, record);
					}
				}

				if (!_phone_bk.IsEmpty()) {
					if (_sms_cfg.report_status_bk) {
						gsm->SendSms(_phone_bk, &dataSegment, record);
					}
				}
			}
			history_record_manager::GetInstance()->InsertRecord(get_ademco_id(), 
														ademcoEvent->_zone,
														record, 
														ademcoEvent->_recv_time,
														RECORD_LEVEL_STATUS);
#pragma endregion
		} else { // alarm or exception event

#pragma region alarm event
			if (!_alarming) {
				_alarming = true;
				group_manager* groupMgr = group_manager::GetInstance();
				group_info_ptr group = groupMgr->GetGroupInfo(_group_id);
				group->UpdateAlarmingDescendantMachineCount();
			}
#pragma region format text
			CString smachine(L""), szone(L""), sevent(L"");
			smachine = get_formatted_name();

			if (ademcoEvent->_zone != 0) {
				if (ademcoEvent->_sub_zone == INDEX_ZONE) {
					CString aliasOfZoneOrSubMachine = fmNull;
					if (zone) {
						aliasOfZoneOrSubMachine = zone->get_alias();
					}
					if (_machine_type == MT_IMPRESSED_GPRS_MACHINE_2050) {
						szone.Format(L"%s%02d(%s)", fmZone, ademcoEvent->_zone, aliasOfZoneOrSubMachine);
					} else {
						szone.Format(L"%s%03d(%s)", fmZone, ademcoEvent->_zone, aliasOfZoneOrSubMachine);
					}
				} else {
					if (ademcoEvent->_sub_zone != INDEX_SUB_MACHINE) {
						CString ssubzone, ssubzone_alias = fmNull;
						if (subMachine) {
							zone_info_ptr subZone = subMachine->GetZone(ademcoEvent->_sub_zone);
							if (subZone) { ssubzone_alias = subZone->get_alias(); }
						}
						ssubzone.Format(L" %s%02d(%s)", fmZone, ademcoEvent->_sub_zone, ssubzone_alias);
						szone += ssubzone;
					}
				}
			} else {
				szone = smachine;
				smachine.Empty();
			}

			CAppResource* res = CAppResource::GetInstance();
			sevent.Format(L"%s", res->AdemcoEventToString(ademcoEvent->_event));

			time_t timestamp = ademcoEvent->_recv_time;
			wchar_t wtime[32] = { 0 };
			struct tm tmtm;
			localtime_s(&tmtm, &timestamp);
			if (timestamp == -1) {
				timestamp = time(nullptr);
				localtime_s(&tmtm, &timestamp);
			}
			wcsftime(wtime, 32, L"%H:%M:%S", &tmtm);

			alarm_text_ptr at = std::make_shared<alarm_text>();
			at->_zone = ademcoEvent->_zone;
			at->_subzone = ademcoEvent->_sub_zone;
			at->_event = ademcoEvent->_event;
			at->_txt.Format(L"%s %s %s", wtime, szone, sevent);
#pragma endregion

			EventLevel eventLevel = GetEventLevel(ademcoEvent->_event);
			set_highestEventLevel(eventLevel);

#pragma region write history recored
			history_record_manager *hr = history_record_manager::GetInstance();
			record_level recordLevel = RECORD_LEVEL_ALARM;
			if (eventLevel == EVENT_LEVEL_EXCEPTION || eventLevel == EVENT_LEVEL_EXCEPTION_RESUME) {
				recordLevel = RECORD_LEVEL_EXCEPTION;
			}
			hr->InsertRecord(get_ademco_id(), ademcoEvent->_zone,
							 smachine + szone + L" " + sevent,
							 ademcoEvent->_recv_time, recordLevel);
#pragma endregion

			// ui
			// 1. main view btn flash
			CWinApp* app = AfxGetApp(); ASSERT(app);
			auto wnd = static_cast<CAlarmCenterDlg*>(app->GetMainWnd()); ASSERT(wnd);
			wnd->MachineAlarm(shared_from_this());

			// 2. alarm text
			if (zone) {	
				map_info_ptr mapInfo = zone->GetMapInfo();
				auto dupAt = std::make_shared<alarm_text>(*at);
				if (subMachine) {
					zone_info_ptr subZone = subMachine->GetZone(ademcoEvent->_sub_zone);
					if (subZone) {
						subZone->HandleAdemcoEvent(ademcoEvent);
						map_info_ptr subMap = subZone->GetMapInfo();
						if (subMap.get()) {
							subMap->InversionControl(ICMC_ADD_ALARM_TEXT, dupAt);
						}
					} else {
						subMachine->_unbindZoneMap->InversionControl(ICMC_ADD_ALARM_TEXT, dupAt);
					}
				} else {
					if (mapInfo)
						mapInfo->InversionControl(ICMC_ADD_ALARM_TEXT, at);
					else
						_unbindZoneMap->InversionControl(ICMC_ADD_ALARM_TEXT, at);
					zone->HandleAdemcoEvent(ademcoEvent);
				}
			} else {	// 2.2 no zone alarm map
				_unbindZoneMap->InversionControl(ICMC_ADD_ALARM_TEXT, at);
			}

			if (subMachine) {
				if (!subMachine->get_alarming()) {
					subMachine->set_alarming(true);
					_alarmingSubMachineCount++;
				}
				subMachine->set_highestEventLevel(GetEventLevel(ademcoEvent->_event));
				subMachine->HandleAdemcoEvent(ademcoEvent);
			} else {
				_has_alarming_direct_zone = true;
			}

			// send sms
			gsm_manager* gsm = gsm_manager::GetInstance();
			if (!_phone.IsEmpty()) {
				if ((_sms_cfg.report_alarm && (eventLevel == EVENT_LEVEL_ALARM))
					|| (_sms_cfg.report_exception && (eventLevel == EVENT_LEVEL_EXCEPTION || eventLevel == EVENT_LEVEL_EXCEPTION_RESUME))) {
					gsm->SendSms(_phone, &dataSegment, szone + sevent);
				}
			}

			if (!_phone_bk.IsEmpty()) {
				if ((_sms_cfg.report_alarm_bk && eventLevel == EVENT_LEVEL_ALARM)
					|| (_sms_cfg.report_exception_bk && (eventLevel == EVENT_LEVEL_EXCEPTION || eventLevel == EVENT_LEVEL_EXCEPTION_RESUME))) {
					gsm->SendSms(_phone_bk, &dataSegment, szone + sevent);
				}
			}			

			if (subMachine) {
				sms_config cfg = subMachine->get_sms_cfg();
				if (!subMachine->get_phone().IsEmpty()) {
					if ((cfg.report_alarm && (eventLevel == EVENT_LEVEL_ALARM))
						|| (cfg.report_exception && (eventLevel == EVENT_LEVEL_EXCEPTION || eventLevel == EVENT_LEVEL_EXCEPTION_RESUME))) {
						gsm->SendSms(subMachine->get_phone(), &dataSegment, szone + sevent);
					}
				}

				if (!subMachine->get_phone_bk().IsEmpty()) {
					if ((cfg.report_alarm_bk && eventLevel == EVENT_LEVEL_ALARM)
						|| (cfg.report_exception_bk && (eventLevel == EVENT_LEVEL_EXCEPTION || eventLevel == EVENT_LEVEL_EXCEPTION_RESUME))) {
						gsm->SendSms(subMachine->get_phone_bk(), &dataSegment, szone + sevent);
					}
				}

				// show map of submachine (if exists)
				if (subMachine->get_auto_show_map_when_start_alarming() && g_baiduMapDlg) {
					g_baiduMapDlg->ShowMap(subMachine->get_ademco_id(), subMachine->get_submachine_zone());
				}
			} else {
				// show baidu map (if its not submachine)
				if (_auto_show_map_when_start_alarming && g_baiduMapDlg) {
					g_baiduMapDlg->ShowMap(_ademco_id, 0);
				}
			}
#pragma endregion
		}
	} else { // _is_submachine
		UpdateLastActionTime();
	}
	notify_observers(ademcoEvent);
}


void alarm_machine::SetAllSubMachineOnOffLine(bool online)
{
	auto t = time(nullptr);
	auto zoneIter = _zoneMap.begin();
	while (zoneIter != _zoneMap.end()) {
		if (!zoneIter->second) {
			zoneIter = _zoneMap.erase(zoneIter);
			continue;
		}
		alarm_machine_ptr subMachine = zoneIter->second->GetSubMachineInfo();
		if (subMachine) {
			subMachine->set_online(online);
			subMachine->SetAdemcoEvent(ES_UNKNOWN, online ? (MachineStatus2AdemcoEvent(subMachine->get_machine_status())) : EVENT_OFFLINE,
										subMachine->get_submachine_zone(),
										INDEX_SUB_MACHINE, t, t);
		}
		zoneIter++;
	}
}


void alarm_machine::HandleRetrieveResult(const ademco::AdemcoEventPtr& ademcoEvent)
{
	AUTO_LOG_FUNCTION;
	if (_machine_type == MT_IMPRESSED_GPRS_MACHINE_2050) {
		notify_observers(ademcoEvent);
		return;
	}

	int gg = ademcoEvent->_sub_zone;
	if (!ademcoEvent->_xdata || !(ademcoEvent->_xdata->size() == 3)) {
		/*ASSERT(0);*/ return;
	}
	char status = ademcoEvent->_xdata->at(0);
	int addr = MAKEWORD(ademcoEvent->_xdata->at(2), ademcoEvent->_xdata->at(1));
	JLOG(L"gg %d, zone %d, status %02X, addr %04X\n", 
		gg, ademcoEvent->_zone, status, addr & 0xFFFF);

	zone_info_ptr zoneInfo = GetZone(ademcoEvent->_zone);
	if (!zoneInfo) { 
		JLOG(L"no zoneInfo for %d\n", ademcoEvent->_zone);
		notify_observers(ademcoEvent);
	} else {
		JLOG(L"has zoneInfo for %d\n", ademcoEvent->_zone);
		alarm_machine_ptr subMachine = zoneInfo->GetSubMachineInfo();
		if (subMachine) {
			JLOG(L"has submachine info\n");
			subMachine->UpdateLastActionTime();
		} else {
			JLOG(L"no submachine info\n");
		}

		bool ok = true;


		if (is_zone_status(static_cast<unsigned char>(status & 0xFF)) != 
			is_zone_status(static_cast<unsigned char>(zoneInfo->get_status_or_property() & 0xFF))) {
			JLOG(L"status %02X != zoneInfo->get_status_or_property() %02X\n", 
				status, zoneInfo->get_status_or_property());
			ok = false;
		}
		if (addr != zoneInfo->get_physical_addr()) {
			JLOG(L"addr %04X != zoneInfo->get_physical_addr() %04X\n",
				addr, zoneInfo->get_physical_addr());
			ok = false;
		}

		if (ok) {
			JLOG(L"ok\n");
			if ((gg == 0xEE) && (subMachine != nullptr)) {
				JLOG(L"(gg == 0xEE) && (subMachine != nullptr)\n");
				ADEMCO_EVENT ademco_event = zone_info::char_to_status(status);
				auto t = time(nullptr);
				SetAdemcoEvent(ademcoEvent->_source, ademco_event, zoneInfo->get_zone_value(), 0xEE, t, t);
			} else if ((gg == 0x00) && (subMachine == nullptr)) {
				JLOG(L"(gg == 0x00) && (subMachine == nullptr)\n");
			} else { ok = false; ASSERT(0); }
			
			if (ok) { 
				JLOG(L"ok\n"); 
				notify_observers(ademcoEvent);
			} else {
				JLOG(L"failed.\n");
			}
		}
	}
}


void alarm_machine::NotifySubmachines(const ademco::AdemcoEventPtr& ademcoEvent)
{
	auto zoneIter = _zoneMap.begin();
	while (zoneIter != _zoneMap.end()) {
		if (!zoneIter->second) {
			zoneIter = _zoneMap.erase(zoneIter);
			continue;
		}
		if (zoneIter->second->get_type() == ZT_SUB_MACHINE) {
			alarm_machine_ptr subMachine = zoneIter->second->GetSubMachineInfo();
			if (subMachine) {
				subMachine->set_machine_type(_machine_type);
				subMachine->HandleAdemcoEvent(ademcoEvent);
			}
		}
		zoneIter++;
	}
}


void alarm_machine::SetAdemcoEvent(EventSource source, 
								   int ademco_event, int zone, int subzone,
								   const time_t& timestamp, const time_t& recv_time,
								   const ademco::char_array_ptr& xdata
								   )
{
	//AUTO_LOG_FUNCTION;
	_last_time_event_source = source;
	std::lock_guard<std::recursive_mutex> lock(_lock4AdemcoEventList);
	ademco::AdemcoEventPtr ademcoEvent = std::make_shared<AdemcoEvent>(source, ademco_event, zone, subzone, timestamp, recv_time, xdata);
	if (EVENT_PRIVATE_EVENT_BASE <= ademco_event && ademco_event <= EVENT_PRIVATE_EVENT_MAX) {
		// 内部事件立即处理
	} else {
#ifdef _DEBUG
		wchar_t wtime[32] = { 0 };
		struct tm tmtm;
		localtime_s(&tmtm, &recv_time);
		wcsftime(wtime, 32, L"%Y-%m-%d %H:%M:%S", &tmtm);
		JLOG(L"param: %s\n", wtime);
#endif
		time_t now = time(nullptr);
		auto iter = _ademcoEventFilter.begin();
		while (iter != _ademcoEventFilter.end()) {
			const ademco::AdemcoEventPtr& oldEvent = *iter;
#ifdef _DEBUG
			localtime_s(&tmtm, &now);
			wcsftime(wtime, 32, L"%Y-%m-%d %H:%M:%S", &tmtm);
			JLOG(L"now: %s\n", wtime);
			localtime_s(&tmtm, &oldEvent->_recv_time);
			wcsftime(wtime, 32, L"%Y-%m-%d %H:%M:%S", &tmtm);
			JLOG(L"old: %s\n", wtime);
#endif
			if (now - oldEvent->_recv_time >= 6) {
				_ademcoEventFilter.erase(iter);
				iter = _ademcoEventFilter.begin();
				continue;
			} else if (oldEvent->operator== (*ademcoEvent)) {
				JLOG(L"same AdemcoEvent, delete it. ademco_id %06d, event %04d, zone %03d, gg %02d\n", 
					 _ademco_id, ademcoEvent->_event, ademcoEvent->_zone, ademcoEvent->_sub_zone);
				_ademcoEventFilter.erase(iter);
				_ademcoEventFilter.push_back(ademcoEvent);
				return;
			}
			iter++;
		}
		_ademcoEventFilter.push_back(ademcoEvent);
	}

	if (_buffer_mode) {
		_ademcoEventList.push_back(ademcoEvent);
	} else {
		HandleAdemcoEvent(ademcoEvent);
	}
}


bool alarm_machine::execute_set_banned(bool banned)
{
	AUTO_LOG_FUNCTION;
	CString query;
	query.Format(L"update table_machine set banned=%d where id=%d and ademco_id=%d",
				 banned, _id, _ademco_id);
	alarm_machine_manager* mgr = alarm_machine_manager::GetInstance();
	BOOL ok = mgr->ExecuteSql(query);
	if (ok) {
		CString rec, fm;
		fm = GetStringFromAppResource(banned ? IDS_STRING_FM_BANNED : IDS_STRING_FM_UNBANNED);
		rec.Format(fm, get_ademco_id()/*, machine->GetDeviceIDW()*/);
		history_record_manager::GetInstance()->InsertRecord(get_ademco_id(),
													0, rec, time(nullptr),
													RECORD_LEVEL_USEREDIT);
		_banned = banned;
		return true;
	}

	return false;
}


bool alarm_machine::execute_set_machine_status(machine_status status)
{
	AUTO_LOG_FUNCTION;
	CString query;
	if (_is_submachine) {
		query.Format(L"update table_sub_machine set machine_status=%d where id=%d", status, _id);
	} else {
		query.Format(L"update table_machine set machine_status=%d where id=%d", status, _id);
	}
	alarm_machine_manager* mgr = alarm_machine_manager::GetInstance();
	BOOL ok = mgr->ExecuteSql(query);
	if (ok) {
		_machine_status = status;
		return true;
	}

	return false;
}


bool alarm_machine::execute_set_machine_type(machine_type type)
{
	AUTO_LOG_FUNCTION;
	CString query;
	query.Format(L"update table_machine set machine_type=%d where id=%d and ademco_id=%d",
				 type, _id, _ademco_id);
	alarm_machine_manager* mgr = alarm_machine_manager::GetInstance();
	BOOL ok = mgr->ExecuteSql(query);
	if (ok) {
		_machine_type = type;
		return true;
	}

	return false;
}


bool alarm_machine::execute_set_alias(const wchar_t* alias)
{
	AUTO_LOG_FUNCTION;
	CString query;
	query.Format(L"update table_machine set machine_name='%s' where id=%d and ademco_id=%d",
				 alias, _id, _ademco_id);
	alarm_machine_manager* mgr = alarm_machine_manager::GetInstance();
	BOOL ok = mgr->ExecuteSql(query);
	if (ok) {
		auto t = time(nullptr);
		CString rec, smachine, sfield;
		smachine = GetStringFromAppResource(IDS_STRING_MACHINE);
		sfield = GetStringFromAppResource(IDS_STRING_ALIAS);
		rec.Format(L"%s(" + GetStringFromAppResource(IDS_STRING_FM_ADEMCO_ID) + L") %s: %s --> %s",
				   smachine, _ademco_id, sfield, get_machine_name(), alias);
		history_record_manager::GetInstance()->InsertRecord(_ademco_id, 0, rec, t, RECORD_LEVEL_USEREDIT);
		set_alias(alias);
		auto ademcoEvent = std::make_shared<AdemcoEvent>(ES_UNKNOWN, EVENT_MACHINE_ALIAS, 0, 0, t, t);
		notify_observers(ademcoEvent);
		return true;
	}

	return false;
}


bool alarm_machine::execute_set_contact(const wchar_t* contact)
{
	AUTO_LOG_FUNCTION;
	CString query;
	query.Format(L"update table_machine set contact='%s' where id=%d and ademco_id=%d",
				 contact, _id, _ademco_id);
	alarm_machine_manager* mgr = alarm_machine_manager::GetInstance();
	BOOL ok = mgr->ExecuteSql(query);
	if (ok) {
		CString rec, smachine, sfield;
		smachine = GetStringFromAppResource(IDS_STRING_MACHINE);
		sfield = GetStringFromAppResource(IDS_STRING_CONTACT);
		rec.Format(L"%s(" + GetStringFromAppResource(IDS_STRING_FM_ADEMCO_ID) + L") %s: %s --> %s", 
				   smachine, get_ademco_id(), sfield, get_contact(), contact);
		history_record_manager::GetInstance()->InsertRecord(get_ademco_id(), 0, rec,
													time(nullptr), RECORD_LEVEL_USEREDIT);
		set_contact(contact);
		return true;
	}

	return false;
}


bool alarm_machine::execute_set_address(const wchar_t* address)
{
	AUTO_LOG_FUNCTION;
	CString query;
	query.Format(L"update table_machine set address='%s' where id=%d and ademco_id=%d",
				 address, _id, _ademco_id);

	alarm_machine_manager* mgr = alarm_machine_manager::GetInstance();
	BOOL ok = mgr->ExecuteSql(query);
	if (ok) {
		CString rec, smachine, sfield;
		smachine = GetStringFromAppResource(IDS_STRING_MACHINE);
		sfield = GetStringFromAppResource(IDS_STRING_ADDRESS);
		rec.Format(L"%s(" + GetStringFromAppResource(IDS_STRING_FM_ADEMCO_ID) + L") %s: %s --> %s", smachine, get_ademco_id(),
				   sfield, get_address(), address);
		history_record_manager::GetInstance()->InsertRecord(get_ademco_id(), 0, rec,
													time(nullptr), RECORD_LEVEL_USEREDIT);
		set_address(address);
		return true;
	}

	return false;
}


bool alarm_machine::execute_set_phone(const wchar_t* phone)
{
	AUTO_LOG_FUNCTION;
	CString query;
	query.Format(L"update table_machine set phone='%s' where id=%d and ademco_id=%d",
				 phone, _id, _ademco_id);

	alarm_machine_manager* mgr = alarm_machine_manager::GetInstance();
	BOOL ok = mgr->ExecuteSql(query);
	if (ok) {
		CString rec, smachine, sfield;
		smachine = GetStringFromAppResource(IDS_STRING_MACHINE);
		sfield = GetStringFromAppResource(IDS_STRING_PHONE);
		rec.Format(L"%s(" + GetStringFromAppResource(IDS_STRING_FM_ADEMCO_ID) + L") %s: %s --> %s", smachine, get_ademco_id(),
				   sfield, get_phone(), phone);
		history_record_manager::GetInstance()->InsertRecord(get_ademco_id(), 0, rec,
													time(nullptr), RECORD_LEVEL_USEREDIT);
		set_phone(phone);
		return true;
	}

	return false;
}


bool alarm_machine::execute_set_phone_bk(const wchar_t* phone_bk)
{
	AUTO_LOG_FUNCTION;
	CString query;
	query.Format(L"update table_machine set phone_bk='%s' where id=%d and ademco_id=%d",
				 phone_bk, _id, _ademco_id);
	alarm_machine_manager* mgr = alarm_machine_manager::GetInstance();
	BOOL ok = mgr->ExecuteSql(query);
	if (ok) {
		CString rec, smachine, sfield;
		smachine = GetStringFromAppResource(IDS_STRING_MACHINE);
		sfield = GetStringFromAppResource(IDS_STRING_PHONE_BK);
		rec.Format(L"%s(" + GetStringFromAppResource(IDS_STRING_FM_ADEMCO_ID) + L") %s: %s --> %s", smachine, get_ademco_id(),
				   sfield, get_phone_bk(), phone_bk);
		history_record_manager::GetInstance()->InsertRecord(get_ademco_id(), 0, rec,
													time(nullptr), RECORD_LEVEL_USEREDIT);
		set_phone_bk(phone_bk);
		return true;
	}

	return false;
}


bool alarm_machine::execute_set_group_id(int group_id)
{
	AUTO_LOG_FUNCTION;
	CString query;
	query.Format(L"update table_machine set group_id=%d where id=%d and ademco_id=%d",
				 group_id, _id, _ademco_id);
	alarm_machine_manager* mgr = alarm_machine_manager::GetInstance();
	BOOL ok = mgr->ExecuteSql(query);
	if (ok) {
		group_manager* group_mgr = group_manager::GetInstance();
		group_info_ptr old_group = group_mgr->GetGroupInfo(_group_id);
		group_info_ptr new_group = group_mgr->GetGroupInfo(group_id);
		old_group->RemoveChildMachine(shared_from_this());
		set_group_id(group_id);
		new_group->AddChildMachine(shared_from_this());
		return true;
	}

	return false;
}


bool alarm_machine::execute_add_zone(const zone_info_ptr& zoneInfo)
{
	CString query;
	if (_is_submachine) {
		zoneInfo->set_sub_machine_id(_id);
		query.Format(L"insert into table_sub_zone ([sub_machine_id],[sub_zone_value],[zone_name],[detector_info_id]) values(%d,%d,'%s',%d)",
					 _id, zoneInfo->get_sub_zone(), zoneInfo->get_alias(), 
					 zoneInfo->get_detector_id());
	} else {
		query.Format(L"insert into table_zone ([ademco_id],[sub_machine_id],[zone_value],[type],[zone_name],[status_or_property],[physical_addr],[detector_info_id])\
 values(%d,%d,%d,%d,'%s',%d,%d,%d)",
					 _ademco_id, -1, zoneInfo->get_zone_value(),
					 zoneInfo->get_type(), zoneInfo->get_alias(),
					 zoneInfo->get_status_or_property(), 
					 zoneInfo->get_physical_addr(), 
					 zoneInfo->get_detector_id());
	}
	alarm_machine_manager* mgr = alarm_machine_manager::GetInstance();
	int id = mgr->AddAutoIndexTableReturnID(query);
	if (-1 != id) {
		zoneInfo->set_id(id);
		zoneInfo->set_ademco_id(_ademco_id);
		if (wcslen(zoneInfo->get_alias()) == 0) {
			CString null;
			null = GetStringFromAppResource(IDS_STRING_NULL);
			zoneInfo->set_alias(null);
		}
		AddZone(zoneInfo);
		return true;
	}

	return false;
}


bool alarm_machine::execute_del_zone(const zone_info_ptr& zoneInfo)
{
	AUTO_LOG_FUNCTION;
	CString query;
	if (_is_submachine) {
		query.Format(L"delete from table_sub_zone where id=%d", zoneInfo->get_id());
	} else {
		query.Format(L"delete from table_zone where id=%d", zoneInfo->get_id());
	}
	alarm_machine_manager* mgr = alarm_machine_manager::GetInstance();
	BOOL ok = mgr->ExecuteSql(query);
	if (ok) {
		mgr->DeleteVideoBindInfoByZoneInfo(zoneInfo);
		detector_info_ptr detInfo = zoneInfo->GetDetectorInfo();
		if (detInfo) {
			query.Format(L"delete from table_detector where id=%d", detInfo->get_id());
			VERIFY(mgr->ExecuteSql(query));
		}

		map_info_ptr mapInfo = zoneInfo->GetMapInfo();
		if (mapInfo) {
			mapInfo->RemoveInterface(zoneInfo);
		}

		if (_is_submachine) {
			_zoneMap[zoneInfo->get_sub_zone()] = nullptr;
		} else {
			_zoneMap[zoneInfo->get_zone_value()] = nullptr;
		}

		return true;
	}

	return false;
}


void alarm_machine::GetAllZoneInfo(zone_info_list& list)
{
	auto zoneIter = _zoneMap.begin();
	while (zoneIter != _zoneMap.end()) {
		if (!zoneIter->second) {
			zoneIter = _zoneMap.erase(zoneIter);
			continue;
		}
		list.push_back(zoneIter->second);
		zoneIter++;
	}
}


void alarm_machine::GetAllMapInfo(map_info_list& list)
{
	std::copy(_mapList.begin(), _mapList.end(), std::back_inserter(list));
}


void alarm_machine::AddZone(const zone_info_ptr& zoneInfo)
{
	assert(zoneInfo);
	int zone = zoneInfo->get_zone_value();
	if (ZT_SUB_MACHINE_ZONE == zoneInfo->get_type()) {
		zone = zoneInfo->get_sub_zone();
	} else if (ZT_SUB_MACHINE == zoneInfo->get_type()) {
		inc_submachine_count();
	}
	if (0 <= zone && zone < MAX_MACHINE_ZONE) {
		_zoneMap[zone] = zoneInfo;

		detector_info_ptr detector = zoneInfo->GetDetectorInfo();
		if (detector) {
			int map_id = detector->get_map_id();
			map_info_ptr mapInfo = GetMapInfo(map_id);
			if (mapInfo) {
				mapInfo->AddInterface(zoneInfo);
				zoneInfo->SetMapInfo(mapInfo);
			}
		} else {
			_unbindZoneMap->AddInterface(zoneInfo);
			zoneInfo->SetMapInfo(_unbindZoneMap);
		}

	} else {
		ASSERT(0);
	}
}


zone_info_ptr alarm_machine::GetZone(int zone)
{
	if (0 <= zone && zone < MAX_MACHINE_ZONE) {
		auto iter = _zoneMap.find(zone);
		if (iter != _zoneMap.end()) {
			return iter->second;
		}
	} 

	return nullptr;
}


bool alarm_machine::execute_add_map(const core::map_info_ptr& mapInfo)
{
	map_type mt = _is_submachine ? MAP_SUB_MACHINE : MAP_MACHINE;
	mapInfo->set_type(mt);
	mapInfo->set_machine_id(_is_submachine ? _id : _ademco_id);

	CString query;
	query.Format(L"insert into table_map ([type],[machine_id],[map_name],[map_pic_path]) values(%d,%d,'%s','%s')",
				 mt, mapInfo->get_machine_id(), mapInfo->get_alias(),
				 mapInfo->get_path());
	alarm_machine_manager* mgr = alarm_machine_manager::GetInstance();
	int id = mgr->AddAutoIndexTableReturnID(query);
	if (-1 != id) {
		mapInfo->set_id(id);
		AddMap(mapInfo);
		mgr->AddMapInfo(mapInfo);
		return true;
	} else {
		ASSERT(0); JLOG(L"add map failed.\n"); 
		return false;
	}
}


bool alarm_machine::execute_update_map_alias(const core::map_info_ptr& mapInfo, const wchar_t* alias)
{
	AUTO_LOG_FUNCTION;
	ASSERT(mapInfo);
	CString query;
	query.Format(L"update table_map set map_name='%s' where id=%d", alias, mapInfo->get_id());
	alarm_machine_manager* mgr = alarm_machine_manager::GetInstance();
	if (mgr->ExecuteSql(query)) {
		mapInfo->set_alias(alias);
		return true;
	} else {
		ASSERT(0); JLOG(L"update map alias failed.\n");
		return false;
	}
}


bool alarm_machine::execute_update_map_path(const core::map_info_ptr& mapInfo, const wchar_t* path)
{
	AUTO_LOG_FUNCTION;
	ASSERT(mapInfo);
	CString query;
	query.Format(L"update table_map set map_pic_path='%s' where id=%d", path, mapInfo->get_id());
	alarm_machine_manager* mgr = alarm_machine_manager::GetInstance();
	if (mgr->ExecuteSql(query)) {
		mapInfo->set_path(path);
		return true;
	} else {
		ASSERT(0); JLOG(L"update map path failed.\n");
		return false;
	}
}


bool alarm_machine::execute_delete_map(const core::map_info_ptr& mapInfo)
{
	AUTO_LOG_FUNCTION;
	ASSERT(mapInfo && (-1 != mapInfo->get_id()));
	CString query;
	alarm_machine_manager* mgr = alarm_machine_manager::GetInstance();
	do {
		query.Format(L"delete from table_map where id=%d", mapInfo->get_id());
		if (!mgr->ExecuteSql(query)) {
			JLOG(L"delete map failed.\n"); break;
		}

		std::list<detector_bind_interface_ptr> list;
		mapInfo->GetAllInterfaceInfo(list);
		for (auto pInterface : list) {
			if (DIT_ZONE_INFO == pInterface->GetInterfaceType()) {
				auto zoneInfo = std::dynamic_pointer_cast<zone_info>(pInterface);
				_unbindZoneMap->AddInterface(zoneInfo);
				zoneInfo->SetMapInfo(_unbindZoneMap);
				zoneInfo->execute_del_detector_info();
			} else if (DIT_CAMERA_INFO == pInterface->GetInterfaceType()) {
				camera_info_ptr cam = std::dynamic_pointer_cast<camera_info>(pInterface);
				mgr->DeleteCameraInfo(cam);
			}
		}

		_mapList.remove(mapInfo);
		mgr->DeleteMapInfo(mapInfo);
		return true;
	} while (0);
	return false;
}


bool alarm_machine::execute_update_expire_time(const std::chrono::system_clock::time_point& tp)
{
	AUTO_LOG_FUNCTION;
	CString query;
	alarm_machine_manager* mgr = alarm_machine_manager::GetInstance();
	do {
		if (_is_submachine) {
			query.Format(L"update table_sub_machine set expire_time='%s' where id=%d",
						 time_point_to_wstring(tp).c_str(), _id);
		} else {
			query.Format(L"update table_machine set expire_time='%s' where id=%d",
						 time_point_to_wstring(tp).c_str(), _id);
		}
		if (!mgr->ExecuteSql(query)) {
			JLOG(L"update expire_time failed.\n"); break;
		}

		expire_time_ = tp;
		return true;
	} while (0);
	return false;
}


bool alarm_machine::execute_set_coor(const web::BaiduCoordinate& coor)
{
	AUTO_LOG_FUNCTION;
	CString query;
	alarm_machine_manager* mgr = alarm_machine_manager::GetInstance();
	do {
		if (_is_submachine) {
			query.Format(L"update table_sub_machine set map_coor_x=%f,map_coor_y=%f where id=%d",
						 coor.x, coor.y, _id);
		} else {
			query.Format(L"update table_machine set map_coor_x=%f,map_coor_y=%f where id=%d",
						 coor.x, coor.y, _id);
		}
		if (!mgr->ExecuteSql(query)) {
			JLOG(L"update baidu coor failed.\n"); break;
		}

		_coor = coor;
		return true;
	} while (0);
	return false;
}


void alarm_machine::inc_submachine_count()
{ 
	AUTO_LOG_FUNCTION;
	_submachine_count++;
	auto t = time(nullptr);
	auto ademcoEvent = std::make_shared<AdemcoEvent>(ES_UNKNOWN, EVENT_SUBMACHINECNT, 0, 0, t, t);
	notify_observers(ademcoEvent);
}


void alarm_machine::dec_submachine_count()
{ 
	//AUTO_LOG_FUNCTION;
	_submachine_count--;
	auto t = time(nullptr);
	auto ademcoEvent = std::make_shared<AdemcoEvent>(ES_UNKNOWN, EVENT_SUBMACHINECNT, 0, 0, t, t);
	notify_observers(ademcoEvent);
}


void alarm_machine::inc_alarmingSubMachineCount()
{
	_alarmingSubMachineCount++;
}


void alarm_machine::dec_alarmingSubMachineCount()
{
	if (_alarmingSubMachineCount == 0)
		return;

	if (--_alarmingSubMachineCount == 0 && !_has_alarming_direct_zone) {
		clear_ademco_event_list();
	}
}


void alarm_machine::set_highestEventLevel(EventLevel level)
{
	if (level > _highestEventLevel)
		_highestEventLevel = level;
}


CString alarm_machine::get_formatted_name(bool show_parent_name_if_has_parent) const {
	CString txt = L"";
	if (_is_submachine) {
		if (show_parent_name_if_has_parent) {
			auto parent = global_get_machine(_ademco_id);
			if (parent) {
				txt = parent->get_formatted_name();
			}
		}

		txt += GetStringFromAppResource(IDS_STRING_SUBMACHINE);
		txt.AppendFormat(L"%03d(%s)", _submachine_zone, alias_);
	} else {
		txt += GetStringFromAppResource(IDS_STRING_MACHINE);
		txt.AppendFormat(GetStringFromAppResource(IDS_STRING_FM_ADEMCO_ID) + L"(%s)", _ademco_id, alias_);
	}
	
	return txt;
}


CString alarm_machine::get_formatted_info(const CString& seperator) const
{
	CString info = L"", fmAlias, fmContact, fmAddress, fmPhone, fmPhoneBk, fmNull;
	CString contact, address, phone, phone_bk;
	fmContact = GetStringFromAppResource(IDS_STRING_CONTACT);
	fmAddress = GetStringFromAppResource(IDS_STRING_ADDRESS);
	fmPhone = GetStringFromAppResource(IDS_STRING_PHONE);
	fmPhoneBk = GetStringFromAppResource(IDS_STRING_PHONE_BK);
	fmNull = GetStringFromAppResource(IDS_STRING_NULL);

	contact = get_contact();
	address = get_address();
	phone = get_phone();
	phone_bk = get_phone_bk();

	info.Format(L"%s:%s%s%s:%s%s%s:%s%s%s:%s",
				   fmContact, contact.IsEmpty() ? fmNull : contact, seperator,
				   fmAddress, address.IsEmpty() ? fmNull : address, seperator,
				   fmPhone, phone.IsEmpty() ? fmNull : phone, seperator,
				   fmPhoneBk, phone_bk.IsEmpty() ? fmNull : phone_bk, seperator);

	return info;
}


bool alarm_machine::execute_set_sms_cfg(const sms_config& cfg)
{
	AUTO_LOG_FUNCTION;
	CString sql = L"";
	sql.Format(L"update table_sms_config set report_alarm=%d,report_exception=%d,report_status=%d,report_alarm_bk=%d,report_exception_bk=%d,report_status_bk=%d where id=%d",
			   cfg.report_alarm, cfg.report_exception, cfg.report_status,
			   cfg.report_alarm_bk, cfg.report_exception_bk, cfg.report_status_bk,
			   cfg.id);

	if (alarm_machine_manager::GetInstance()->ExecuteSql(sql)) {
		_sms_cfg = cfg;
		return true;
	}
	return false;
}


};

