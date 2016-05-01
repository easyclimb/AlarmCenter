﻿#include "stdafx.h"
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
#include "Gsm.h"
#include "tinyxml\\tinyxml.h"
#include "BaiduMapViewerDlg.h"
#include "CameraInfo.h"
#include "AlarmCenterDlg.h"


using namespace ademco;
namespace core {

#ifdef _DEBUG
static const int CHECK_EXPIRE_GAP_TIME = 6 * 1000; // check machine if expire in every 6 seconds.
#else
static const int CHECK_EXPIRE_GAP_TIME = 60 * 1000; // check machine if expire in every minutes.
#endif

//IMPLEMENT_OBSERVER(CAlarmMachine)
	
CAlarmMachine::CAlarmMachine()
	: _id(0)
	, _ademco_id(0)
	, _group_id(0)
	, _machine_status(MACHINE_STATUS_UNKNOWN)
	, _alarming(false)
	, _has_alarming_direct_zone(false)
	, _buffer_mode(false)
	, _is_submachine(false)
	, _has_video(false)
	, _submachine_zone(0)
	, _submachine_count(0)
	, _unbindZoneMap(nullptr)
	, _highestEventLevel(EVENT_LEVEL_STATUS)
	, _alarmingSubMachineCount(0)
	, _lastActionTime(time(nullptr))
	, _bChecking(false)
	, _expire_time()
	, _last_time_check_if_expire(0)
	, _coor()
	, _zoomLevel(14)
	// 2015年8月1日 14:46:21 storaged in xml
	, _auto_show_map_when_start_alarming(true)
	, _privatePacket(nullptr)
{
	memset(_ipv4, 0, sizeof(_ipv4));

	_unbindZoneMap = std::make_shared<CMapInfo>();
	_unbindZoneMap->set_id(-1);
	CString fmAlias;
	fmAlias = GetStringFromAppResource(IDS_STRING_NOZONEMAP);
	_unbindZoneMap->set_alias(fmAlias);

	//LoadXmlConfig();
}


CAlarmMachine::~CAlarmMachine()
{
	auto t = time(nullptr);
	auto ademcoEvent = std::make_shared<AdemcoEvent>(ES_UNKNOWN, EVENT_IM_GONNA_DIE, 0, 0, t, t);
	notify_observers(ademcoEvent);

	_mapList.clear();

	_ademcoEventList.clear();

	_ademcoEventFilter.clear();

	_zoneMap.clear();

}


void CAlarmMachine::SetPrivatePacket(const ademco::PrivatePacket* privatePacket)
{
	if (privatePacket == nullptr) {
		_privatePacket = nullptr;;
		return;
	}

	if (_privatePacket == nullptr)
		_privatePacket = std::make_shared<PrivatePacket>();
	_privatePacket->Copy(privatePacket);
}


const ademco::PrivatePacketPtr CAlarmMachine::GetPrivatePacket() const
{
	return _privatePacket;
}


std::string CAlarmMachine::get_xml_path()
{
	USES_CONVERSION;
	CString dir = L"", path = L"";
	dir.Format(L"%s\\data\\config", GetModuleFilePath());
	CreateDirectory(dir, nullptr);
	dir += L"\\AlarmMachine";
	CreateDirectory(dir, nullptr);
	if (_is_submachine) {
		path.Format(L"%s\\%04d-%03d.xml", dir, _ademco_id, _submachine_zone);
	} else {
		path.Format(L"%s\\%04d.xml", dir, _ademco_id);
	}
	return W2A(path);
}


void CAlarmMachine::LoadXmlConfig()
{
	using namespace tinyxml;
	std::string path = get_xml_path();
	TiXmlDocument doc(path.c_str());
	bool ok = false;
	do {
		if (!doc.LoadFile()) break;
		TiXmlElement* root = doc.FirstChildElement();
		if (!root)break;
		TiXmlElement* auto_show_map_when_start_alarming = root->FirstChildElement("auto_show_map_when_start_alarming");
		if (!auto_show_map_when_start_alarming)break;
		const char* text = auto_show_map_when_start_alarming->Attribute("value");
		if (!text || strlen(text) == 0) break;
		_auto_show_map_when_start_alarming = atoi(text) == 1;

		TiXmlElement *zoom_level = root->FirstChildElement("zoom_level");
		if (!zoom_level)break;
		text = zoom_level->Attribute("value");
		if (!text || strlen(text) == 0) break;
		_zoomLevel = atoi(text);
		if (_zoomLevel < 0) _zoomLevel = 14;

		ok = true;
	} while (0);

	if (!ok) {
		SaveXmlConfig();
	}
}


void CAlarmMachine::SaveXmlConfig()
{
	using namespace tinyxml;
	std::string path = get_xml_path();
	TiXmlDocument doc;
	TiXmlDeclaration* dec = new TiXmlDeclaration("1.0", "", "");
	doc.LinkEndChild(dec);
	TiXmlElement* root = new TiXmlElement("AlarmMachineConfig");
	doc.LinkEndChild(root);

	TiXmlElement* auto_show_map_when_start_alarming = new TiXmlElement("auto_show_map_when_start_alarming");
	auto_show_map_when_start_alarming->SetAttribute("value", _auto_show_map_when_start_alarming);
	root->LinkEndChild(auto_show_map_when_start_alarming);

	TiXmlElement* zoom_level = new TiXmlElement("zoom_level");
	zoom_level->SetAttribute("value", _zoomLevel);
	root->LinkEndChild(zoom_level);

	doc.SaveFile(path.c_str());
}


void CAlarmMachine::set_auto_show_map_when_start_alarming(bool b)
{
	if (b != _auto_show_map_when_start_alarming) {
		_auto_show_map_when_start_alarming = b;
		SaveXmlConfig();
	}
}


void CAlarmMachine::set_zoomLevel(int zoomLevel)
{
	if (zoomLevel != _zoomLevel) {
		_zoomLevel = zoomLevel;
		if (_zoomLevel < 0) _zoomLevel = 14;
		SaveXmlConfig();
	}
}


void CAlarmMachine::clear_ademco_event_list()
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
		CMapInfoPtr mapInfo = zoneIter->second->GetMapInfo();
		if (mapInfo.get()) {
			mapInfo->InversionControl(ICMC_CLR_ALARM_TEXT);
		}
		if (zoneIter->second->get_type() == ZT_SUB_MACHINE) {
			CAlarmMachinePtr subMachine = zoneIter->second->GetSubMachineInfo();
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
	CString srecord, suser, sfm, sop, spost, fmSubmachine;
	suser = GetStringFromAppResource(IDS_STRING_USER);
	sfm = GetStringFromAppResource(IDS_STRING_LOCAL_OP);
	sop = GetStringFromAppResource(IDS_STRING_CLR_MSG);
	fmSubmachine = GetStringFromAppResource(IDS_STRING_SUBMACHINE);
	auto user = CUserManager::GetInstance()->GetCurUserInfo();
	srecord.Format(L"%s(ID:%d,%s)%s:%s", suser,
				   user->get_user_id(), user->get_user_name(),
				   sfm, sop);
	if (_is_submachine) {
		CAlarmMachineManager* mgr = CAlarmMachineManager::GetInstance();
		CAlarmMachinePtr parent_machine = mgr->GetMachine(_ademco_id);
		if (parent_machine) {
			spost.Format(L"%s%s%s", parent_machine->get_formatted_machine_name(),
						 fmSubmachine, get_formatted_machine_name());
			parent_machine->dec_alarmingSubMachineCount();
		}
	} else {
		spost = get_formatted_machine_name();
	}
	srecord += spost;
	CHistoryRecord::GetInstance()->InsertRecord(get_ademco_id(),
												_is_submachine ? _submachine_zone : 0,
												srecord, time(nullptr),
												RECORD_LEVEL_USERCONTROL);

	if (!_is_submachine) {
		CGroupManager* groupMgr = CGroupManager::GetInstance();
		CGroupInfoPtr group = groupMgr->GetGroupInfo(_group_id);
		group->UpdateAlarmingDescendantMachineCount(false);

		CWinApp* app = AfxGetApp(); ASSERT(app);
		auto wnd = static_cast<CAlarmCenterDlg*>(app->GetMainWnd()); ASSERT(wnd);
		wnd->MachineDisalarm(shared_from_this());
	}
}


bool CAlarmMachine::EnterBufferMode()
{ 
	AUTO_LOG_FUNCTION;
	_ootebmOjb.call();
	if (_lock4AdemcoEventList.try_lock()) {
		std::lock_guard<std::recursive_mutex> lock(_lock4AdemcoEventList, std::adopt_lock);
		_buffer_mode = true;
		return true;
	}
	return false;
}


bool CAlarmMachine::LeaveBufferMode()
{
	AUTO_LOG_FUNCTION;
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


void CAlarmMachine::TraverseAdmecoEventList(const observer_ptr& obj)
{
	AUTO_LOG_FUNCTION;
	std::lock_guard<std::recursive_mutex> lock(_lock4AdemcoEventList);
	std::shared_ptr<observer_type> obs(obj.lock());
	if (obs) {
		for (auto ademcoEvent : _ademcoEventList) {
			obs->on_update(ademcoEvent);
		}
	}
}


CMapInfoPtr CAlarmMachine::GetMapInfo(int map_id)
{
	for (auto mapInfo : _mapList) {
		if (mapInfo->get_id() == map_id)
			return mapInfo;
	}
	return nullptr;
}


void CAlarmMachine::HandleAdemcoEvent(const ademco::AdemcoEventPtr& ademcoEvent)
{
	AUTO_LOG_FUNCTION;

	// check if expire
	if (GetTickCount() - _last_time_check_if_expire > CHECK_EXPIRE_GAP_TIME) {
		if (get_left_service_time() <= 0) {
			CString rec, fmmachine, fmsubmachine, fmexpire;
			fmmachine = GetStringFromAppResource(IDS_STRING_MACHINE);
			fmsubmachine = GetStringFromAppResource(IDS_STRING_SUBMACHINE);
			fmexpire = GetStringFromAppResource(IDS_STRING_EXPIRE);
			int zoneValue = 0;
			if (_is_submachine) {
				CAlarmMachinePtr parentMachine = CAlarmMachineManager::GetInstance()->GetMachine(_ademco_id);
				rec.Format(L"%s%s%s%s %s", 
						   fmmachine, parentMachine->get_formatted_machine_name(), 
						   fmsubmachine, get_formatted_machine_name(), fmexpire);
				zoneValue = _submachine_zone;
			} else {
				rec.Format(L"%s%s %s", fmmachine, get_formatted_machine_name(), fmexpire);
			}
			CHistoryRecord::GetInstance()->InsertRecord(_ademco_id, zoneValue, rec, 
														ademcoEvent->_recv_time, 
														RECORD_LEVEL_EXCEPTION);
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
		CString fmEvent, fmNull, record, fmMachine, fmSubMachine, fmZone, fmHangup, fmResume;
		fmNull = GetStringFromAppResource(IDS_STRING_NULL);
		fmMachine = GetStringFromAppResource(IDS_STRING_MACHINE);
		fmSubMachine = GetStringFromAppResource(IDS_STRING_SUBMACHINE);
		fmZone = GetStringFromAppResource(IDS_STRING_ZONE);
		fmHangup = GetStringFromAppResource(IDS_STRING_CONN_HANGUP);
		fmResume = GetStringFromAppResource(IDS_STRING_CONN_RESUME);
		bool online = true;
		MachineStatus machine_status = MACHINE_DISARM;
		CZoneInfoPtr zone = GetZone(ademcoEvent->_zone);
		CAlarmMachinePtr subMachine = nullptr;
		CString aliasOfZoneOrSubMachine = fmNull;
		if (zone) {
			subMachine = zone->GetSubMachineInfo();
			if (subMachine) { aliasOfZoneOrSubMachine = subMachine->get_machine_name(); } 
			else { aliasOfZoneOrSubMachine = zone->get_alias(); }
		}
#pragma endregion

#pragma region switch event
		switch (ademcoEvent->_event) {
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
				record.Format(L"%s%s %s", fmMachine, get_formatted_machine_name(), fmHangup);
				CHistoryRecord::GetInstance()->InsertRecord(_ademco_id, -1, record, 
															ademcoEvent->_recv_time,
															RECORD_LEVEL_STATUS);
				return;
				break;
			case ademco::EVENT_CONN_RESUME:
				if (_rcccObj.valid()) { _rcccObj.cb(_rcccObj.udata, RCCC_RESUME); }
				record.Format(L"%s%s %s", fmMachine, get_formatted_machine_name(), fmResume);
				CHistoryRecord::GetInstance()->InsertRecord(_ademco_id, -1, record,
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
				record.Format(L"%s%s %s", fmMachine, get_formatted_machine_name(), fmEvent);
				CHistoryRecord::GetInstance()->InsertRecord(_ademco_id, -1, record,
					ademcoEvent->_recv_time,
					RECORD_LEVEL_STATUS);
				return;
				break;
			case ademco::EVENT_DISARM_PWD_ERR:
				CAlarmMachineManager::GetInstance()->DisarmPasswdWrong(_ademco_id);
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
				CSoundPlayer::GetInstance()->LoopPlay(CSoundPlayer::SI_BUGLAR);
				break;
			case ademco::EVENT_SERIAL485DIS:
				CSoundPlayer::GetInstance()->LoopPlay(CSoundPlayer::SI_OFFLINE);
				break;
			case ademco::EVENT_DOORRINGING:
				CSoundPlayer::GetInstance()->LoopPlay(CSoundPlayer::SI_DOORRING);
				break;
			case ademco::EVENT_FIRE:
				CSoundPlayer::GetInstance()->LoopPlay(CSoundPlayer::SI_FIRE);
				break;
			case ademco::EVENT_GAS:
				CSoundPlayer::GetInstance()->LoopPlay(CSoundPlayer::SI_GAS);
				break;
			case ademco::EVENT_DURESS:
				CSoundPlayer::GetInstance()->LoopPlay(CSoundPlayer::SI_PLEASE_HELP);
				break;
			case ademco::EVENT_WATER:
				CSoundPlayer::GetInstance()->LoopPlay(CSoundPlayer::SI_WATER);
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
						CGroupManager* groupMgr = CGroupManager::GetInstance();
						CGroupInfoPtr group = groupMgr->GetGroupInfo(_group_id);
						group->UpdateOnlineDescendantMachineCount(get_online());
						if (get_online()) {
							fmEvent = GetStringFromAppResource(IDS_STRING_ONLINE);
#if LOOP_PLAY_OFFLINE_SOUND
							CSoundPlayer::GetInstance()->DecOffLineMachineNum();
#endif
						} else {
							fmEvent = GetStringFromAppResource(IDS_STRING_OFFLINE);
#if LOOP_PLAY_OFFLINE_SOUND
							CSoundPlayer::GetInstance()->IncOffLineMachineNum();
#else
							CSoundPlayer::GetInstance()->PlayOnce(CSoundPlayer::SI_OFFLINE);
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
			CGsm* gsm = CGsm::GetInstance();
			if (ademcoEvent->_zone == 0) { // netmachine
				record.Format(L"%s%s %s", fmMachine, get_formatted_machine_name(), fmEvent);
				// 2015-06-05 16:35:49 submachine on/off line status follow machine on/off line status
				if (bOnofflineStatus && !_is_submachine) {
					SetAllSubMachineOnOffLine(online);
				} 
			} else { // submachine
				record.Format(L"%s%s %s%03d(%s) %s",
							  fmMachine, get_formatted_machine_name(),
							  fmSubMachine, ademcoEvent->_zone, aliasOfZoneOrSubMachine,
							  fmEvent);
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
						SmsConfigure cfg = subMachine->get_sms_cfg();
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
			CHistoryRecord::GetInstance()->InsertRecord(get_ademco_id(), 
														ademcoEvent->_zone,
														record, 
														ademcoEvent->_recv_time,
														RECORD_LEVEL_STATUS);
#pragma endregion
		} else { // alarm or exception event

#pragma region alarm event
			if (!_alarming) {
				_alarming = true;
				CGroupManager* groupMgr = CGroupManager::GetInstance();
				CGroupInfoPtr group = groupMgr->GetGroupInfo(_group_id);
				group->UpdateAlarmingDescendantMachineCount();
			}
#pragma region format text
			CString smachine(L""), szone(L""), sevent(L""), stmp(L"");
			smachine.Format(L"%s%s ", fmMachine, get_formatted_machine_name());

			if (ademcoEvent->_zone != 0) {
				if (ademcoEvent->_sub_zone == INDEX_ZONE) {
					if (_machine_type == MT_IMPRESSED_GPRS_MACHINE_2050) {
						szone.Format(L"%s%02d(%s)", fmZone, ademcoEvent->_zone, aliasOfZoneOrSubMachine);
					} else {
						szone.Format(L"%s%03d(%s)", fmZone, ademcoEvent->_zone, aliasOfZoneOrSubMachine);
					}
				} else {
					stmp.Format(L"%s%03d(%s)", fmSubMachine, ademcoEvent->_zone, aliasOfZoneOrSubMachine);
					smachine += stmp; 
					if (ademcoEvent->_sub_zone != INDEX_SUB_MACHINE) {
						CString ssubzone, ssubzone_alias = fmNull;
						if (subMachine) {
							CZoneInfoPtr subZone = subMachine->GetZone(ademcoEvent->_sub_zone);
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

			AlarmTextPtr at = std::make_shared<AlarmText>();
			at->_zone = ademcoEvent->_zone;
			at->_subzone = ademcoEvent->_sub_zone;
			at->_event = ademcoEvent->_event;
			at->_txt.Format(L"%s %s %s", wtime, szone, sevent);
#pragma endregion

			EventLevel eventLevel = GetEventLevel(ademcoEvent->_event);
			set_highestEventLevel(eventLevel);

#pragma region write history recored
			CHistoryRecord *hr = CHistoryRecord::GetInstance();
			RecordLevel recordLevel = RECORD_LEVEL_ALARM;
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
				CMapInfoPtr mapInfo = zone->GetMapInfo();
				auto dupAt = std::make_shared<AlarmText>(*at);
				if (subMachine) {
					CZoneInfoPtr subZone = subMachine->GetZone(ademcoEvent->_sub_zone);
					if (subZone) {
						subZone->HandleAdemcoEvent(ademcoEvent);
						CMapInfoPtr subMap = subZone->GetMapInfo();
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
			CGsm* gsm = CGsm::GetInstance();
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
				SmsConfigure cfg = subMachine->get_sms_cfg();
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


void CAlarmMachine::SetAllSubMachineOnOffLine(bool online)
{
	auto t = time(nullptr);
	auto zoneIter = _zoneMap.begin();
	while (zoneIter != _zoneMap.end()) {
		if (!zoneIter->second) {
			zoneIter = _zoneMap.erase(zoneIter);
			continue;
		}
		CAlarmMachinePtr subMachine = zoneIter->second->GetSubMachineInfo();
		if (subMachine) {
			subMachine->set_online(online);
			subMachine->SetAdemcoEvent(ES_UNKNOWN, online ? (MachineStatus2AdemcoEvent(subMachine->get_machine_status())) : EVENT_OFFLINE,
										subMachine->get_submachine_zone(),
										INDEX_SUB_MACHINE, t, t);
		}
		zoneIter++;
	}
}


void CAlarmMachine::HandleRetrieveResult(const ademco::AdemcoEventPtr& ademcoEvent)
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

	CZoneInfoPtr zoneInfo = GetZone(ademcoEvent->_zone);
	if (!zoneInfo) { 
		JLOG(L"no zoneInfo for %d\n", ademcoEvent->_zone);
		notify_observers(ademcoEvent);
	} else {
		JLOG(L"has zoneInfo for %d\n", ademcoEvent->_zone);
		CAlarmMachinePtr subMachine = zoneInfo->GetSubMachineInfo();
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
				ADEMCO_EVENT ademco_event = CZoneInfo::char_to_status(status);
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


void CAlarmMachine::NotifySubmachines(const ademco::AdemcoEventPtr& ademcoEvent)
{
	auto zoneIter = _zoneMap.begin();
	while (zoneIter != _zoneMap.end()) {
		if (!zoneIter->second) {
			zoneIter = _zoneMap.erase(zoneIter);
			continue;
		}
		if (zoneIter->second->get_type() == ZT_SUB_MACHINE) {
			CAlarmMachinePtr subMachine = zoneIter->second->GetSubMachineInfo();
			if (subMachine) {
				subMachine->set_machine_type(_machine_type);
				subMachine->HandleAdemcoEvent(ademcoEvent);
			}
		}
		zoneIter++;
	}
}


void CAlarmMachine::SetAdemcoEvent(EventSource source, 
								   int ademco_event, int zone, int subzone,
								   const time_t& timestamp, const time_t& recv_time,
								   const ademco::char_array_ptr& xdata
								   )
{
	AUTO_LOG_FUNCTION;
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
		//_ademcoEventList.push_back(ademcoEvent);
		HandleAdemcoEvent(ademcoEvent);
	}
}


//void CAlarmMachine::set_device_id(const wchar_t* device_id)
//{
//	wcscpy_s(_device_idW, device_id);
//	USES_CONVERSION;
//	strcpy_s(_device_id, W2A(_device_idW));
//}
//
//
//void CAlarmMachine::set_device_id(const char* device_id)
//{
//	strcpy_s(_device_id, device_id);
//	USES_CONVERSION;
//	wcscpy_s(_device_idW, A2W(device_id));
//}


bool CAlarmMachine::execute_set_banned(bool banned)
{
	AUTO_LOG_FUNCTION;
	CString query;
	query.Format(L"update AlarmMachine set banned=%d where id=%d and ademco_id=%d",
				 banned, _id, _ademco_id);
	CAlarmMachineManager* mgr = CAlarmMachineManager::GetInstance();
	BOOL ok = mgr->ExecuteSql(query);
	if (ok) {
		_banned = banned;
		return true;
	}

	return false;
}

bool CAlarmMachine::execute_set_has_video(bool has)
{
	AUTO_LOG_FUNCTION;
	CString query;
	query.Format(L"update AlarmMachine set has_video=%d where id=%d and ademco_id=%d",
				 has, _id, _ademco_id);
	CAlarmMachineManager* mgr = CAlarmMachineManager::GetInstance();
	BOOL ok = mgr->ExecuteSql(query);
	if (ok) {
		_has_video = has;
		return true;
	}

	return false;
}


bool CAlarmMachine::execute_set_machine_status(MachineStatus status)
{
	AUTO_LOG_FUNCTION;
	CString query;
	if (_is_submachine) {
		query.Format(L"update SubMachine set machine_status=%d where id=%d", status, _id);
	} else {
		query.Format(L"update AlarmMachine set machine_status=%d where id=%d", status, _id);
	}
	CAlarmMachineManager* mgr = CAlarmMachineManager::GetInstance();
	BOOL ok = mgr->ExecuteSql(query);
	if (ok) {
		_machine_status = status;
		return true;
	}

	return false;
}


bool CAlarmMachine::execute_set_machine_type(MachineType type)
{
	AUTO_LOG_FUNCTION;
	CString query;
	query.Format(L"update AlarmMachine set machine_type=%d where id=%d and ademco_id=%d",
				 type, _id, _ademco_id);
	CAlarmMachineManager* mgr = CAlarmMachineManager::GetInstance();
	BOOL ok = mgr->ExecuteSql(query);
	if (ok) {
		_machine_type = type;
		return true;
	}

	return false;
}


bool CAlarmMachine::execute_set_alias(const wchar_t* alias)
{
	AUTO_LOG_FUNCTION;
	CString query;
	query.Format(L"update AlarmMachine set alias='%s' where id=%d and ademco_id=%d",
				 alias, _id, _ademco_id);
	CAlarmMachineManager* mgr = CAlarmMachineManager::GetInstance();
	BOOL ok = mgr->ExecuteSql(query);
	if (ok) {
		set_alias(alias);
		auto t = time(nullptr);
		auto ademcoEvent = std::make_shared<AdemcoEvent>(ES_UNKNOWN, EVENT_MACHINE_ALIAS, 0, 0, t, t);
		notify_observers(ademcoEvent);
		return true;
	}

	return false;
}


bool CAlarmMachine::execute_set_contact(const wchar_t* contact)
{
	AUTO_LOG_FUNCTION;
	CString query;
	query.Format(L"update AlarmMachine set contact='%s' where id=%d and ademco_id=%d",
				 contact, _id, _ademco_id);
	CAlarmMachineManager* mgr = CAlarmMachineManager::GetInstance();
	BOOL ok = mgr->ExecuteSql(query);
	if (ok) {
		set_contact(contact);
		return true;
	}

	return false;
}


bool CAlarmMachine::execute_set_address(const wchar_t* address)
{
	AUTO_LOG_FUNCTION;
	CString query;
	query.Format(L"update AlarmMachine set address='%s' where id=%d and ademco_id=%d",
				 address, _id, _ademco_id);

	CAlarmMachineManager* mgr = CAlarmMachineManager::GetInstance();
	BOOL ok = mgr->ExecuteSql(query);
	if (ok) {
		set_address(address);
		return true;
	}

	return false;
}


bool CAlarmMachine::execute_set_phone(const wchar_t* phone)
{
	AUTO_LOG_FUNCTION;
	CString query;
	query.Format(L"update AlarmMachine set phone='%s' where id=%d and ademco_id=%d",
				 phone, _id, _ademco_id);

	CAlarmMachineManager* mgr = CAlarmMachineManager::GetInstance();
	BOOL ok = mgr->ExecuteSql(query);
	if (ok) {
		set_phone(phone);
		return true;
	}

	return false;
}


bool CAlarmMachine::execute_set_phone_bk(const wchar_t* phone_bk)
{
	AUTO_LOG_FUNCTION;
	CString query;
	query.Format(L"update AlarmMachine set phone_bk='%s' where id=%d and ademco_id=%d",
				 phone_bk, _id, _ademco_id);
	CAlarmMachineManager* mgr = CAlarmMachineManager::GetInstance();
	BOOL ok = mgr->ExecuteSql(query);
	if (ok) {
		set_phone_bk(phone_bk);
		return true;
	}

	return false;
}


bool CAlarmMachine::execute_set_group_id(int group_id)
{
	AUTO_LOG_FUNCTION;
	CString query;
	query.Format(L"update AlarmMachine set group_id=%d where id=%d and ademco_id=%d",
				 group_id, _id, _ademco_id);
	CAlarmMachineManager* mgr = CAlarmMachineManager::GetInstance();
	BOOL ok = mgr->ExecuteSql(query);
	if (ok) {
		CGroupManager* group_mgr = CGroupManager::GetInstance();
		CGroupInfoPtr old_group = group_mgr->GetGroupInfo(_group_id);
		CGroupInfoPtr new_group = group_mgr->GetGroupInfo(group_id);
		old_group->RemoveChildMachine(shared_from_this());
		set_group_id(group_id);
		new_group->AddChildMachine(shared_from_this());
		return true;
	}

	return false;
}


bool CAlarmMachine::execute_add_zone(const CZoneInfoPtr& zoneInfo)
{
	CString query;
	if (_is_submachine) {
		zoneInfo->set_sub_machine_id(_id);
		query.Format(L"insert into SubZone ([sub_zone],[sub_machine_id],[alias],[detector_info_id]) values(%d,%d,'%s',%d)",
					 zoneInfo->get_sub_zone(), _id, zoneInfo->get_alias(), 
					 zoneInfo->get_detector_id());
	} else {
		query.Format(L"insert into ZoneInfo ([ademco_id],[zone_value],[type],[detector_info_id],[sub_machine_id],[alias],[status_or_property],[physical_addr]) values(%d,%d,%d,%d,%d,'%s',%d,%d)",
					 _ademco_id, zoneInfo->get_zone_value(),
					 zoneInfo->get_type(), zoneInfo->get_detector_id(), -1,
					 zoneInfo->get_alias(), zoneInfo->get_status_or_property(), 
					 zoneInfo->get_physical_addr());
	}
	CAlarmMachineManager* mgr = CAlarmMachineManager::GetInstance();
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


bool CAlarmMachine::execute_del_zone(const CZoneInfoPtr& zoneInfo)
{
	AUTO_LOG_FUNCTION;
	CString query;
	if (_is_submachine) {
		query.Format(L"delete from SubZone where id=%d", zoneInfo->get_id());
	} else {
		query.Format(L"delete from ZoneInfo where id=%d", zoneInfo->get_id());
	}
	CAlarmMachineManager* mgr = CAlarmMachineManager::GetInstance();
	BOOL ok = mgr->ExecuteSql(query);
	if (ok) {
		mgr->DeleteVideoBindInfoByZoneInfo(zoneInfo);
		CDetectorInfoPtr detInfo = zoneInfo->GetDetectorInfo();
		if (detInfo) {
			query.Format(L"delete from DetectorInfo where id=%d", detInfo->get_id());
			VERIFY(mgr->ExecuteSql(query));
		}

		CMapInfoPtr mapInfo = zoneInfo->GetMapInfo();
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


void CAlarmMachine::GetAllZoneInfo(CZoneInfoList& list)
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


void CAlarmMachine::GetAllMapInfo(CMapInfoList& list)
{
	std::copy(_mapList.begin(), _mapList.end(), std::back_inserter(list));
}


void CAlarmMachine::AddZone(const CZoneInfoPtr& zoneInfo)
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

		CDetectorInfoPtr detector = zoneInfo->GetDetectorInfo();
		if (detector) {
			int map_id = detector->get_map_id();
			CMapInfoPtr mapInfo = GetMapInfo(map_id);
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


CZoneInfoPtr CAlarmMachine::GetZone(int zone)
{
	if (0 <= zone && zone < MAX_MACHINE_ZONE) {
		auto iter = _zoneMap.find(zone);
		if (iter != _zoneMap.end()) {
			return iter->second;
		}
	} 

	return nullptr;
}


bool CAlarmMachine::execute_add_map(const core::CMapInfoPtr& mapInfo)
{
	MapType mt = _is_submachine ? MAP_SUB_MACHINE : MAP_MACHINE;
	mapInfo->set_type(mt);
	mapInfo->set_machine_id(_is_submachine ? _id : _ademco_id);

	CString query;
	query.Format(L"insert into MapInfo ([type],[machine_id],[alias],[path]) values(%d,%d,'%s','%s')",
				 mt, mapInfo->get_machine_id(), mapInfo->get_alias(),
				 mapInfo->get_path());
	CAlarmMachineManager* mgr = CAlarmMachineManager::GetInstance();
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


bool CAlarmMachine::execute_update_map_alias(const core::CMapInfoPtr& mapInfo, const wchar_t* alias)
{
	AUTO_LOG_FUNCTION;
	ASSERT(mapInfo);
	CString query;
	query.Format(L"update MapInfo set alias='%s' where id=%d", alias, mapInfo->get_id());
	CAlarmMachineManager* mgr = CAlarmMachineManager::GetInstance();
	if (mgr->ExecuteSql(query)) {
		mapInfo->set_alias(alias);
		return true;
	} else {
		ASSERT(0); JLOG(L"update map alias failed.\n");
		return false;
	}
}


bool CAlarmMachine::execute_update_map_path(const core::CMapInfoPtr& mapInfo, const wchar_t* path)
{
	AUTO_LOG_FUNCTION;
	ASSERT(mapInfo);
	CString query;
	query.Format(L"update MapInfo set path='%s' where id=%d", path, mapInfo->get_id());
	CAlarmMachineManager* mgr = CAlarmMachineManager::GetInstance();
	if (mgr->ExecuteSql(query)) {
		mapInfo->set_path(path);
		return true;
	} else {
		ASSERT(0); JLOG(L"update map alias failed.\n");
		return false;
	}
}


bool CAlarmMachine::execute_delete_map(const core::CMapInfoPtr& mapInfo)
{
	AUTO_LOG_FUNCTION;
	ASSERT(mapInfo && (-1 != mapInfo->get_id()));
	CString query;
	CAlarmMachineManager* mgr = CAlarmMachineManager::GetInstance();
	do {
		query.Format(L"delete from MapInfo where id=%d", mapInfo->get_id());
		if (!mgr->ExecuteSql(query)) {
			JLOG(L"delete map failed.\n"); break;
		}

		query.Format(L"update DetectorInfo set map_id=-1 where map_id=%d",
					 mapInfo->get_id());
		if (!mgr->ExecuteSql(query)) {
			JLOG(L"update DetectorInfo failed.\n"); break;
		}

		std::list<CDetectorBindInterfacePtr> list;
		mapInfo->GetAllInterfaceInfo(list);
		for (auto pInterface : list) {
			if (DIT_ZONE_INFO == pInterface->GetInterfaceType()) {
				auto zoneInfo = std::dynamic_pointer_cast<CZoneInfo>(pInterface);
				_unbindZoneMap->AddInterface(zoneInfo);
				zoneInfo->SetMapInfo(_unbindZoneMap);
				zoneInfo->execute_del_detector_info();
			} else if (DIT_CAMERA_INFO == pInterface->GetInterfaceType()) {
				CCameraInfoPtr cam = std::dynamic_pointer_cast<CCameraInfo>(pInterface);
				mgr->DeleteCameraInfo(cam);
			}
		}

		_mapList.remove(mapInfo);
		mgr->DeleteMapInfo(mapInfo);
		return true;
	} while (0);
	return false;
}


bool CAlarmMachine::execute_update_expire_time(const COleDateTime& datetime)
{
	AUTO_LOG_FUNCTION;
	CString query;
	CAlarmMachineManager* mgr = CAlarmMachineManager::GetInstance();
	do {
		if (_is_submachine) {
			query.Format(L"update SubMachine set expire_time='%s' where id=%d",
						 datetime.Format(L"%Y-%m-%d %H:%M:%S"), _id);
		} else {
			query.Format(L"update AlarmMachine set expire_time='%s' where id=%d",
						 datetime.Format(L"%Y-%m-%d %H:%M:%S"), _id);
		}
		if (!mgr->ExecuteSql(query)) {
			JLOG(L"update expire_time failed.\n"); break;
		}

		_expire_time = datetime;
		return true;
	} while (0);
	return false;
}


bool CAlarmMachine::execute_set_coor(const web::BaiduCoordinate& coor)
{
	AUTO_LOG_FUNCTION;
	CString query;
	CAlarmMachineManager* mgr = CAlarmMachineManager::GetInstance();
	do {
		if (_is_submachine) {
			query.Format(L"update SubMachine set baidu_x=%f,baidu_y=%f where id=%d",
						 coor.x, coor.y, _id);
		} else {
			query.Format(L"update AlarmMachine set baidu_x=%f,baidu_y=%f where id=%d",
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


void CAlarmMachine::inc_submachine_count()
{ 
	AUTO_LOG_FUNCTION;
	_submachine_count++;
	auto t = time(nullptr);
	auto ademcoEvent = std::make_shared<AdemcoEvent>(ES_UNKNOWN, EVENT_SUBMACHINECNT, 0, 0, t, t);
	notify_observers(ademcoEvent);
}


void CAlarmMachine::dec_submachine_count()
{ 
	//AUTO_LOG_FUNCTION;
	_submachine_count--;
	auto t = time(nullptr);
	auto ademcoEvent = std::make_shared<AdemcoEvent>(ES_UNKNOWN, EVENT_SUBMACHINECNT, 0, 0, t, t);
	notify_observers(ademcoEvent);
}


void CAlarmMachine::inc_alarmingSubMachineCount()
{
	_alarmingSubMachineCount++;
}


void CAlarmMachine::dec_alarmingSubMachineCount()
{
	if (_alarmingSubMachineCount == 0)
		return;

	if (--_alarmingSubMachineCount == 0 && !_has_alarming_direct_zone) {
		clear_ademco_event_list();
	}
}


void CAlarmMachine::set_highestEventLevel(EventLevel level)
{
	if (level > _highestEventLevel)
		_highestEventLevel = level;
}




NAMESPACE_END
