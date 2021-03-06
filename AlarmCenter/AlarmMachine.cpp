﻿#include "stdafx.h"
#include "AlarmCenter.h"
#include "AlarmMachine.h"
#include "ZoneInfo.h"
#include "DetectorInfo.h"
#include "MapInfo.h"
#include "ademco_event.h"
#include "HistoryRecord.h"
#include "AlarmCenter.h"
#include "UserInfo.h"
#include "AlarmMachineManager.h"
#include "GroupInfo.h"
//#include "SubMachineInfo.h"
#include "SoundPlayer.h"
#include <algorithm>
#include <iterator>
#include <fstream>
#include "Gsm.h"
#include "alarm_center_map_service.h"
#include "CameraInfo.h"
#include "AlarmCenterDlg.h"
//#include "DbOper.h"
#include "../contrib/sqlitecpp/SQLiteCpp.h"
#include "../contrib/json/json.h"
#include "ConfigHelper.h"
#include "alarm_center_video_service.h"
#include "consumer.h"
#include "alarm_handle_mgr.h"
#include "congwin_fe100_mgr.h"

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

/////////////////// alarm machine implement /////////////////////////////////////////
alarm_machine::alarm_machine()
{
	_unbindZoneMap = std::make_shared<map_info>();
	_unbindZoneMap->set_id(-1);
	_unbindZoneMap->set_alias(TR(IDS_STRING_NOZONEMAP));
}


alarm_machine::~alarm_machine()
{
	auto t = time(nullptr);
	auto ademcoEvent = std::make_shared<AdemcoEvent>(ES_UNKNOWN, EVENT_IM_GONNA_DIE, 0, 0, t, t);
	notify_observers(ademcoEvent);
}

machine_uuid alarm_machine::get_uuid() const
{
	machine_uuid uuid;
	uuid.first = _ademco_id;
	if (_is_submachine) {
		uuid.second = _submachine_zone;
	} else {
		uuid.second = 0;
	}

	return uuid;
}


void alarm_machine::SetPrivatePacketFromServer1(const ademco::PrivatePacket* privatePacket)
{
	if (privatePacket == nullptr) {
		privatePacket_from_server1_ = nullptr;
		return;
	}

	if (privatePacket_from_server1_ == nullptr)
		privatePacket_from_server1_ = std::make_shared<PrivatePacket>();
	privatePacket_from_server1_->Copy(privatePacket);
}


void alarm_machine::SetPrivatePacketFromServer2(const ademco::PrivatePacket* privatePacket)
{
	if (privatePacket == nullptr) {
		privatePacket_from_server2_ = nullptr;
		return;
	}

	if (privatePacket_from_server2_ == nullptr)
		privatePacket_from_server2_ = std::make_shared<PrivatePacket>();
	privatePacket_from_server2_->Copy(privatePacket);
}


const ademco::PrivatePacketPtr alarm_machine::GetPrivatePacketFromServer1() const
{
	return privatePacket_from_server1_;
}


const ademco::PrivatePacketPtr alarm_machine::GetPrivatePacketFromServer2() const
{
	return privatePacket_from_server2_;
}


