#include "stdafx.h"
#include "AlarmCenter.h"
#include "AlarmMachineManager.h"
#include "AlarmMachine.h"
#include "ademco_func.h"
#include "MapInfo.h"
#include "ZoneInfo.h"
#include "CameraInfo.h"
#include "DetectorInfo.h"
#include "DetectorLib.h"
#include "ConfigHelper.h"
#include "NetworkConnector.h"
#include "UserInfo.h"
#include "HistoryRecord.h"
#include "GroupInfo.h"
#include "AlarmCenterDlg.h"
#include "baidu.h"
#include "CsrInfo.h"
#include <memory> // for std::shared_ptr
#include <algorithm>
#include "InputDlg.h" // todo: remove this
#include "../contrib/sqlitecpp/SQLiteCpp.h"
#include "../video/video.h"
#include "alarm_center_video_service.h"
#include "VideoManager.h"
#include "ConfigHelper.h"
#include "consumer.h"

namespace core {
using namespace SQLite;

static const int ONE_MINUTE = 60 * 1000;
static const int ONE_HOUR = 60 * ONE_MINUTE;
//#ifdef _DEBUG
//static const int MAX_SUBMACHINE_ACTION_TIME_OUT = 16 * ONE_MINUTE;
//static const int CHECK_GAP = ONE_MINUTE;
//static const int TRY_LOCK_RETRY_GAP = ONE_MINUTE;
//static const int WAIT_TIME_FOR_RETRIEVE_RESPONCE = 3000;
//#else
static const int MAX_SUBMACHINE_ACTION_TIME_OUT = 16 * ONE_HOUR; // 16 hour
static const int CHECK_GAP = 16 * ONE_HOUR;
static const int TRY_LOCK_RETRY_GAP = ONE_MINUTE;
static const int WAIT_TIME_FOR_RETRIEVE_RESPONCE = ONE_MINUTE;
//#endif


//IMPLEMENT_SINGLETON(alarm_machine_manager)

alarm_machine_manager::alarm_machine_manager()
	: m_pPrevCallDisarmWnd(nullptr)
	, m_prevCallDisarmAdemcoID(-1)
	, m_prevCallDisarmGG(-1)
	, m_prevCallDisarmZoneValue(-1)
#ifdef USE_ARRAY
	, m_curMachinePos(0)
	, m_validMachineCount(0)
#endif
	, m_hEventOotebm(INVALID_HANDLE_VALUE)
{
	AUTO_LOG_FUNCTION;
#ifdef USE_ARRAY
	//size_t sss = sizeof(m_alarmMachines);
	memset(m_alarmMachines, 0, sizeof(m_alarmMachines));
#endif
	

	m_hEventOotebm = CreateEvent(nullptr, TRUE, FALSE, nullptr);

	thread_ = std::thread(&alarm_machine_manager::ThreadCheckSubMachine, this);
}


alarm_machine_manager::~alarm_machine_manager()
{
	AUTO_LOG_FUNCTION;
	{
		std::lock_guard<std::mutex> lock(mutex_);
		running_ = false;				
	}

	condvar_.notify_one();
	thread_.join();

	CLOSEHANDLE(m_hEventOotebm);

	m_machineMap.clear();
	for (auto iter : m_cameraMap) {
		iter.second.clear();
	}
	m_cameraMap.clear();

	detector_lib_manager::release_singleton();
	group_manager::release_singleton();
	consumer_manager::release_singleton();
}


void alarm_machine_manager::LoadFromDB(void* udata, LoadDBProgressCB cb)
{
	AUTO_LOG_FUNCTION;
	InitDB();
	InitCsrInfo();
	InitDetectorLib();
	LoadDetectorLibFromDB();
	//LoadZonePropertyInfoFromDB();
	LoadGroupInfoFromDB();

	//TestLoadAlarmMachineFromDB(udata, cb);
	LoadAlarmMachineFromDB(udata, cb);

	group_manager::get_instance()->_tree->SortDescendantMachines(group_manager::get_instance()->get_cur_sort_machine_way());

	LoadCameraInfoFromDB();

	LoadServiceInfoFromDB();

}


void alarm_machine_manager::LoadServiceInfoFromDB()
{
	auto mgr = consumer_manager::get_instance();
	auto list = mgr->load_consumers();
	auto iter = list.begin();
	while (iter != list.end()) {
		auto consumer = *iter;
		auto machine = GetMachine(consumer->ademco_id);
		assert(machine);
		auto target_machine = machine;
		if (machine) {
			if (0 != consumer->zone_value) {
				auto zone = machine->GetZone(consumer->zone_value); assert(zone);
				if (zone) {
					auto sub_machine = zone->GetSubMachineInfo(); assert(sub_machine);
					if (sub_machine) {
						target_machine = sub_machine;
					}
				}
			}
		}

		if (target_machine) {
			target_machine->set_consumer(consumer);
			iter = list.erase(iter);
		} else {
			assert(0);
			iter++;
		}
	}

	assert(list.empty());

	// set default consumer type to exsiting machines
	auto consumer_type_shop = mgr->get_consumer_type_by_id(1); // shop
	assert(consumer_type_shop);
	if (consumer_type_shop) {
		for (auto machine_iter : m_machineMap) {
			auto machine = machine_iter.second;
			if (!machine->get_consumer()) {
				auto a_consumer = mgr->execute_add_consumer(machine->get_ademco_id(), 0, 
															consumer_type_shop, 0, 0, machine->get_expire_time()); 
				assert(a_consumer);
				if (a_consumer) {
					machine->set_consumer(a_consumer);
				}
			}

			zone_info_list zones;
			machine->GetAllZoneInfo(zones);
			for (auto zone : zones) {
				auto sub_machine = zone->GetSubMachineInfo();
				if (sub_machine && !sub_machine->get_consumer()) {
					auto a_consumer = mgr->execute_add_consumer(machine->get_ademco_id(), zone->get_zone_value(), 
																consumer_type_shop, 0, 0, sub_machine->get_expire_time());
					assert(a_consumer);
					if (a_consumer) {
						sub_machine->set_consumer(a_consumer);
					}
				}
			}
		}
	}
}


void alarm_machine_manager::LoadSmsConfigFromDB(const core::alarm_machine_ptr& machine)
{
	AUTO_LOG_FUNCTION;
	CString sql = L"";
	sql.Format(L"select id,report_alarm,report_exception,report_status,report_alarm_bk,report_exception_bk,report_status_bk from table_sms_config where is_submachine=%d and ademco_id=%d and zone_value=%d",
			   machine->get_is_submachine(), machine->get_ademco_id(), machine->get_is_submachine() ? machine->get_submachine_zone() : -1);

	auto sqla = utf8::w2a((LPCTSTR(sql)));
	try {
		Statement query(*db_, sqla);
		if (query.executeStep()) {
			int ndx = 0;
			sms_config cfg = {};
			cfg.id = static_cast<int>(query.getColumn(ndx++));
			cfg.report_alarm = query.getColumn(ndx++).getInt() > 0;
			cfg.report_exception = query.getColumn(ndx++).getInt() > 0;
			cfg.report_status = query.getColumn(ndx++).getInt() > 0;
			cfg.report_alarm_bk = query.getColumn(ndx++).getInt() > 0;
			cfg.report_exception_bk = query.getColumn(ndx++).getInt() > 0;
			cfg.report_status_bk = query.getColumn(ndx++).getInt() > 0;
			machine->set_sms_cfg(cfg);
		} else {
			// add sms config
			CreateSmsConfigForMachine(machine);
		}
	} catch (std::exception& e) {
		JLOGA(e.what());
	}
}


void alarm_machine_manager::InitCsrInfo()
{
	csr_manager::get_instance();
}


BOOL alarm_machine_manager::ExecuteSql(const CString& sql)
{
	try {
		return db_->exec(utf8::w2a((LPCTSTR)sql)) > 0;
	} catch (std::exception& e) {
		JLOG(sql);
		JLOGA(e.what());
		return FALSE;
	}
}


int alarm_machine_manager::AddAutoIndexTableReturnID(const CString& sql)
{
	try {
		if (db_->exec(utf8::w2a((LPCTSTR)sql)) > 0) {
			int id = static_cast<int>(db_->getLastInsertRowid());
			return id;
		}
	} catch (std::exception& e) {
		JLOG(sql);
		JLOGA(e.what());
	}
	return -1;
}


void alarm_machine_manager::InitDB()
{
	auto path = get_config_path() + "\\center.db3";
	db_ = std::make_shared<Database>(path, SQLITE_OPEN_READWRITE | SQLITE_OPEN_CREATE);
	assert(db_);
	if (!db_) { return; }

	try {
		// check if db empty
		{
			Statement query(*db_, "select name from sqlite_master where type='table'");
			if (!query.executeStep()) {
				// init tables
				db_->exec("drop table if exists table_machine");
				db_->exec("create table table_machine (id integer primary key AUTOINCREMENT, \
ademco_id integer, \
group_id integer, \
banned integer, \
machine_status integer, \
machine_type integer, \
machine_name text, \
contact text, \
address text, \
phone text, \
phone_bk text, \
expire_time text, \
map_coor_x real, \
map_coor_y real, \
map_zoom_level integer, \
auto_show_map_when_alarm integer)");

				db_->exec("drop table if exists table_camera");
				db_->exec("create table table_camera (id integer primary key AUTOINCREMENT, \
ademco_id integer, \
sub_machine_id ingeger, \
map_id integer, \
x integer, \
y integer, \
distance integer, \
angle integer, \
detector_lib_id integer, \
device_info_id integer, \
device_productor integer)");

				db_->exec("drop table if exists table_detector");
				db_->exec("create table table_detector (id integer primary key AUTOINCREMENT, \
map_id integer, \
zone_info_id integer, \
x integer, y integer, \
distance integer, \
angle integer, \
detector_lib_id integer)");

				db_->exec("drop table if exists table_detector_lib");
				db_->exec("create table table_detector_lib (id integer primary key AUTOINCREMENT, \
type integer, \
detector_name text, \
detector_pic_path text, \
pair_pic_path text, \
antline_count integer, \
antline_gap integer)");

				db_->exec("drop table if exists table_group");
				db_->exec("create table table_group (id integer primary key AUTOINCREMENT, \
group_name text, \
parent_group_id integer)");
				db_->exec("insert into table_group values(NULL,\"root\", 0)");

				db_->exec("drop table if exists table_map");
				db_->exec("create table table_map (id integer primary key AUTOINCREMENT, \
type integer, \
machine_id integer, \
map_name text, \
map_pic_path text)");

				db_->exec("drop table if exists table_sub_machine");
				db_->exec("create table table_sub_machine (id integer primary key AUTOINCREMENT, \
machine_status integer, \
contact text, \
address text, \
phone text, \
phone_bk text, \
expire_time text, \
map_coor_x real, \
map_coor_y real, \
map_zoom_level integer, \
auto_show_map_when_alarm integer)");

				db_->exec("drop table if exists table_sub_zone");
				db_->exec("create table table_sub_zone (id integer primary key AUTOINCREMENT, \
sub_machine_id integer, \
sub_zone_value integer, \
zone_name text, \
detector_info_id integer)");

				db_->exec("drop table if exists table_zone");
				db_->exec("create table table_zone (id integer primary key AUTOINCREMENT, \
ademco_id integer, \
sub_machine_id integer, \
zone_value integer, \
type integer, \
zone_name text, \
status_or_property integer, \
physical_addr integer, \
detector_info_id integer)");

				db_->exec("drop table if exists table_sms_config");
				db_->exec("create table table_sms_config (id integer primary key AUTOINCREMENT, \
is_submachine integer, \
ademco_id integer, \
zone_value integer,  \
report_alarm integer, \
report_exception integer, \
report_status integer, \
report_alarm_bk integer, \
report_exception_bk integer, \
report_status_bk integer)");


			} else {
				std::string name = query.getColumn(0);
				JLOGA(name.c_str());
				while (query.executeStep()) {
					name = query.getColumn(0).getText();
					JLOGA(name.c_str());
				}
			}
		}
	} catch (std::exception& e) {
		JLOGA(e.what());
	}
}


static const TCHAR* TRIPLE_CONDITION(int condition, const TCHAR* a,
									 const TCHAR* b, const TCHAR* c)
{
	switch (condition) {
		case 0: return a; break;
		case 1: return b; break;
		case 2: return c; break;
		default: ASSERT(0); return _T(""); break;
	}
}


void alarm_machine_manager::InitDetectorLib()
{
	AUTO_LOG_FUNCTION;
	try {
		db_->exec("delete from table_detector_lib");
		db_->exec("update sqlite_sequence set seq=0 where name='table_detector_lib'");

		int condition = 0;

		util::ApplicationLanguage lang = util::CConfigHelper::get_instance()->get_current_language();
		switch (lang) {
		case util::AL_CHINESE:condition = 0;
			break;
		case util::AL_ENGLISH:condition = 2;
			break;
		case util::AL_TAIWANESE:condition = 1;
			break;
		default:ASSERT(0);
			break;
		}

		CString detPath = _T("");
		detPath.Format(_T("%s\\Detectors\\"), get_exe_path().c_str());

		CString format, sql;
		format = L"insert into table_detector_lib ([type],[detector_name],[detector_pic_path],[pair_pic_path],[antline_count],[antline_gap]) \
values(%d,'%s','%s','%s',%d,%d)";

		// 二光束对射A2
		sql.Format(format, DT_DOUBLE, _T("二光束对射A2"), detPath + _T("A2.bmp"),
				   detPath + L"A2Receiver.bmp", ALN_2, ALG_12);
		VERIFY(ExecuteSql(sql));

		// 四光束庭院灯A4
		sql.Format(format, DT_DOUBLE, _T("四光束庭院灯A4"), detPath + _T("A4.bmp"),
				   detPath + L"A4Receiver.bmp", ALN_4, ALG_12);
		VERIFY(ExecuteSql(sql));

		// 八光束庭院灯A8
		sql.Format(format, DT_DOUBLE, _T("八光束庭院灯A8"), detPath + _T("A8.bmp"),
				   detPath + L"A8Receiver.bmp", ALN_8, ALG_12);
		VERIFY(ExecuteSql(sql));

		// 二光束对射R2
		sql.Format(format, DT_DOUBLE, _T("二光束对射R2"), detPath + _T("R2.bmp"),
				   detPath + L"R2Receiver.bmp", ALN_2, ALG_12);
		VERIFY(ExecuteSql(sql));

		// 三光束对射R3
		sql.Format(format, DT_DOUBLE, _T("三光束对射R3"), detPath + _T("R3.bmp"),
				   detPath + L"R3Receiver.bmp", ALN_3, ALG_12);
		VERIFY(ExecuteSql(sql));

		// 四光束光墙R4
		sql.Format(format, DT_DOUBLE, _T("四光束光墙R4"), detPath + _T("R4.bmp"),
				   detPath + L"R4Receiver.bmp", ALN_4, ALG_12);
		VERIFY(ExecuteSql(sql));

		// 六光束光墙R6
		sql.Format(format, DT_DOUBLE, _T("六光束光墙R6"), detPath + _T("R6.bmp"),
				   detPath + L"R6Receiver.bmp", ALN_6, ALG_12);
		VERIFY(ExecuteSql(sql));

		// 八光束光墙R8
		sql.Format(format, DT_DOUBLE, _T("八光束光墙R8"), detPath + _T("R8.bmp"),
				   detPath + L"R8Receiver.bmp", ALN_8, ALG_16);
		VERIFY(ExecuteSql(sql));

		// 四光束光栅S4
		sql.Format(format, DT_DOUBLE, _T("四光束光栅S4"), detPath + _T("S4.bmp"),
				   detPath + L"S4Receiver.bmp", ALN_4, ALG_12);
		VERIFY(ExecuteSql(sql));

		// 四光束庭院灯S4-D
		sql.Format(format, DT_DOUBLE, _T("四光束庭院灯S4-D"), detPath + _T("S4-D.bmp"),
				   detPath + L"S4-DReceiver.bmp", ALN_4, ALG_12);
		VERIFY(ExecuteSql(sql));

		// 八光束光栅S8
		sql.Format(format, DT_DOUBLE, _T("八光束光栅S8"), detPath + _T("S8.bmp"),
				   detPath + L"S8Receiver.bmp", ALN_8, ALG_14);
		VERIFY(ExecuteSql(sql));

		// 八光束庭院灯S8-D
		sql.Format(format, DT_DOUBLE, _T("八光束庭院灯S8-D"), detPath + _T("S8-D.bmp"),
				   detPath + L"S8-DReceiver.bmp", ALN_8, ALG_14);
		VERIFY(ExecuteSql(sql));

		// 广角探测器T205
		sql.Format(format, DT_SINGLE, _T("广角探测器T205"), detPath + _T("T205.bmp"),
				   L"", ALN_0, ALG_0);
		VERIFY(ExecuteSql(sql));

		// 户外警号灯JHD-2
		sql.Format(format, DT_SINGLE, _T("户外警号灯JHD-2"), detPath + _T("JHD-2.bmp"),
				   L"", ALN_0, ALG_0);
		VERIFY(ExecuteSql(sql));

		// 红外幕帘T201
		sql.Format(format, DT_SINGLE, _T("红外幕帘T201"), detPath + _T("T201.bmp"),
				   L"", ALN_0, ALG_0);
		VERIFY(ExecuteSql(sql));

		// 光电烟感T601
		sql.Format(format, DT_SINGLE, _T("光电烟感T601"), detPath + _T("T601.bmp"),
				   L"", ALN_0, ALG_0);
		VERIFY(ExecuteSql(sql));

		// 无线门磁
		sql.Format(format, DT_SINGLE,
				   TRIPLE_CONDITION(condition, _T("无线门磁"), _T("無線門磁"), _T("WirelessDoorSensor")),
				   detPath + _T("WirelessDoorSensor.bmp"), L"", ALN_0, ALG_0);
		VERIFY(ExecuteSql(sql));

		// 紧急按钮HB-A380
		sql.Format(format, DT_SINGLE,
				   TRIPLE_CONDITION(condition, _T("紧急按钮HB-A380"), _T("緊急按鈕HB-A380"), _T("EmergencyButtonHB-A380")),
				   detPath + _T("EmergencyButtonHB-A380.bmp"), L"", ALN_0, ALG_0);
		VERIFY(ExecuteSql(sql));

		// 卧室主机HB-3030C
		sql.Format(format, DT_SUB_MACHINE,
				   TRIPLE_CONDITION(condition, _T("卧式主机HB-3030C"), _T("臥式主機HB-3030C"), _T("HB-3030C")),
				   detPath + _T("HB-3030C.bmp"), L"", ALN_0, ALG_0);
		VERIFY(ExecuteSql(sql));

		// 液晶主机HB-BJQ-560
		sql.Format(format, DT_SINGLE,
				   TRIPLE_CONDITION(condition, _T("液晶主机HB-BJQ-560"), _T("液晶主機HB-BJQ-560"), _T("HB-BJQ-560")),
				   detPath + _T("HB-BJQ-560.bmp"), L"", ALN_0, ALG_0);
		VERIFY(ExecuteSql(sql));

		// 防漏水报警器HB-L188
		sql.Format(format, DT_SINGLE,
				   TRIPLE_CONDITION(condition, _T("防漏水报警器HB-L188"), _T("防漏水報警器HB-BJQ-560"), _T("HB-L188")),
				   detPath + _T("HB-L188.bmp"), L"", ALN_0, ALG_0);
		VERIFY(ExecuteSql(sql));

		// 室内光能无线门磁HB-MC-2
		sql.Format(format, DT_SINGLE,
				   TRIPLE_CONDITION(condition, _T("室内光能无线门磁HB-MC-2"), _T("室內光能無線門磁HB-MC-2"), _T("HB-MC-2")),
				   detPath + _T("HB-MC-2.bmp"), L"", ALN_0, ALG_0);
		VERIFY(ExecuteSql(sql));

		// 双鉴红外探测器HB-T206
		sql.Format(format, DT_SINGLE,
				   TRIPLE_CONDITION(condition, _T("双鉴红外探测器HB-T206"), _T("雙鑑紅外探測器HB-T206"), _T("HB-T206")),
				   detPath + _T("HB-T206.bmp"), L"", ALN_0, ALG_0);
		VERIFY(ExecuteSql(sql));

		// 吸顶式红外探测器HB-T305
		sql.Format(format, DT_SINGLE,
				   TRIPLE_CONDITION(condition, _T("吸顶式红外探测器HB-T305"), _T("吸頂式紅外探測器HB-BJQ-560"), _T("HB-T305")),
				   detPath + _T("HB-T305.bmp"), L"", ALN_0, ALG_0);
		VERIFY(ExecuteSql(sql));

		// 防燃气泄漏报警器HB-T501
		sql.Format(format, DT_SINGLE,
				   TRIPLE_CONDITION(condition, _T("防燃气泄漏报警器HB-T501"), _T("防燃氣洩漏報警器HB-BJQ-560"), _T("HB-T501")),
				   detPath + _T("HB-T501.bmp"), L"", ALN_0, ALG_0);
		VERIFY(ExecuteSql(sql));

		// 户外幕帘探测器HB-T701
		sql.Format(format, DT_SINGLE,
				   TRIPLE_CONDITION(condition, _T("户外幕帘探测器HB-T701"), _T("戶外幕簾探測器HB-T701"), _T("HB-T701")),
				   detPath + _T("HB-T701.bmp"), L"", ALN_0, ALG_0);
		VERIFY(ExecuteSql(sql));

		// 摄像机
		sql.Format(format, DT_CAMERA,
				   TRIPLE_CONDITION(condition, _T("摄像机"), _T("摄像机"), _T("Camera")),
				   detPath + _T("camera_72px.bmp"), L"", ALN_0, ALG_0);
		VERIFY(ExecuteSql(sql));

	} catch (std::exception& e) {
		JLOGA(e.what());
	}
}


void alarm_machine_manager::LoadGroupInfoFromDB()
{
	auto mgr = group_manager::get_instance(); 
	std::list<group_info_ptr> unresolvedGroupList;

	Statement query(*db_, "select * from table_group order by parent_group_id");
	while(query.executeStep()) {
		int id, parent_id, ndx = 0;
		id = static_cast<int>(query.getColumn(ndx++));
		std::wstring name = utf8::a2w(query.getColumn(ndx++).getText());
		parent_id = query.getColumn(ndx++);
			
		if (id == 1 && parent_id == 0) {
			mgr->_tree->set_id(1);
			mgr->_tree->set_parent_id(0);
			CString rootName;
			rootName = TR(IDS_STRING_GROUP_ROOT);
			mgr->_tree->set_group_name(rootName);
		} else {
			auto group = std::make_shared<group_info>();
			group->set_id(id);
			group->set_parent_id(parent_id);
			group->set_group_name(name.c_str());

			bool ok = mgr->_tree->AddChildGroup(group);
			if (!ok) {
				unresolvedGroupList.push_back(group);
			}
		}
	}

	while (!unresolvedGroupList.empty()) {
		auto iter = unresolvedGroupList.begin();
		while (iter != unresolvedGroupList.end()) {
			bool ok = mgr->_tree->AddChildGroup(*iter);
			if (ok) {
				iter = unresolvedGroupList.erase(iter);
			} else {
				iter++;
			}
		}
	}

	mgr->_tree->SortDescendantGroupsByName();
}


void alarm_machine_manager::LoadAlarmMachineFromDB(void* udata, LoadDBProgressCB cb)
{
	AUTO_LOG_FUNCTION;

	// get machine count
	int count = MAX_MACHINE;
	{
		Statement query(*db_, "select count(*) from table_machine");
		if (query.executeStep()) {
			count = query.getColumn(0);
		}
	}

	Statement query(*db_, "select * from table_machine order by ademco_id");
	auto mgr = group_manager::get_instance();
	int i = 0;

	while (query.executeStep())	{
		long id, ademco_id, group_id;
		int banned, status, type;
		std::string name, contact, address, phone, phone_bk, expire_time;
		double x, y; int zoom_level, auto_show_map_when_alarming;

		int ndx = 0;
		
		id = static_cast<int>(query.getColumn(ndx++));
		ademco_id = query.getColumn(ndx++);
		if (ademco_id < MIN_MACHINE || MAX_MACHINE <= ademco_id)
			continue;
		group_id = query.getColumn(ndx++);
		banned = query.getColumn(ndx++);
		status = query.getColumn(ndx++);
		type = query.getColumn(ndx++);

		name = query.getColumn(ndx++).getText();
		contact = query.getColumn(ndx++).getText();
		address = query.getColumn(ndx++).getText();
		phone = query.getColumn(ndx++).getText();
		phone_bk = query.getColumn(ndx++).getText();
		expire_time = query.getColumn(ndx++).getText();

		x = query.getColumn(ndx++).getDouble();
		y = query.getColumn(ndx++).getDouble();
		zoom_level = query.getColumn(ndx++);
		auto_show_map_when_alarming = query.getColumn(ndx++);

		auto machine = std::make_shared<alarm_machine>();
		machine->set_id(id);
		machine->set_ademco_id(ademco_id);
		machine->set_group_id(group_id);
		machine->set_banned(banned != 0);
		machine->set_machine_status(Integer2MachineStatus(status));
		machine->set_machine_type(Integer2MachineType(type));
		
		machine->set_alias(utf8::a2w(name).c_str());
		machine->set_contact(utf8::a2w(contact).c_str());
		machine->set_address(utf8::a2w(address).c_str());
		machine->set_phone(utf8::a2w(phone).c_str());
		machine->set_phone_bk(utf8::a2w(phone_bk).c_str());
		machine->set_expire_time(string_to_time_point(expire_time));
		machine->set_coor(web::BaiduCoordinate(x, y));
		machine->set_zoomLevel(zoom_level);
		machine->set_auto_show_map_when_start_alarming(auto_show_map_when_alarming != 0);
			
		LoadSmsConfigFromDB(machine);

		m_machineMap[ademco_id] = machine;

		if (cb && udata) {
			auto progress = std::make_shared<progress_ex>();
			progress->main = true;
			progress->progress = static_cast<int>(i++ * MAX_MACHINE / count);
			progress->value = ademco_id;
			progress->total = count;
			cb(udata, progress);
		}

		LoadMapInfoFromDB(machine);
		LoadZoneInfoFromDB(machine, udata, cb);
		bool ok = mgr->_tree->AddChildMachine(machine);
		VERIFY(ok);
	}

	if (cb && udata) {
		auto progress = std::make_shared<progress_ex>();
		progress->main = true;
		progress->progress = MAX_MACHINE;
		progress->value = MAX_MACHINE;
		progress->total = MAX_MACHINE;
		cb(udata, progress);
	}
}


void alarm_machine_manager::LoadMapInfoFromDB(const core::alarm_machine_ptr& machine)
{
	AUTO_LOG_FUNCTION;

	CString sql;
	sql.Format(L"select * from table_map where type=%d and machine_id=%d order by id", 
			   machine->get_is_submachine() ? MAP_SUB_MACHINE : MAP_MACHINE,
			   machine->get_is_submachine() ? machine->get_id() : machine->get_ademco_id());
	Statement query(*db_, utf8::w2a((LPCTSTR)sql));
	
	while (query.executeStep())	{
		long id, machine_id, type;
		std::string alias, path; 
		
		int ndx = 0;

		id = static_cast<int>(query.getColumn(ndx++));
		type = query.getColumn(ndx++);
		machine_id = query.getColumn(ndx++);
		alias = query.getColumn(ndx++).getText();
		path = query.getColumn(ndx++).getText();

		map_info_ptr mapInfo = std::make_shared<map_info>();
		mapInfo->set_id(id);
		mapInfo->set_type(type);
		mapInfo->set_machine_id(machine_id);
		auto w = utf8::a2w(alias);
		if (w.empty()) {
			auto s = TR(IDS_STRING_ZONE);
			s.AppendFormat(L"%d", id);
			w = (LPCTSTR)s;
		}
		mapInfo->set_alias(w.c_str());
		mapInfo->set_path(utf8::a2w(path).c_str());

		machine->AddMap(mapInfo);
		m_mapInfoMap[id] = mapInfo;
	}
}


void alarm_machine_manager::LoadZoneInfoFromDB(const core::alarm_machine_ptr& machine, void* udata, LoadDBProgressCB cb)
{
	AUTO_LOG_FUNCTION;
	auto subProgress = std::make_shared<progress_ex>();
	if (cb && udata) {
		subProgress->main = false;
		subProgress->progress = 0;
		subProgress->value = 0;
		subProgress->total = MAX_MACHINE_ZONE;
		cb(udata, subProgress);
		JLOG(L"SUBPROGRESS reset 0 OK\n");
	}

	CString sql;
	sql.Format(L"select * from table_zone where ademco_id=%d order by zone_value",
				 machine->get_ademco_id());
	
	Statement query(*db_, utf8::w2a(LPCTSTR(sql)));
	int i = 0;
	while (query.executeStep()) {
		int id, ademco_id, sub_machine_id, zone_value, type,
			status_or_property, addr, detector_id;
		int ndx = 0;

		id = static_cast<int>(query.getColumn(ndx++));
		ademco_id = query.getColumn(ndx++);
		sub_machine_id = query.getColumn(ndx++);
		zone_value = query.getColumn(ndx++);
		type = query.getColumn(ndx++);
		
		std::string alias = query.getColumn(ndx++).getText();
		auto w = utf8::a2w(alias);
		if (w.empty()) {
			auto s = TR(IDS_STRING_ZONE);
			s.AppendFormat(L"%03d", zone_value);
			w = (LPCTSTR)s;
		}

		status_or_property = query.getColumn(ndx++);
		addr = query.getColumn(ndx++);
		detector_id = query.getColumn(ndx++);
			
		zone_info_ptr zone = std::make_shared<zone_info>();
		zone->set_id(id);
		zone->set_ademco_id(ademco_id);
		zone->set_zone_value(zone_value);
		zone->set_type(type);
		zone->set_alias(w.c_str());
		zone->set_detector_id(detector_id);
		zone->set_sub_machine_id(sub_machine_id);
		zone->set_status_or_property(status_or_property);
		zone->set_physical_addr(addr);
		detector_info_ptr detInfo = LoadDetectorInfoFromDB(zone->get_detector_id());
		if (detInfo) {
			detInfo->set_zone_info_id(zone->get_id());
			detInfo->set_zone_value(zone_value);
			zone->SetDetectorInfo(detInfo);
			m_detectorList.push_back(detInfo);
		}

		if (zone->get_type() == ZT_SUB_MACHINE)
			LoadSubMachineInfoFromDB(zone);
		machine->AddZone(zone);

		if (cb && udata) { 
			subProgress->progress = static_cast<int>(i++ * MAX_MACHINE_ZONE / MAX_MACHINE_ZONE);
			subProgress->value = zone_value;
			subProgress->total = MAX_MACHINE_ZONE;
			cb(udata, subProgress);
		}
	}
}


detector_info_ptr alarm_machine_manager::LoadDetectorInfoFromDB(int id)
{
	AUTO_LOG_FUNCTION;
	detector_info_ptr detector;
	CString sql;
	sql.Format(L"select * from table_detector where id=%d", id);
	Statement query(*db_, utf8::w2a((LPCTSTR)sql));
	if (query.executeStep()) {
		int map_id, zone_info_id, x, y, distance, angle, detector_lib_id;

		int ndx = 1; // skip column id
		map_id = query.getColumn(ndx++); 
		//zone_info_id = query.getColumn(ndx++);
		ndx++; // skip zone info id, but why?
		x = query.getColumn(ndx++);
		y = query.getColumn(ndx++);
		distance = query.getColumn(ndx++);
		angle = query.getColumn(ndx++);
		detector_lib_id = query.getColumn(ndx++);

		detector = std::make_shared<detector_info>();
		detector->set_id(id);
		detector->set_map_id(map_id);
		detector->set_zone_info_id(zone_info_id);
		detector->set_x(x);
		detector->set_y(y);
		detector->set_distance(distance);
		detector->set_angle(angle);
		detector->set_detector_lib_id(detector_lib_id);
	}

	return detector;
}


void alarm_machine_manager::LoadCameraInfoFromDB()
{
	AUTO_LOG_FUNCTION;
	CString sql;
	sql.Format(L"select * from table_camera order by device_info_id and device_productor");
	Statement query(*db_, utf8::w2a((LPCTSTR)sql));
	while (query.executeStep()) {
		int id, ademco_id, sub_machine_id, map_id, x, y, distance, angle, detector_lib_id, device_info_id, device_productor;
		int ndx = 0;

		id = static_cast<int>(query.getColumn(ndx++));
		ademco_id = query.getColumn(ndx++);
		sub_machine_id = query.getColumn(ndx++);
		map_id = query.getColumn(ndx++);
		x = query.getColumn(ndx++);
		y = query.getColumn(ndx++);
		distance = query.getColumn(ndx++);
		angle = query.getColumn(ndx++);
		detector_lib_id = query.getColumn(ndx++);
		device_info_id = query.getColumn(ndx++);
		device_productor = query.getColumn(ndx++);

		detector_info_ptr detector = std::make_shared<detector_info>();
		detector->set_id(id);
		detector->set_map_id(map_id);
		detector->set_x(x);
		detector->set_y(y);
		detector->set_distance(distance);
		detector->set_angle(angle);
		detector->set_detector_lib_id(detector_lib_id);

		camera_info_ptr cameraInfo = std::make_shared<camera_info>();
		cameraInfo->set_ademco_id(ademco_id);
		cameraInfo->set_sub_machine_id(sub_machine_id);
		cameraInfo->SetDetectorInfo(detector);
		cameraInfo->set_device_info_id(device_info_id);
		cameraInfo->set_productor_type(device_productor);

		m_cameraMap[std::pair<int, int>(device_info_id, device_productor)].push_back(cameraInfo);
		m_cameraIdMap[id] = cameraInfo;
	}
}


void alarm_machine_manager::ResolveCameraInfo(int device_id, int productor)
{
	std::list<std::pair<int, int>> unresolvedList;
	auto iter = m_cameraMap.find(std::pair<int, int>(device_id, productor));
	if (iter != m_cameraMap.end()) {
		for (auto camera : iter->second) {
			bool resolved = false;
			do {
				alarm_machine_ptr machine = m_machineMap[camera->get_ademco_id()];
				if (!machine) break;
				if (camera->get_sub_machine_id() != -1) {
					auto zone = machine->GetZone(camera->get_sub_machine_id());
					if (!zone) break;
					machine = zone->GetSubMachineInfo();
					if (!machine) break;
				} 
				int map_id = camera->GetDetectorInfo()->get_map_id();
				auto map = machine->GetMapInfo(map_id);
				if (!map) break;
				map->AddInterface(camera);
				resolved = true;
			} while (false);
			if(!resolved)
				unresolvedList.push_back(std::make_pair(device_id, productor));
		}
	} else {
		unresolvedList.push_back(std::make_pair(device_id, productor));
	}

	CString query;
	for (auto pair : unresolvedList) {
		auto cameraList = m_cameraMap[pair];
		for (auto camera : cameraList) {
			m_cameraIdMap.erase(camera->GetDetectorInfo()->get_id());
		}
		m_cameraMap.erase(pair);
		query.Format(L"delete from table_camera where device_info_id=%d and device_productor=%d", pair.first, pair.second);
		ExecuteSql(query);
	}
}


void alarm_machine_manager::DeleteCameraInfo(const camera_info_ptr& camera)
{
	AUTO_LOG_FUNCTION;
	assert(camera);
	auto pair = std::make_pair(camera->get_device_info_id(), camera->get_productor_type());
	CString query;
	query.Format(L"delete from table_camera where id=%d", camera->GetDetectorInfo()->get_id());
	ExecuteSql(query);
	m_cameraMap[pair].remove(camera);
	m_cameraIdMap.erase(camera->GetDetectorInfo()->get_id());
}


void alarm_machine_manager::AddMapInfo(const core::map_info_ptr& mapInfo)
{ 
	m_mapInfoMap[mapInfo->get_id()] = mapInfo;
}


void alarm_machine_manager::DeleteMapInfo(const core::map_info_ptr& mapInfo)
{ 
	m_mapInfoMap.erase(mapInfo->get_id());
	if (m_mapInfoMap.empty()) {
		ExecuteSql(L"update sqlite_sequence set seq=0 where name='table_map'");
	}
}


void alarm_machine_manager::DeleteCameraInfo(int device_id, int productor)
{
	AUTO_LOG_FUNCTION;
	auto pair = std::make_pair(device_id, productor);
	CString query;
	query.Format(L"delete from table_camera where device_info_id=%d and device_productor=%d", device_id, productor);
	ExecuteSql(query);
	auto iter = m_cameraMap.find(pair);
	if (iter != m_cameraMap.end()) {
		for (auto camera : iter->second) {
			auto detInfo = camera->GetDetectorInfo();
			auto map_id = detInfo->get_map_id();
			auto mapInfo = GetMapInfoById(map_id);
			if (mapInfo) {
				mapInfo->InversionControl(ICMC_MODE_EDIT);
				mapInfo->SetActiveInterfaceInfo(camera);
				mapInfo->InversionControl(ICMC_DEL_DETECTOR);
				mapInfo->RemoveInterface(camera);
				mapInfo->InversionControl(ICMC_MODE_NORMAL);
			}
			m_cameraIdMap.erase(camera->GetDetectorInfo()->get_id());
		}
		iter->second.clear();
		m_cameraMap.erase(iter);
	}
}


void alarm_machine_manager::AddCameraInfo(const camera_info_ptr& camera)
{
	AUTO_LOG_FUNCTION;
	assert(camera);
	auto pair = std::make_pair(camera->get_device_info_id(), camera->get_productor_type());
	m_cameraMap[pair].push_back(camera);
	m_cameraIdMap[camera->GetDetectorInfo()->get_id()] = camera;
}


map_info_ptr alarm_machine_manager::GetMapInfoById(int id)
{
	auto iter = m_mapInfoMap.find(id);
	if (iter != m_mapInfoMap.end()) {
		return iter->second;
	}
	return nullptr;
}


void alarm_machine_manager::LoadSubMachineInfoFromDB(const zone_info_ptr& zone)
{
	AUTO_LOG_FUNCTION;
	CString sql;
	sql.Format(L"select * from table_sub_machine where id=%d", zone->get_sub_machine_id());
	Statement query(*db_, utf8::w2a((LPCTSTR)sql));
	if (query.executeStep()) {
		CString null;
		null = TR(IDS_STRING_NULL);

		int status;
		std::string contact, address, phone, phone_bk, expire_time;
		double x, y; int zoom_level, auto_show_map_when_alarm;
		int ndx = 1; // skip id

		status = query.getColumn(ndx++);
		contact = query.getColumn(ndx++).getText();
		address = query.getColumn(ndx++).getText();
		phone = query.getColumn(ndx++).getText();
		phone_bk = query.getColumn(ndx++).getText();
		expire_time = query.getColumn(ndx++).getText();

		x = query.getColumn(ndx++).getDouble();
		y = query.getColumn(ndx++).getDouble();
		zoom_level = query.getColumn(ndx++);
		auto_show_map_when_alarm = query.getColumn(ndx++);

		alarm_machine_ptr subMachine = std::make_shared<alarm_machine>();
		subMachine->set_is_submachine(true);
		subMachine->set_id(zone->get_sub_machine_id());
		subMachine->set_machine_status(Integer2MachineStatus(status));
		subMachine->set_ademco_id(zone->get_ademco_id());
		subMachine->set_submachine_zone(zone->get_zone_value());
		subMachine->set_alias(zone->get_alias());
		subMachine->set_contact(utf8::a2w(contact).c_str());
		subMachine->set_address(utf8::a2w(address).c_str());
		subMachine->set_phone(utf8::a2w(phone).c_str());
		subMachine->set_phone_bk(utf8::a2w(phone_bk).c_str());
		subMachine->set_expire_time(string_to_time_point(expire_time));
		subMachine->set_coor(web::BaiduCoordinate(x, y));
		subMachine->set_zoomLevel(zoom_level);
		subMachine->set_auto_show_map_when_start_alarming(auto_show_map_when_alarm != 0);

		LoadSmsConfigFromDB(subMachine);

		alarm_machine_ptr parentMachine = GetMachine(zone->get_ademco_id());
		if (parentMachine) {
			subMachine->set_machine_type(parentMachine->get_machine_type());
		}

		LoadMapInfoFromDB(subMachine);
		LoadSubZoneInfoOfSubMachineFromDB(subMachine);
		zone->SetSubMachineInfo(subMachine);
	}
}


void alarm_machine_manager::LoadSubZoneInfoOfSubMachineFromDB(const core::alarm_machine_ptr& subMachine)
{
	CString sql;
	sql.Format(L"select * from table_sub_zone where sub_machine_id=%d", subMachine->get_id());
	Statement query(*db_, utf8::w2a((LPCTSTR)sql));

	while (query.executeStep()) {
		std::string alias;
		int id, sub_zone_value, detector_info_id;
		int ndx = 0;

		id = static_cast<int>(query.getColumn(ndx++));
		sub_zone_value = query.getColumn(ndx++);
		alias = query.getColumn(ndx++).getText();
		auto w = utf8::a2w(alias);
		if (w.empty()) {
			auto s = TR(IDS_STRING_ZONE);
			s.AppendFormat(L"%02d", sub_zone_value);
			w = (LPCTSTR)s;
		}
		detector_info_id = query.getColumn(ndx++);

		zone_info_ptr subZone = std::make_shared<zone_info>();
		subZone->set_id(id);
		subZone->set_ademco_id(subMachine->get_ademco_id());
		subZone->set_zone_value(subMachine->get_submachine_zone());
		subZone->set_sub_zone(sub_zone_value);
		subZone->set_sub_machine_id(subMachine->get_id());
		subZone->set_alias(w.c_str());
		subZone->set_detector_id(detector_info_id);
		//subZone->set_property_id(property_info_id);
		subZone->set_type(ZT_SUB_MACHINE_ZONE);

		detector_info_ptr detInfo = LoadDetectorInfoFromDB(subZone->get_detector_id());
		if (detInfo) {
			detInfo->set_zone_info_id(subZone->get_id());
			int zone_value = subZone->get_sub_zone();
			detInfo->set_zone_value(zone_value);
			subZone->SetDetectorInfo(detInfo);
			m_detectorList.push_back(detInfo);
		}

		subMachine->AddZone(subZone);
	}
}


void alarm_machine_manager::LoadDetectorLibFromDB()
{
	auto detectorLib = detector_lib_manager::get_instance();
	Statement query(*db_, "select * from table_detector_lib order by id");
	while (query.executeStep()) {
		int id, type, antline_num, antline_gap;
		std::string detector_name, path, path_pair;

		int ndx = 0;

		id = static_cast<int>(query.getColumn(ndx++));
		type = query.getColumn(ndx++);
		detector_name = query.getColumn(ndx++).getText();
		path = query.getColumn(ndx++).getText();
		path_pair = query.getColumn(ndx++).getText();
		antline_num = query.getColumn(ndx++);
		antline_gap = query.getColumn(ndx++);

		detector_lib_data_ptr data = std::make_shared<detector_lib_data>();
		data->set_id(id);
		data->set_type(type);
		data->set_detector_name(utf8::a2w(detector_name).c_str());
		data->set_path(utf8::a2w(path).c_str());
		data->set_path_pair(utf8::a2w(path_pair).c_str());
		data->set_antline_num(antline_num);
		data->set_antline_gap(antline_gap);
		detectorLib->AddDetectorLibData(data);
	}
}


int alarm_machine_manager::GetMachineCount() const
{
	return m_machineMap.size();
}


alarm_machine_ptr alarm_machine_manager::GetMachine(int ademco_id)
{
	auto iter = m_machineMap.find(ademco_id);
	if (iter != m_machineMap.end() && iter->second) {
		return iter->second;
	} else {
		iter = invalid_machine_map_.find(ademco_id);
		if (iter != invalid_machine_map_.end() && iter->second) {
			return iter->second;
		}
	}
	return nullptr;
}

alarm_machine_ptr alarm_machine_manager::GetMachineByUuid(machine_uuid uuid)
{
	auto machine = GetMachine(uuid.first);
	if (uuid.second != 0) {
		auto zone = machine->GetZone(uuid.second);
		if (zone) {
			machine = zone->GetSubMachineInfo();
		}
	}
	
	return machine;
}


BOOL alarm_machine_manager::CheckIsValidMachine(int ademco_id, /*const char* device_id, */int zone)
{
	if (ademco_id < MIN_MACHINE || MAX_MACHINE <= ademco_id) {
		return FALSE;
	}

	if (zone < 0 || MAX_MACHINE_ZONE < zone) {
		return FALSE;
	}

	auto iter = m_machineMap.find(ademco_id);
	if (iter != m_machineMap.end() && iter->second) {
		//if (iter->second) {
			//if (!iter->second->get_banned()) {
				return TRUE;
			//}
		//}
	} else {
		iter = invalid_machine_map_.find(ademco_id);
		if (iter == invalid_machine_map_.end()) {
			// create a dummy machine
			if (invalid_machine_group_ == nullptr) {
				invalid_machine_group_ = std::make_shared<group_info>();
			}

			auto machine = std::make_shared<core::alarm_machine>();
			machine->set_ademco_id(ademco_id);
			machine->set_group_id(invalid_machine_group_->get_id());
			machine->set_alias(TR(IDS_STRING_MACHINE_CANNOT_BE_IDENTIFIED));
			machine->set_consumer(std::make_shared<core::consumer>(0, ademco_id, 0,
																   core::consumer_manager::get_instance()->get_all_types()[0],
																   0, 0, std::chrono::system_clock::now()));

			std::lock_guard<std::mutex> lg(lock_for_invlaid_machines_);
			invalid_machine_map_[ademco_id] = machine;
			return TRUE;
		} else {
			return TRUE;
		}

	}
	return FALSE;
}


BOOL alarm_machine_manager::CheckIfMachineAdemcoIdCanUse(int ademco_id)
{
	if (ademco_id < MIN_MACHINE || MAX_MACHINE <= ademco_id) {
		return FALSE;
	}

	auto iter = m_machineMap.find(ademco_id);
	if (iter != m_machineMap.end() && iter->second) {
		return FALSE;

	}

	return TRUE;
}


BOOL alarm_machine_manager::DistributeAdemcoID(int& ademco_id)
{
	if (m_machineMap.size() >= MAX_MACHINE) {
		return FALSE;
	}
	
	for (int i = MIN_MACHINE; i < MAX_MACHINE; i++) {
		auto machine = m_machineMap[i];
		if (!machine) {
			ademco_id = i;
			return TRUE;
		}
	}

	return FALSE;
}


std::list<int> alarm_machine_manager::fuzzy_search_machine(const std::wstring & keyword, int limit)
{
	auto formatted_key = utf8::w2a(double_quotes(keyword));
	auto like = " '%" + formatted_key + "%' ";
	std::stringstream ss;
	ss << "select ademco_id from table_machine where "
		<< "ademco_id like" << like
		<< "or machine_name like" << like
		<< "or contact like" << like
		<< "or address like" << like
		<< "or phone like" << like
		<< "or phone_bk like" << like
		<< "order by ademco_id "
		<< "limit " << limit;

	auto sql = ss.str();
	//JLOGA(sql.c_str());
	Statement query(*db_, sql);
	std::list<int> ret;

	while (query.executeStep()) {
		int aid = query.getColumn(0);
		ret.push_back(aid);
	}

	return ret;
}

bool alarm_machine_manager::CreateSmsConfigForMachine(const core::alarm_machine_ptr& machine)
{
	sms_config cfg = {};
	CString sql;
	sql.Format(L"insert into table_sms_config \
([is_submachine],[ademco_id],[zone_value],[report_alarm],[report_exception],[report_status],[report_alarm_bk],[report_exception_bk],[report_status_bk]) \
values(%d,%d,%d,%d,%d,%d,%d,%d,%d)",
			   machine->get_is_submachine(), machine->get_ademco_id(), 
			   machine->get_is_submachine() ? machine->get_submachine_zone() : -1,
			   cfg.report_alarm, cfg.report_exception, cfg.report_status,
			   cfg.report_alarm_bk, cfg.report_exception_bk, cfg.report_status_bk);

	db_->exec(utf8::w2a((LPCTSTR)sql));
	cfg.id = static_cast<int>(db_->getLastInsertRowid());
	machine->set_sms_cfg(cfg);
	return true;
}


BOOL alarm_machine_manager::AddMachine(const core::alarm_machine_ptr& machine)
{
	int ademco_id = machine->get_ademco_id();
	if (ademco_id < MIN_MACHINE || MAX_MACHINE <= ademco_id) {
		return FALSE;
	}

	std::lock_guard<std::mutex> lock(m_lock4Machines);
	CString sql;
	sql.Format(L"insert into [table_machine] \
([ademco_id],[group_id],[banned],[machine_status],[machine_type],\
[machine_name],[contact],[address],[phone],[phone_bk],\
[expire_time],\
[map_coor_x],[map_coor_y],[map_zoom_level],[auto_show_map_when_alarm]) \
values(%d,%d,%d,%d,%d,\
'%s','%s','%s','%s','%s',\
'%s',\
%f,%f,%d,%d)",
			   ademco_id, machine->get_group_id(), machine->get_banned(), machine->get_machine_status(), machine->get_machine_type(),
			   machine->get_machine_name(), machine->get_contact(), machine->get_address(), machine->get_phone(), machine->get_phone_bk(),
			   time_point_to_wstring(machine->get_expire_time()).c_str(),
			   machine->get_coor().x, machine->get_coor().y, machine->get_zoomLevel(), machine->get_auto_show_map_when_start_alarming());

	int id = AddAutoIndexTableReturnID(sql);
	if (-1 == id) {
		return FALSE;
	}

	machine->set_id(id);

	// add sms config
	{
		CreateSmsConfigForMachine(machine);
	}

	m_machineMap[ademco_id] = machine;

	{
		std::lock_guard<std::mutex> lg(lock_for_invlaid_machines_);
		auto iter = invalid_machine_map_.find(ademco_id);
		if (iter != invalid_machine_map_.end()) {
			machine->set_online(iter->second->get_online());
			machine->execute_set_machine_type(iter->second->get_machine_type());
			machine->set_machine_status(iter->second->get_machine_status());
			
			invalid_machine_map_.erase(ademco_id);
		}
	}
	return TRUE;
}


BOOL alarm_machine_manager::DeleteMachine(const core::alarm_machine_ptr& machine)
{
	int ademco_id = machine->get_ademco_id();
	if (ademco_id < MIN_MACHINE || MAX_MACHINE <= ademco_id) {
		return FALSE;
	}

	std::lock_guard<std::mutex> lock(m_lock4Machines);

	machine->kill_connction();
	CString sql;
	sql.Format(L"delete from table_machine where id=%d and ademco_id=%d",
			   machine->get_id(), machine->get_ademco_id());
	if (ExecuteSql(sql)) {
		// delete consumer info
		consumer_manager::get_instance()->execute_delete_consumer(machine->get_consumer());

		// delete all camera info
		map_info_list mapList;
		machine->GetAllMapInfo(mapList);
		for (auto map : mapList) {
			std::list<detector_bind_interface_ptr> interfaceList;
			map->GetAllInterfaceInfo(interfaceList);
			for (auto pInterface : interfaceList) {
				if (pInterface->GetInterfaceType() == DIT_CAMERA_INFO) {
					camera_info_ptr cam = std::dynamic_pointer_cast<camera_info>(pInterface);
					DeleteCameraInfo(cam);
				}
			}
		}

		// delete all zone & detector info of machine
		std::list<zone_info_ptr> zoneList;
		machine->GetAllZoneInfo(zoneList);
		for (auto zone : zoneList) {
			int detector_id = zone->get_detector_id();
			if (-1 != detector_id) {
				sql.Format(L"delete from table_detector where id=%d", detector_id);
				(ExecuteSql(sql));
			}
			alarm_machine_ptr subMachine = zone->GetSubMachineInfo();
			if (subMachine) {
				DeleteSubMachine(zone);
			}
			DeleteVideoBindInfoByZoneInfo(zone);
		}

		sql.Format(L"delete from table_zone where ademco_id=%d", machine->get_ademco_id());
		(ExecuteSql(sql));

		sql.Format(L"delete from table_map where machine_id=%d and type=%d",
					 machine->get_ademco_id(), MAP_MACHINE);
		(ExecuteSql(sql));
		
		group_info_ptr group = group_manager::get_instance()->GetGroupInfo(machine->get_group_id());
		group->RemoveChildMachine(machine); 

		// delete sms config
		//sql.Format(L"delete from table_sms_config where id=%d", machine->get_sms_cfg().id);
		sql.Format(L"delete from table_sms_config where id=%d or (is_submachine=%d and ademco_id=%d and zone_value=%d)",
				   machine->get_sms_cfg().id, machine->get_is_submachine(), machine->get_ademco_id(),
				   machine->get_is_submachine() ? machine->get_submachine_zone() : -1);
		(ExecuteSql(sql));
		
		m_machineMap.erase(ademco_id);

		// 2016-11-11 14:01:03 add history 
		CString rec;
		rec.Format(L"%s %s", TR(IDS_STRING_DELED_MACHINE), machine->get_formatted_name());
		history_record_manager::get_instance()->InsertRecord(ademco_id, machine->get_is_submachine() ? machine->get_submachine_zone() : 0,
															 rec, time(nullptr), RECORD_LEVEL_USEREDIT);


		return TRUE;
	}
	return FALSE;
}


BOOL alarm_machine_manager::DeleteSubMachine(const zone_info_ptr& zoneInfo)
{
	ASSERT(zoneInfo);
	alarm_machine_ptr subMachine = zoneInfo->GetSubMachineInfo();
	ASSERT(subMachine);
	CString sql;

	// delete sms config
	sql.Format(L"delete from table_sms_config where id=%d", subMachine->get_sms_cfg().id);
	(ExecuteSql(sql));

	consumer_manager::get_instance()->execute_delete_consumer(subMachine->get_consumer());
	
	sql.Format(L"delete from table_sub_machine where id=%d", subMachine->get_id());
	JLOG(L"%s\n", sql);
	(ExecuteSql(sql));

	// delete all camera info
	map_info_list mapList;
	subMachine->GetAllMapInfo(mapList);
	for (auto map : mapList) {
		std::list<detector_bind_interface_ptr> interfaceList;
		map->GetAllInterfaceInfo(interfaceList);
		for (auto pInterface : interfaceList) {
			if (pInterface->GetInterfaceType() == DIT_CAMERA_INFO) {
				camera_info_ptr cam = std::dynamic_pointer_cast<camera_info>(pInterface);
				DeleteCameraInfo(cam);
			}
		}
	}

	// delete all zone & detector info of machine
	std::list<zone_info_ptr> zoneList;
	subMachine->GetAllZoneInfo(zoneList);
	for (auto zone : zoneList) {
		int detector_id = zone->get_detector_id();
		if (-1 != detector_id) {
			sql.Format(L"delete from table_detector where id=%d", detector_id);
			JLOG(L"%s\n", sql);
			(ExecuteSql(sql));
		}
		DeleteVideoBindInfoByZoneInfo(zone);
	}

	sql.Format(L"delete from table_sub_zone where sub_machine_id=%d",
				 subMachine->get_id());
	JLOG(L"%s\n", sql);
	ExecuteSql(sql);

	sql.Format(L"delete from table_map where machine_id=%d and type=%d",
				 subMachine->get_id(), MAP_SUB_MACHINE);
	JLOG(L"%s\n", sql);
	ExecuteSql(sql);

	sql.Format(L"update table_zone set type=%d,sub_machine_id=-1 where id=%d",
				 ZT_ZONE, zoneInfo->get_id());
	JLOG(L"%s\n", sql);
	(ExecuteSql(sql));

	return TRUE;
}


void alarm_machine_manager::MachineEventHandler(EventSource source, 
											   int ademco_id, int ademco_event,
											   int zone, int subzone, 
											   const time_t& timestamp,
											   const time_t& recv_time,
											   const ademco::char_array_ptr& xdata
											   )
{
	//AUTO_LOG_FUNCTION;
	auto mode = util::CConfigHelper::get_instance()->get_congwin_fe100_router_mode();
	if (mode) {
		return;
	}

	alarm_machine_ptr machine = GetMachine(ademco_id);
	if (machine) {
		machine->SetAdemcoEvent(source, ademco_event, zone, subzone, timestamp, recv_time, xdata);
	}
}


void alarm_machine_manager::MachineOnline(ademco::EventSource source, 
										 int ademco_id, BOOL online, const char* ipv4,
										 net::server::CClientDataPtr udata, remote_control_command_conn_call_back cb)
{
	AUTO_LOG_FUNCTION;

	auto mode = util::CConfigHelper::get_instance()->get_congwin_fe100_router_mode();
	if (mode) {
		return;
	}


	alarm_machine_ptr machine = GetMachine(ademco_id);
	if (machine) {
		time_t event_time = time(nullptr);
		machine->SetAdemcoEvent(source, online ? EVENT_ONLINE : EVENT_OFFLINE, 0, 0, event_time, event_time);
		if (online && udata && cb) {
			machine->SetConnHangupCallback(udata, cb);
		}
		machine->set_ipv4(ipv4);

		// 2016年8月30日17:48:56 主动索要主机类型
		if (online) {
			net::CNetworkConnector::get_instance()->Send(machine->get_ademco_id(), EVENT_WHAT_IS_YOUR_TYPE, 0, 0);
		}
	}
}


BOOL alarm_machine_manager::RemoteControlAlarmMachine(const alarm_machine_ptr& machine, 
													 int ademco_event, int gg, int zone, 
													 const ademco::char_array_ptr& xdata,
													 const ademco::char_array_ptr& cmd,
													 ademco::EventSource path,
													 CWnd* pWnd)
{
	assert(machine);
	//char xdata[64] = { 0 };
	//int xdata_len = 0;
	if (ademco_event == ademco::EVENT_DISARM) {
		m_pPrevCallDisarmWnd = pWnd;
		m_prevCallDisarmAdemcoID = machine->get_ademco_id();
		m_prevCallDisarmGG = gg;
		m_prevCallDisarmZoneValue = zone;
	}

	CString srecord, suser, sfm, sop;
	suser = TR(IDS_STRING_USER);
	sfm = TR(IDS_STRING_LOCAL_OP);
	switch (ademco_event) {
		case EVENT_ARM:
			sop = TR(IDS_STRING_ARM);
			break;
		case EVENT_HALFARM:
			sop = TR(IDS_STRING_HALFARM);
			break;
		case EVENT_DISARM:
			sop = TR(IDS_STRING_DISARM);
			break;
		case EVENT_EMERGENCY:
			sop = TR(IDS_STRING_EMERGENCY);
			break;
		case EVENT_QUERY_SUB_MACHINE:
			sop = TR(IDS_STRING_QUERY);
			break;
		case EVENT_RETRIEVE_ZONE_OR_SUB_MACHINE:
			sop = TR(IDS_STRING_RETRIEVE);
			break;
		case EVENT_WRITE_TO_MACHINE: {
			sop = TR(IDS_STRING_IDD_DIALOG_RESTORE_MACHINE);
			CString s, szone; szone = TR(IDS_STRING_ZONE);
			s.Format(L"(%s%03d)", szone, zone);
			sop += s;
		}
			break;
		default:
			assert(0);
			break;
	}
	user_info_ptr user = user_manager::get_instance()->get_cur_user_info();
	srecord.Format(L"%s(ID:%d,%s)%s:%s", suser,
				   user->get_id(), user->get_name().c_str(),
				   sfm, sop);

	srecord += L" " + machine->get_formatted_name();
	history_record_manager::get_instance()->InsertRecord(machine->get_ademco_id(),
														zone, srecord, time(nullptr),
														RECORD_LEVEL_USERCONTROL);

	BOOL ok = net::CNetworkConnector::get_instance()->Send(machine->get_ademco_id(), ademco_event, gg, zone, xdata, cmd, path);
	if (!ok) {
		srecord = TR(IDS_STRING_OP_FAILED_BY_OFFLINE);
		history_record_manager::get_instance()->InsertRecord(machine->get_ademco_id(),
															zone, srecord, time(nullptr),
															RECORD_LEVEL_USERCONTROL);
	} 


	
	return ok;
}


void alarm_machine_manager::DisarmPasswdWrong(int ademco_id)
{
	PostMessageToMainWnd(WM_DISARM_PASSWD_WRONG, ademco_id);
}


BOOL alarm_machine_manager::EnterBufferMode()
{
	AUTO_LOG_FUNCTION;
	for (auto iter : m_machineMap) {
		if(iter.second)
			iter.second->EnterBufferMode();
	}
	return TRUE;
}


BOOL alarm_machine_manager::LeaveBufferMode()
{
	AUTO_LOG_FUNCTION;
	for (auto iter : m_machineMap) {
		if (iter.second)
			iter.second->LeaveBufferMode();
	}
	return TRUE;
}


void __stdcall alarm_machine_manager::OnOtherCallEnterBufferMode(void* udata)
{
	AUTO_LOG_FUNCTION;
	auto mgr = reinterpret_cast<alarm_machine_manager*>(udata);
	SetEvent(mgr->m_hEventOotebm);
}

//m_hEventOotebm
void alarm_machine_manager::ThreadCheckSubMachine()
{
	AUTO_LOG_FUNCTION;
	
	while (running_) {
		

		{
			std::unique_lock<std::mutex> lock(mutex_);
			condvar_.wait_for(lock, std::chrono::milliseconds(CHECK_GAP), [this]() {return !running_; });
		}

		if (!running_) {
			break;
		}

		{
			auto now = time(nullptr);
			std::lock_guard<std::mutex> lg(lock_for_invlaid_machines_);
			for (auto iter = invalid_machine_map_.begin(); iter != invalid_machine_map_.end(); ) {
				auto last = iter->second->GetLastActionTime();
				if (now - last > 60 * 60 * 1) { // delete invalid machine for 1 hour no data
					iter = invalid_machine_map_.erase(iter);
				} else {
					iter++;
				}
			}
		}


		if (GetMachineCount() == 0) {
			std::this_thread::sleep_for(std::chrono::milliseconds(1000));
			continue;
		}

		std::lock_guard<std::mutex> lock(m_lock4Machines);
		alarm_machine_list *subMachineList = nullptr;
		for (int i = MIN_MACHINE; i < MAX_MACHINE; i++) {
			if (!running_)
				break;

			alarm_machine_ptr machine = m_machineMap[i];
			if (machine && machine->get_online() && machine->get_submachine_count() > 0) {
				if (!machine->EnterBufferMode()) {
					machine->SetOotebmObj(nullptr, nullptr);
					continue;
				}

				machine->SetOotebmObj(OnOtherCallEnterBufferMode, this);
				zone_info_list list;
				machine->GetAllZoneInfo(list);
				bool bAlreadyLeaveBuffMode = false;

				for (auto zoneInfo : list) {
					if (!running_)
						break;

					if (WAIT_OBJECT_0 == WaitForSingleObject(m_hEventOotebm, 0)) {
						machine->SetOotebmObj(nullptr, nullptr); 
						machine->LeaveBufferMode();
						bAlreadyLeaveBuffMode = true;
						break;
					}

					alarm_machine_ptr subMachine = zoneInfo->GetSubMachineInfo();
					if (subMachine) {
						time_t lastActionTime = subMachine->GetLastActionTime();
						time_t check_time = time(nullptr);
						if ((check_time - lastActionTime) * 1000 >= MAX_SUBMACHINE_ACTION_TIME_OUT) {
							if (subMachineList == nullptr) {
								subMachineList = new alarm_machine_list();
							}
							subMachineList->push_back(subMachine);
						}
					}
				}

				if (!bAlreadyLeaveBuffMode) {
					machine->SetOotebmObj(nullptr, nullptr);
					machine->LeaveBufferMode();
				}
			}
		}

		if (subMachineList && subMachineList->size() > 0) {
			PostMessageToMainWnd(WM_NEEDQUERYSUBMACHINE, (WPARAM)subMachineList, subMachineList->size());
		}
	}
}


void core::alarm_machine_manager::DeleteVideoBindInfoByZoneInfo(const zone_info_ptr& zoneInfo)
{
	AUTO_LOG_FUNCTION;

	video::zone_uuid uuid(zoneInfo->get_ademco_id(), zoneInfo->get_zone_value(), 0);
	if (zoneInfo->get_type() == ZT_SUB_MACHINE_ZONE) {
		uuid._gg = zoneInfo->get_sub_zone();
	}
	video::video_manager::get_instance()->UnbindZoneAndDevice(uuid);
}


detector_info_ptr core::alarm_machine_manager::GetDetectorInfo(int id)
{ 
	for (auto det : m_detectorList) { 
		if (det->get_id() == id)
			return det; 
	} 
	return nullptr; 
}



};