bool alarm_machine::execute_set_auto_show_map_when_start_alarming(bool b)
{
	auto mgr = alarm_machine_manager::get_instance();
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
	auto mgr = alarm_machine_manager::get_instance();
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


void alarm_machine::clear_ademco_event_list(bool alarm_handled_over, bool clear_sub_machine)
{
	if (!_alarming) return;
	if (!alarm_handled_over && alarm_id_ != 0) {
		return;
	}

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
			if (clear_sub_machine && subMachine && subMachine->get_alarming()) {
				subMachine->clear_ademco_event_list(alarm_handled_over);
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
	suser = TR(IDS_STRING_USER);
	sfm = TR(IDS_STRING_LOCAL_OP);
	sop = TR(IDS_STRING_CLR_ALM_MSG);
	auto user = user_manager::get_instance()->get_cur_user_info();
	srecord.Format(L"%s(ID:%d,%s)%s:%s", suser,
				   user->get_id(), user->get_name().c_str(),
				   sfm, sop);
	if (_is_submachine) {
		auto mgr = alarm_machine_manager::get_instance();
		alarm_machine_ptr parent_machine = mgr->GetMachine(_ademco_id);
		if (parent_machine) {
			parent_machine->dec_alarmingSubMachineCount();
		}
	}

	spost = get_formatted_name();
	srecord += spost;
	history_record_manager::get_instance()->InsertRecord(get_ademco_id(),
												_is_submachine ? _submachine_zone : 0,
												srecord, time(nullptr),
												RECORD_LEVEL_USERCONTROL);

	if (!_is_submachine) {
		auto groupMgr = group_manager::get_instance();
		group_info_ptr group = groupMgr->GetGroupInfo(_group_id);
		group->UpdateAlarmingDescendantMachineCount(false);

		CWinApp* app = AfxGetApp(); ASSERT(app);
		auto wnd = static_cast<CAlarmCenterDlg*>(app->GetMainWnd()); ASSERT(wnd);
		wnd->MachineDisalarm(shared_from_this());
	}
}

void alarm_machine::set_consumer(const consumer_ptr & consumer)
{
	bool empty = consumer_ == nullptr;
	consumer_ = consumer;
	if (!empty) {
		auto t = time(nullptr);
		auto ademcoEvent = std::make_shared<AdemcoEvent>(ES_UNKNOWN, EVENT_MACHINE_INFO_CHANGED, 0, 0, t, t);
		notify_observers(ademcoEvent);
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
	//AUTO_LOG_FUNCTION;

	// check reminder/expire
	if (_group_id != 0 && std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::steady_clock::now() - last_check_if_expire_time_).count() > CHECK_EXPIRE_GAP_TIME) {
		
		auto now = std::chrono::system_clock::now();

		{
			auto diff = consumer_->remind_time - now;
			if (std::chrono::duration_cast<std::chrono::minutes>(diff).count() <= 0) {
				CString rec;
				int zoneValue = 0;
				if (_is_submachine) {
					zoneValue = _submachine_zone;
				}

				rec = get_formatted_name() + L" " + TR(IDS_STRING_REMIND_TIME_UP);
				history_record_manager::get_instance()->InsertRecord(_ademco_id, zoneValue, rec,
																	 ademcoEvent->_recv_time,
																	 RECORD_LEVEL_EXCEPTION);
				notify_observers_with_event();
			}
		}

		if (get_left_service_time_in_minutes() <= 0) {
			CString rec;
			int zoneValue = 0;
			if (_is_submachine) {
				zoneValue = _submachine_zone;
			}

			rec = get_formatted_name() + L" " + TR(IDS_STRING_EXPIRE);
			history_record_manager::get_instance()->InsertRecord(_ademco_id, zoneValue, rec, 
														ademcoEvent->_recv_time, 
														RECORD_LEVEL_EXCEPTION);
			PostMessageToMainWnd(WM_SERVICE_TIME_UP, _ademco_id, _submachine_zone);
			
			notify_observers_with_event();
		}

		last_check_if_expire_time_ = std::chrono::steady_clock::now();
	}

	// handle ademco event
	if (!_is_submachine) {
		UpdateLastActionTime();
		if (_banned) {
			return;
		}
#pragma region define val
		bool bMachineStatus = false;
		bool bOnofflineStatus = false;
		CString fmEvent, fmNull, record, fmZone, fmHangup, fmResume;
		fmNull = TR(IDS_STRING_NULL);
		fmZone = TR(IDS_STRING_ZONE);
		fmHangup = TR(IDS_STRING_CONN_HANGUP);
		fmResume = TR(IDS_STRING_CONN_RESUME);
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
				history_record_manager::get_instance()->InsertRecord(_ademco_id, -1, record, 
															ademcoEvent->_recv_time,
															RECORD_LEVEL_STATUS);
				return;
				break;
			case ademco::EVENT_CONN_RESUME:
				if (_rcccObj.valid()) { _rcccObj.cb(_rcccObj.udata, RCCC_RESUME); }
				record = get_formatted_name() + L" " + fmResume;
				history_record_manager::get_instance()->InsertRecord(_ademco_id, -1, record,
															ademcoEvent->_recv_time,
															RECORD_LEVEL_STATUS);
				return;
				break;
			case ademco::EVENT_RETRIEVE_ZONE_OR_SUB_MACHINE:
			case ademco::EVENT_STOP_RETRIEVE:
			case ademco::EVENT_ENTER_SET_MODE:
				HandleRetrieveResult(ademcoEvent);
				return;
				break;
			case ademco::EVENT_ENTER_SETTING_MODE:
				setting_mode_ = true;
				handle_setting_mode();
				return;
				break;
			case ademco::EVENT_EXIT_SETTING_MODE:
				setting_mode_ = false;
				handle_setting_mode();
				return;
				break;
			case ademco::EVENT_RESTORE_FACTORY_SETTINGS:
				handle_restore_factory_settings();
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
			case EVENT_I_AM_LCD_MACHINE:
				execute_set_machine_type(MT_LCD);
				notify_observers(ademcoEvent);
				NotifySubmachines(ademcoEvent);
				return;
				break;
			case EVENT_I_AM_WIRE_MACHINE:
				execute_set_machine_type(MT_WIRE);
				notify_observers(ademcoEvent);
				NotifySubmachines(ademcoEvent);
				return;
				break;
			case EVENT_I_AM_WIFI_MACHINE:
				execute_set_machine_type(MT_WIFI);
				notify_observers(ademcoEvent);
				NotifySubmachines(ademcoEvent);
				return;
				break;
			case EVENT_PHONE_USER_CANCLE_ALARM:
				fmEvent = TR(IDS_STRING_PHONE_USER_CANCLE_ALARM);
				record = get_formatted_name() + L" " + fmEvent;
				history_record_manager::get_instance()->InsertRecord(_ademco_id, -1, record,
					ademcoEvent->_recv_time,
					RECORD_LEVEL_STATUS);
				return;
				break;
			case ademco::EVENT_DISARM_PWD_ERR:
				alarm_machine_manager::get_instance()->DisarmPasswdWrong(_ademco_id);
				return;
				break;
			case ademco::EVENT_DISARM: bMachineStatus = true; machine_status = MACHINE_DISARM; fmEvent = TR(IDS_STRING_DISARM);
				break;
			case ademco::EVENT_HALFARM_1456:
			case ademco::EVENT_HALFARM: 
				ademcoEvent->_event = EVENT_HALFARM;
				bMachineStatus = true; machine_status = MACHINE_HALFARM; fmEvent = TR(IDS_STRING_HALFARM);
				break;
			case ademco::EVENT_ARM: bMachineStatus = true; machine_status = MACHINE_ARM; fmEvent = TR(IDS_STRING_ARM);
				break;
			case ademco::EVENT_RECONNECT:
			case ademco::EVENT_SERIAL485CONN:
			case ademco::EVENT_SUB_MACHINE_SENSOR_RESUME:
			case ademco::EVENT_SUB_MACHINE_POWER_RESUME:
			case ademco::EVENT_BATTERY_EXCEPTION_RECOVER:
			case ademco::EVENT_OTHER_EXCEPTION_RECOVER:
				bMachineStatus = false;
				break;
			
			case ademco::EVENT_DISCONNECT:
			case ademco::EVENT_TEMPER:
			case ademco::EVENT_ZONE_TEMPER:
			case ademco::EVENT_BADBATTERY:
			case ademco::EVENT_LOWBATTERY:
			case ademco::EVENT_SOLARDISTURB:
			case ademco::EVENT_SUB_MACHINE_SENSOR_EXCEPTION:
			case ademco::EVENT_SUB_MACHINE_POWER_EXCEPTION:
			case ademco::EVENT_BATTERY_EXCEPTION:
			case ademco::EVENT_OTHER_EXCEPTION:
				sound_manager::get_instance()->Play(sound_manager::SI_EXCEPTION);
				break;

			case ademco::EVENT_EMERGENCY:
			case ademco::EVENT_BURGLAR:
				sound_manager::get_instance()->Play(sound_manager::SI_BUGLAR);
				break;
			
			case ademco::EVENT_SERIAL485DIS:
				sound_manager::get_instance()->Play(sound_manager::SI_OFFLINE);
				break;
			case ademco::EVENT_DOORRINGING:
				sound_manager::get_instance()->Play(sound_manager::SI_DOORRING);
				break;
			case ademco::EVENT_FIRE:
				sound_manager::get_instance()->Play(sound_manager::SI_FIRE);
				break;
			case ademco::EVENT_GAS:
				sound_manager::get_instance()->Play(sound_manager::SI_GAS);
				break;
			case ademco::EVENT_DURESS:
				sound_manager::get_instance()->Play(sound_manager::SI_PLEASE_HELP);
				break;
			case ademco::EVENT_WATER:
				sound_manager::get_instance()->Play(sound_manager::SI_WATER);
				break;
			case ademco::EVENT_BY_PASS:
			case ademco::EVENT_BY_PASS_RESUME:
			case ademco::EVENT_3100:
				return;
			default: bMachineStatus = false;
				break;
		}
#pragma endregion

		// 2016-12-2 15:04:49 若此时存储的是在设置状态，且此时收到了主机状态和防区报警，说明主机退出设置的命令字已经漏掉，需要手动提示主机已退出设置
		if (setting_mode_) {
			setting_mode_ = false;
			handle_setting_mode();
		}

		// define AdemcoDataSegment for sending sms
		AdemcoDataSegment dataSegment;
		dataSegment.Make(_ademco_id, ademcoEvent->_sub_zone, ademcoEvent->_event, ademcoEvent->_zone);

		if (bMachineStatus) {	// status of machine
			bool bStatusChanged = false;
			bool is_need_write_history = false;
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
						is_need_write_history = true;
						auto groupMgr = group_manager::get_instance();
						group_info_ptr group = groupMgr->GetGroupInfo(_group_id);
						group->UpdateOnlineDescendantMachineCount(get_online());
						if (get_online()) {
							fmEvent = TR(IDS_STRING_ONLINE);
#if 1
							sound_manager::get_instance()->DecOffLineMachineNum();
#endif
						} else {
							fmEvent = TR(IDS_STRING_OFFLINE);
#if 1
							sound_manager::get_instance()->IncOffLineMachineNum();
							sound_manager::get_instance()->Play(sound_manager::SI_OFFLINE);
#else
							sound_manager::get_instance()->PlayOnce(sound_manager::SI_OFFLINE);
#endif
						}
					} else {
						return;
					}
				}

				if (!bOnofflineStatus && (_machine_status != machine_status)) {
					bStatusChanged = true;
					is_need_write_history = true;
					execute_set_machine_status(machine_status);

					// 2016-11-26 14:54:55
					if (setting_mode_) {

					}
				}
			}
#pragma endregion

#pragma region status event

			auto gsm = gsm_manager::get_instance();
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

			if (is_need_write_history) {
				history_record_manager::get_instance()->InsertRecord(get_ademco_id(),
																	 ademcoEvent->_zone,
																	 record,
																	 ademcoEvent->_recv_time,
																	 RECORD_LEVEL_STATUS);

				core::congwin_fe100_mgr::get_instance()->add_event(&dataSegment);
			}
#pragma endregion
		} else { // alarm or exception event
			/*if (_banned) {
				return;
			}*/
#pragma region alarm event
			core::congwin_fe100_mgr::get_instance()->add_event(&dataSegment);
			if (!_alarming) {
				_alarming = true;
				auto groupMgr = group_manager::get_instance();
				group_info_ptr group = groupMgr->GetGroupInfo(_group_id);
				group->UpdateAlarmingDescendantMachineCount();
			}
#pragma region format text
			CString smachine(L""), szone(L""), sevent(L"");
			smachine = get_formatted_name();

			bool is_machine_self_alarm = false;

			if (ademcoEvent->_zone != 0) {
				if (ademcoEvent->_sub_zone == INDEX_ZONE) {
					CString aliasOfZoneOrSubMachine = fmNull;
					if (zone) {
						aliasOfZoneOrSubMachine = zone->get_alias();
					}
						
					szone += fmZone;
					szone.AppendFormat(get_format_string_of_machine_zone_count_figure_by_type(_machine_type), ademcoEvent->_zone);
					szone.AppendFormat(L"(%s)", aliasOfZoneOrSubMachine);

				} else {
					if (ademcoEvent->_sub_zone != INDEX_SUB_MACHINE) {
						CString ssubzone, ssubzone_alias = fmNull;
						if (subMachine) {
							zone_info_ptr subZone = subMachine->GetZone(ademcoEvent->_sub_zone);
							if (subZone) { ssubzone_alias = subZone->get_alias(); }
						}
						ssubzone.Format(L" %s%02d(%s)", fmZone, ademcoEvent->_sub_zone, ssubzone_alias);
						szone += ssubzone;
					} else {
						is_machine_self_alarm = true;
					}
				}
			} else {
				szone = smachine;
				smachine.Empty();
				is_machine_self_alarm = true;
			}			

			auto res = CAppResource::get_instance();
			sevent.Format(L"%s", res->AdemcoEventToString(ademcoEvent->_event));

			time_t timestamp = ademcoEvent->_recv_time;
			/*auto wtime = jlib::time_t_to_wstring(timestamp, 2);	*/
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
			at->_time = ademcoEvent->_recv_time;
			at->_txt.Format(L"%s %s %s", wtime, szone, sevent);

			// 2016年8月28日16:18:13 处理主机自身报警
			if (is_machine_self_alarm) {
				auto video_service = ipc::alarm_center_video_service::get_instance();
				if (ademcoEvent->_zone == 0) {
					video_service->play_video(std::make_shared<video::zone_uuid>(_ademco_id, ZONE_VALUE_FOR_MACHINE_SELF, 0), at);
				} else if (ademcoEvent->_sub_zone == INDEX_SUB_MACHINE) {
					video_service->play_video(std::make_shared<video::zone_uuid>(_ademco_id, ademcoEvent->_zone, ZONE_VALUE_FOR_MACHINE_SELF), at);
				} 
			}

			// 2016-11-15 14:35:09 处理正在处警、未消警时又来了报警
			auto alarm_mgr = alarm_handle_mgr::get_instance();
			int alarm_id = 0;
			if (subMachine) {
				alarm_id = subMachine->get_alarm_id();
			} else {
				alarm_id = alarm_id_;
			}

			if (alarm_id != 0) {
				alarm_mgr->execute_add_alarm_text(alarm_id, at);
			}

#pragma endregion

			EventLevel eventLevel = GetEventLevel(ademcoEvent->_event);
			set_highestEventLevel(eventLevel);

#pragma region write history recored
			auto hr = history_record_manager::get_instance();
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
			if (_group_id != 0) {
				CWinApp* app = AfxGetApp(); ASSERT(app);
				auto wnd = static_cast<CAlarmCenterDlg*>(app->GetMainWnd()); ASSERT(wnd);
				wnd->MachineAlarm(shared_from_this());
			}

			// 2. alarm text
			if (zone) {	
				map_info_ptr mapInfo = zone->GetMapInfo();
				if (subMachine) {
					zone_info_ptr subZone = subMachine->GetZone(ademcoEvent->_sub_zone);
					if (subZone) {
						subZone->HandleAdemcoEvent(ademcoEvent, at);
						map_info_ptr subMap = subZone->GetMapInfo();
						if (subMap.get()) {
							subMap->InversionControl(ICMC_ADD_ALARM_TEXT, at);
						}
					} else {
						subMachine->_unbindZoneMap->InversionControl(ICMC_ADD_ALARM_TEXT, at);
					}
				} else {
					if (mapInfo)
						mapInfo->InversionControl(ICMC_ADD_ALARM_TEXT, at);
					else
						_unbindZoneMap->InversionControl(ICMC_ADD_ALARM_TEXT, at);
					zone->HandleAdemcoEvent(ademcoEvent, at);
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
			auto gsm = gsm_manager::get_instance();
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
				if (subMachine->get_auto_show_map_when_start_alarming() /*&& g_baiduMapDlg*/) {
					ipc::alarm_center_map_service::get_instance()->show_map(subMachine->get_ademco_id(), subMachine->get_submachine_zone());
				}
			} else {
				// show baidu map (if its not submachine)
				if (_auto_show_map_when_start_alarming/* && g_baiduMapDlg*/) {
					ipc::alarm_center_map_service::get_instance()->show_map(_ademco_id, 0);
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

void alarm_machine::handle_setting_mode()
{
	CString rec = get_formatted_name();
	if (setting_mode_) {
		rec += L" " + TR(IDS_STRING_ENTER_SET_MODE);
	} else {
		rec += L" " + TR(IDS_STRING_EXIT_SET_MODE);
	}

	history_record_manager::get_instance()->InsertRecord(_ademco_id, _is_submachine ? _submachine_zone : 0, rec, time(nullptr), RECORD_LEVEL_STATUS);
	notify_observers_with_event();
}

void alarm_machine::handle_restore_factory_settings()
{
	CString rec = get_formatted_name();
	rec += L" " + TR(IDS_STRING_RESTORE_FACTORY_SETTINGS);
	history_record_manager::get_instance()->InsertRecord(_ademco_id, _is_submachine ? _submachine_zone : 0, rec, time(nullptr), RECORD_LEVEL_STATUS);
}

void alarm_machine::notify_observers_with_event(ademco::ADEMCO_EVENT evnt)
{
	auto t = time(nullptr);
	static AdemcoEventPtr ademcoEvent = nullptr;
	if (!ademcoEvent) {
		ademcoEvent = std::make_shared<AdemcoEvent>(ES_UNKNOWN, evnt, 0, 0, t, t);
	} else {
		ademcoEvent->_event = evnt;
		ademcoEvent->_recv_time = ademcoEvent->_timestamp = t;
	}

	notify_observers(ademcoEvent);
}


void alarm_machine::HandleRetrieveResult(const ademco::AdemcoEventPtr& ademcoEvent)
{
	AUTO_LOG_FUNCTION;
	if (is_machine_can_only_add_zone_by_retrieve(_machine_type)) {
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
				ADEMCO_EVENT ademco_event = static_cast<ADEMCO_EVENT>(zone_info::char_to_status(status));
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
								   ADEMCO_EVENT ademco_event, int zone, int subzone,
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

		////#define LOG_TIME_FOR_ALARM_MACHINE_SET_ADEMCO_EVENT
		//
		//#ifdef LOG_TIME_FOR_ALARM_MACHINE_SET_ADEMCO_EVENT
		//		wchar_t wtime[32] = { 0 };
		//		struct tm tmtm;
		//		localtime_s(&tmtm, &recv_time);
		//		wcsftime(wtime, 32, L"%Y-%m-%d %H:%M:%S", &tmtm);
		//		JLOG(L"param: %s\n", wtime);
		//#endif
		//		time_t now = time(nullptr);
		//		auto iter = _ademcoEventFilter.begin();
		//		while (iter != _ademcoEventFilter.end()) {
		//			const ademco::AdemcoEventPtr& oldEvent = *iter;
		//#ifdef LOG_TIME_FOR_ALARM_MACHINE_SET_ADEMCO_EVENT
		//			localtime_s(&tmtm, &now);
		//			wcsftime(wtime, 32, L"%Y-%m-%d %H:%M:%S", &tmtm);
		//			JLOG(L"now: %s\n", wtime);
		//			localtime_s(&tmtm, &oldEvent->_recv_time);
		//			wcsftime(wtime, 32, L"%Y-%m-%d %H:%M:%S", &tmtm);
		//			JLOG(L"old: %s\n", wtime);
		//#endif
		//			if (now - oldEvent->_recv_time >= 6) {
		//				iter = _ademcoEventFilter.erase(iter);
		//				continue;
		//			} else if (oldEvent->operator== (*ademcoEvent)) {
		//				JLOG(L"same AdemcoEvent, delete it. ademco_id %06d, event %04d, zone %03d, gg %02d\n", 
		//					 _ademco_id, ademcoEvent->_event, ademcoEvent->_zone, ademcoEvent->_sub_zone);
		//				_ademcoEventFilter.erase(iter);
		//				_ademcoEventFilter.push_back(ademcoEvent);
		//				return;
		//			}
		//			iter++;
		//		}
		//		_ademcoEventFilter.push_back(ademcoEvent);
		//	}

		if (ademco_event_filter_) {
			time_t now = time(nullptr);
			if ((now - ademco_event_filter_->_recv_time < 6) && (ademco_event_filter_->operator== (*ademcoEvent))) {
				ademco_event_filter_ = ademcoEvent;
				return;
			}
		} 

		ademco_event_filter_ = ademcoEvent;
		
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
	auto mgr = alarm_machine_manager::get_instance();
	BOOL ok = mgr->ExecuteSql(query);
	if (ok) {
		CString rec, fm;
		auto t = time(nullptr);
		fm = TR(banned ? IDS_STRING_FM_BANNED : IDS_STRING_FM_UNBANNED);
		rec.Format(fm, get_ademco_id()/*, machine->GetDeviceIDW()*/);
		history_record_manager::get_instance()->InsertRecord(get_ademco_id(),
													0, rec, t,
													RECORD_LEVEL_USEREDIT);
		_banned = banned;

		notify_observers_with_event();
		return true;
	}

	return false;
}


bool alarm_machine::execute_set_machine_status(machine_status status)
{
	AUTO_LOG_FUNCTION;
	_machine_status = status;
	CString query;
	if (_is_submachine) {
		query.Format(L"update table_sub_machine set machine_status=%d where id=%d", status, _id);
	} else {
		query.Format(L"update table_machine set machine_status=%d where id=%d", status, _id);
	}
	auto mgr = alarm_machine_manager::get_instance();
	BOOL ok = mgr->ExecuteSql(query);
	if (ok) {
		return true;
	}

	return false;
}


bool alarm_machine::execute_set_machine_type(machine_type type)
{
	AUTO_LOG_FUNCTION;
	if (_machine_type == type) {
		return true;
	}

	_machine_type = type;
	CString query;
	query.Format(L"update table_machine set machine_type=%d where id=%d and ademco_id=%d",
				 type, _id, _ademco_id);
	auto mgr = alarm_machine_manager::get_instance();
	BOOL ok = mgr->ExecuteSql(query);
	if (ok) {
		query = get_formatted_name();
		query += L" " + TR(IDS_STRING_REPORT_MACHINE_TYPE);
		query += L" " + CAppResource::get_instance()->MachineTypeToString(_machine_type);
		auto t = time(nullptr);
		history_record_manager::get_instance()->InsertRecord(_ademco_id, 0, query, t, RECORD_LEVEL_STATUS);

		notify_observers_with_event();
		return true;
	}

	return false;
}


bool alarm_machine::execute_set_alias(const wchar_t* alias)
{
	AUTO_LOG_FUNCTION;

	if (alias_.Compare(alias) == 0)
		return true;

	CString query;
	query.Format(L"update table_machine set machine_name='%s' where id=%d and ademco_id=%d",
				 alias, _id, _ademco_id);
	auto mgr = alarm_machine_manager::get_instance();
	BOOL ok = mgr->ExecuteSql(query);
	if (ok) {
		auto t = time(nullptr);
		CString rec, smachine, sfield;
		smachine = TR(IDS_STRING_MACHINE);
		sfield = TR(IDS_STRING_ALIAS);
		rec.Format(L"%s(" + TR(IDS_STRING_FM_ADEMCO_ID) + L") %s: %s --> %s",
				   smachine, _ademco_id, sfield, get_machine_name(), alias);
		history_record_manager::get_instance()->InsertRecord(_ademco_id, 0, rec, t, RECORD_LEVEL_USEREDIT);
		set_alias(alias);
		notify_observers_with_event();
		return true;
	}

	return false;
}


bool alarm_machine::execute_set_contact(const wchar_t* contact)
{
	AUTO_LOG_FUNCTION;

	if (_contact.Compare(contact) == 0)
		return true;

	CString query;
	query.Format(L"update table_machine set contact='%s' where id=%d and ademco_id=%d",
				 contact, _id, _ademco_id);
	auto mgr = alarm_machine_manager::get_instance();
	BOOL ok = mgr->ExecuteSql(query);
	if (ok) {
		CString rec, smachine, sfield;
		smachine = TR(IDS_STRING_MACHINE);
		sfield = TR(IDS_STRING_CONTACT);
		rec.Format(L"%s(" + TR(IDS_STRING_FM_ADEMCO_ID) + L") %s: %s --> %s", 
				   smachine, get_ademco_id(), sfield, get_contact(), contact);
		auto t = time(nullptr);
		history_record_manager::get_instance()->InsertRecord(get_ademco_id(), 0, rec,
													t, RECORD_LEVEL_USEREDIT);
		set_contact(contact);
		notify_observers_with_event();
		return true;
	}

	return false;
}


bool alarm_machine::execute_set_address(const wchar_t* address)
{
	AUTO_LOG_FUNCTION;

	if (_address.Compare(address) == 0)
		return true;

	CString query;
	query.Format(L"update table_machine set address='%s' where id=%d and ademco_id=%d",
				 address, _id, _ademco_id);

	auto mgr = alarm_machine_manager::get_instance();
	BOOL ok = mgr->ExecuteSql(query);
	if (ok) {
		CString rec, smachine, sfield;
		smachine = TR(IDS_STRING_MACHINE);
		sfield = TR(IDS_STRING_ADDRESS);
		rec.Format(L"%s(" + TR(IDS_STRING_FM_ADEMCO_ID) + L") %s: %s --> %s", smachine, get_ademco_id(),
				   sfield, get_address(), address);
		auto t = time(nullptr);
		history_record_manager::get_instance()->InsertRecord(get_ademco_id(), 0, rec,
													t, RECORD_LEVEL_USEREDIT);
		set_address(address);
		notify_observers_with_event();
		return true;
	}

	return false;
}


bool alarm_machine::execute_set_phone(const wchar_t* phone)
{
	AUTO_LOG_FUNCTION;

	if (_phone.Compare(phone) == 0)
		return true;

	CString query;
	query.Format(L"update table_machine set phone='%s' where id=%d and ademco_id=%d",
				 phone, _id, _ademco_id);

	auto mgr = alarm_machine_manager::get_instance();
	BOOL ok = mgr->ExecuteSql(query);
	if (ok) {
		CString rec, smachine, sfield;
		smachine = TR(IDS_STRING_MACHINE);
		sfield = TR(IDS_STRING_PHONE);
		rec.Format(L"%s(" + TR(IDS_STRING_FM_ADEMCO_ID) + L") %s: %s --> %s", smachine, get_ademco_id(),
				   sfield, get_phone(), phone);
		auto t = time(nullptr);
		history_record_manager::get_instance()->InsertRecord(get_ademco_id(), 0, rec,
													t, RECORD_LEVEL_USEREDIT);
		set_phone(phone);
		notify_observers_with_event();
	}

	return false;
}


bool alarm_machine::execute_set_phone_bk(const wchar_t* phone_bk)
{
	AUTO_LOG_FUNCTION;

	if (_phone_bk.Compare(phone_bk) == 0)
		return true;

	CString query;
	query.Format(L"update table_machine set phone_bk='%s' where id=%d and ademco_id=%d",
				 phone_bk, _id, _ademco_id);
	auto mgr = alarm_machine_manager::get_instance();
	BOOL ok = mgr->ExecuteSql(query);
	if (ok) {
		CString rec, smachine, sfield;
		smachine = TR(IDS_STRING_MACHINE);
		sfield = TR(IDS_STRING_PHONE_BK);
		rec.Format(L"%s(" + TR(IDS_STRING_FM_ADEMCO_ID) + L") %s: %s --> %s", smachine, get_ademco_id(),
				   sfield, get_phone_bk(), phone_bk);
		auto t = time(nullptr);
		history_record_manager::get_instance()->InsertRecord(get_ademco_id(), 0, rec,
													t, RECORD_LEVEL_USEREDIT);
		set_phone_bk(phone_bk);
		notify_observers_with_event();
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
	auto mgr = alarm_machine_manager::get_instance();
	BOOL ok = mgr->ExecuteSql(query);
	if (ok) {
		auto group_mgr = group_manager::get_instance();
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
	auto mgr = alarm_machine_manager::get_instance();
	int id = mgr->AddAutoIndexTableReturnID(query);
	if (-1 != id) {
		zoneInfo->set_id(id);
		zoneInfo->set_ademco_id(_ademco_id);
		if (wcslen(zoneInfo->get_alias()) == 0) {
			CString null;
			null = TR(IDS_STRING_NULL);
			zoneInfo->set_alias(null);
		}

		CString rec;
		rec.Format(L"%s %s %s", get_formatted_name(), 
				   TR(zoneInfo->get_type() == ZT_SUB_MACHINE ? IDS_STRING_ADDED_SUBMACHINE : IDS_STRING_ADDED_ZONE),
				   zoneInfo->get_formatted_short_name());
		core::history_record_manager::get_instance()->InsertRecord(_ademco_id, zoneInfo->get_zone_value(),
																   rec, time(nullptr), RECORD_LEVEL_USEREDIT);

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
	auto mgr = alarm_machine_manager::get_instance();
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

		CString rec;
		rec.Format(L"%s %s %s", get_formatted_name(), 
				   TR(zoneInfo->get_type() == ZT_SUB_MACHINE ? IDS_STRING_DELED_SUBMACHINE : IDS_STRING_DELED_ZONE),
				   zoneInfo->get_formatted_short_name());
		core::history_record_manager::get_instance()->InsertRecord(_ademco_id, zoneInfo->get_zone_value(),
																   rec, time(nullptr), RECORD_LEVEL_USEREDIT);

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
	auto mgr = alarm_machine_manager::get_instance();
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
	auto mgr = alarm_machine_manager::get_instance();
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
	auto mgr = alarm_machine_manager::get_instance();
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
	auto mgr = alarm_machine_manager::get_instance();
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
	auto mgr = alarm_machine_manager::get_instance();
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

		// 2016-11-19 14:21:05 automatically set remind time to 7 days before expire time.
		if (consumer_) {
			auto diff = std::chrono::hours(24 * 7);
			auto remind_time = tp - diff;
			auto tmp = std::make_shared<consumer>(*consumer_);
			tmp->remind_time = remind_time;
			if (consumer_manager::get_instance()->execute_update_consumer(tmp)) {
				consumer_ = tmp;
			}
		}

		expire_time_ = tp;

		notify_observers_with_event();
		
		return true;
	} while (0);
	return false;
}


bool alarm_machine::execute_set_coor(const web::BaiduCoordinate& coor)
{
	AUTO_LOG_FUNCTION;
	CString query;
	auto mgr = alarm_machine_manager::get_instance();
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
	//AUTO_LOG_FUNCTION;
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
		clear_ademco_event_list(true, true);
	}
}


void alarm_machine::set_highestEventLevel(EventLevel level)
{
	if (level > _highestEventLevel)
		_highestEventLevel = level;
}


CString alarm_machine::get_formatted_name(bool show_parent_name_if_has_parent) const 
{
	CString txt = L"";
	if (_is_submachine) {
		if (show_parent_name_if_has_parent) {
			auto parent = global_get_machine(_ademco_id);
			if (parent) {
				txt = parent->get_formatted_name();
			}
		}

		txt += TR(IDS_STRING_SUBMACHINE);
		txt.AppendFormat(L"%03d(%s)", _submachine_zone, alias_);
	} else {
		txt += TR(IDS_STRING_MACHINE);
		txt.AppendFormat(TR(IDS_STRING_FM_ADEMCO_ID) + L"(%s)", _ademco_id, alias_);
	}
	
	return txt;
}


CString alarm_machine::get_formatted_info(const CString& seperator) const
{
	CString info = L"", fmAlias, fmType, fmContact, fmAddress, fmPhone, fmPhoneBk, fmNull;
	CString contact, address, phone, phone_bk;
	fmType = TR(IDS_STRING_MACHINE_TYPE);
	fmContact = TR(IDS_STRING_CONTACT);
	fmAddress = TR(IDS_STRING_ADDRESS);
	fmPhone = TR(IDS_STRING_PHONE);
	fmPhoneBk = TR(IDS_STRING_PHONE_BK);
	fmNull = TR(IDS_STRING_NULL);

	contact = get_contact();
	address = get_address();
	phone = get_phone();
	phone_bk = get_phone_bk();

	info.Format(L"%s:%s%s%s:%s%s%s:%s%s%s:%s%s%s:%s",
				fmType, CAppResource::get_instance()->MachineTypeToString(_machine_type), seperator,
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

	if (alarm_machine_manager::get_instance()->ExecuteSql(sql)) {
		_sms_cfg = cfg;
		return true;
	}
	return false;
}

void alarm_machine::set_alarm_id(int id)
{
	//if (alarm_id_ != id) {
		alarm_id_ = id;
		notify_observers_with_event();
	//}
}

};

