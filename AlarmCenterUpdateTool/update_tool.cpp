#include "stdafx.h"
#include "resource.h"
#include "update_tool.h"
#include <thread>
#include "DbOper.h"
#include "sqlitecpp\SQLiteCpp.h"
#include <cstdlib>
#include "../json/json.h"

using namespace ado;
using namespace SQLite;
using namespace Json;

namespace detail {

update_tool* g_instance = nullptr;

#define call(f) { if (g_instance) g_instance->f; }

typedef std::function<void(void)> wait_cb;

inline int execute(const std::string& cmd, wait_cb cb = nullptr)
{
	/*SECURITY_ATTRIBUTES sa = {};
	sa.nLength = sizeof(sa);
	sa.bInheritHandle = TRUE;
	sa.lpSecurityDescriptor = nullptr;


	HANDLE pipe, dummy;
	BOOL ok = CreatePipe(&pipe, &dummy, &sa, 0);
	ok = SetHandleInformation(pipe, HANDLE_FLAG_INHERIT, 0);*/

	STARTUPINFOA si = { 0 };
	si.cb = sizeof(si);
	//si.hStdError = pipe;
	//si.hStdOutput = pipe;
	si.dwFlags |= STARTF_USESHOWWINDOW/* | STARTF_USESTDHANDLES*/;
	si.wShowWindow = SW_HIDE;
	PROCESS_INFORMATION pi = { };
	//ZeroMemory(&pi, sizeof(PROCESS_INFORMATION));
	//const auto npos = std::string::size_type(-1);

	BOOL bRet = CreateProcessA(NULL, (LPSTR)(cmd.c_str()), NULL, NULL, FALSE, 0, NULL, NULL, &si, &pi);
	if (bRet) {
		while (WAIT_OBJECT_0 != WaitForSingleObject(pi.hProcess, 200)) {
			if(cb)
				cb();

			/*char buffer[1024] = { 0 };
			DWORD dwRead = 0;
			BOOL ok = FALSE;
			std::string result;

			ok = ReadFile(pipe, buffer, 1024, &dwRead, nullptr);
			if (ok && dwRead != 0) {
				result += buffer;
				auto pos = result.find_first_of("\r\n");
				while (pos != npos) {
					std::string sub = result.substr(0, pos);
					if (pos != result.size() - 1)
						result = result.substr(pos);
					else
						result.clear();

					if(!sub.empty())
						call(add_up(std::make_shared<update_progress>(2, 100, utf8::a2w(sub))));

					pos = result.find_first_of("\r\n");
				}
			}*/
		}
		DWORD ret;
		GetExitCodeThread(pi.hThread, &ret);
		CloseHandle(pi.hThread);
		CloseHandle(pi.hProcess);
		return ret;
	}
	return -1;
}

auto get_data_path() {
	return get_exe_path_a() + "\\data";
}

auto get_config_path() {
	return get_data_path() + "\\config";
}

void create_dst_db3(std::shared_ptr<SQLite::Database>& dbdst) {
	dbdst->exec("drop table if exists table_machine");
	dbdst->exec("create table table_machine (id integer primary key AUTOINCREMENT, \
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

	dbdst->exec("drop table if exists table_camera");
	dbdst->exec("create table table_camera (id integer primary key AUTOINCREMENT, \
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

	dbdst->exec("drop table if exists table_detector");
	dbdst->exec("create table table_detector (id integer primary key AUTOINCREMENT, \
map_id integer, \
zone_info_id integer, \
x integer, y integer, \
distance integer, \
angle integer, \
detector_lib_id integer)");

	dbdst->exec("drop table if exists table_detector_lib");
	dbdst->exec("create table table_detector_lib (id integer primary key AUTOINCREMENT, \
type integer, \
detector_name text, \
detector_pic_path text, \
pair_pic_path text, \
antline_count integer, \
antline_gap integer)");

	dbdst->exec("drop table if exists table_group");
	dbdst->exec("create table table_group (id integer primary key AUTOINCREMENT, \
group_name text, \
parent_group_id integer)");
	dbdst->exec("insert into table_group values(NULL,\"root\", 0)");

	dbdst->exec("drop table if exists table_map");
	dbdst->exec("create table table_map (id integer primary key AUTOINCREMENT, \
type integer, \
machine_id integer, \
map_name text, \
map_pic_path text)");

	dbdst->exec("drop table if exists table_sub_machine");
	dbdst->exec("create table table_sub_machine (id integer primary key AUTOINCREMENT, \
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

	dbdst->exec("drop table if exists table_sub_zone");
	dbdst->exec("create table table_sub_zone (id integer primary key AUTOINCREMENT, \
sub_machine_id integer, \
sub_zone_value integer, \
zone_name text, \
detector_info_id integer)");

	dbdst->exec("drop table if exists table_zone");
	dbdst->exec("create table table_zone (id integer primary key AUTOINCREMENT, \
ademco_id integer, \
sub_machine_id integer, \
zone_value integer, \
type integer, \
zone_name text, \
status_or_property integer, \
physical_addr integer, \
detector_info_id integer)");

	dbdst->exec("drop table if exists table_sms_config");
	dbdst->exec("create table table_sms_config (id integer primary key AUTOINCREMENT, \
is_submachine integer, \
ademco_id integer, \
zone_value integer,  \
report_alarm integer, \
report_exception integer, \
report_status integer, \
report_alarm_bk integer, \
report_exception_bk integer, \
report_status_bk integer)");
}


void migrate_csr(std::shared_ptr<CDbOper>& dbsrc) {
	CString sql; sql = L"select * from CsrInfo order by id";
	ado::CADORecordset recordset(dbsrc->GetDatabase());
	recordset.Open(dbsrc->GetDatabase()->m_pConnection, sql);
	DWORD count = recordset.GetRecordCount();
	if (count == 1) {
		int zoomLevel; double x, y;
		recordset.MoveFirst();
		recordset.GetFieldValue(L"CsrBaiduMapX", x);
		recordset.GetFieldValue(L"CsrBaiduMapY", y);
		recordset.GetFieldValue(L"ZoomLevel", zoomLevel);
		recordset.Close();

		Value value;
		value["map"]["x"] = x;
		value["map"]["y"] = y;
		value["map"]["level"] = zoomLevel;

		std::ofstream out(get_config_path() + "\\center.json");
		if (out) {
			StyledWriter writer;
			out << writer.write(value);
			out.close();

		}
	}
}

void migrate_group(std::shared_ptr<CDbOper>& dbsrc, std::shared_ptr<SQLite::Database>& dbdst) {
	// load from old db
	CString sql; sql = L"select * from GroupInfo order by id";
	ado::CADORecordset recordset(dbsrc->GetDatabase());
	recordset.Open(dbsrc->GetDatabase()->m_pConnection, sql);
	DWORD count = recordset.GetRecordCount();
	if (count > 0) {
		recordset.MoveFirst();
		for (DWORD i = 0; i < count; i++) {
			long id, parent_id;
			CString name;
			recordset.GetFieldValue(L"id", id);
			recordset.GetFieldValue(L"parent_id", parent_id);
			recordset.GetFieldValue(L"group_name", name);
			recordset.MoveNext();

			try {
				sql.Format(L"insert into table_group values(%d,'%s',%d)", id, name, parent_id);
				dbdst->exec(utf8::w2a((LPCTSTR)sql));

			} catch (std::exception& e) {
				JLOGA(e.what());
			}
		}

	}
	recordset.Close();
}

void migrate_baidu(std::shared_ptr<SQLite::Database>& dbdst, int ademco_id, int is_sub_machine, int zone_value, int& auto_show_map_while_alarm, int& zoom_level) {
	CString dir = L"", path = L"";
	dir.Format(L"%s\\data\\config", GetModuleFilePath());
	CreateDirectory(dir, nullptr);
	dir += L"\\AlarmMachine";
	CreateDirectory(dir, nullptr);
	if (is_sub_machine) {
		path.Format(L"%s\\%04d-%03d.json", dir, ademco_id, zone_value);
	} else {
		path.Format(L"%s\\%04d.json", dir, ademco_id);
	}

	std::ifstream in((LPCTSTR)path);
	if (in) {
		Json::Value value; Json::Reader reader;
		if (reader.parse(in, value)) {
			auto_show_map_while_alarm = value["sectionBaiduMap"]["auto_show_map_while_alarming"].asBool();
			zoom_level = value["sectionBaiduMap"]["zoom_level"].asUInt();
			if (19 < zoom_level || zoom_level < 1) zoom_level = 14;
		}
		in.close();
	}
}

void migrate_sms(std::shared_ptr<SQLite::Database>& dbdst) {
	std::shared_ptr<CDbOper>& dbsrc = std::make_shared<CDbOper>();
	dbsrc->Open(L"sms.mdb");

	int sms_id, ademco_id, is_sub_machine, zone_value, alarm, except, status, alarm_bk, except_bk, status_bk;
	CString sql;
	sql.Format(L"select * from sms_config order by id");
	ado::CADORecordset recordset_sms(dbsrc->GetDatabase()); 
	recordset_sms.Open(sql);
	auto count = recordset_sms.GetRecordCount();
	if (count > 0) {
		recordset_sms.MoveFirst();
		int ndx = 15;
		double step = 10.0 / count;
		for (DWORD i = 0; i < count; i++) {
			recordset_sms.GetFieldValue(L"ID", sms_id);
			recordset_sms.GetFieldValue(L"ademco_id", ademco_id);
			recordset_sms.GetFieldValue(L"is_submachine", is_sub_machine);
			recordset_sms.GetFieldValue(L"zone_value", zone_value);
			recordset_sms.GetFieldValue(L"report_alarm", alarm);
			recordset_sms.GetFieldValue(L"report_exception", except);
			recordset_sms.GetFieldValue(L"report_status", status);
			recordset_sms.GetFieldValue(L"report_alarm_bk", alarm_bk);
			recordset_sms.GetFieldValue(L"report_exception_bk", except_bk);
			recordset_sms.GetFieldValue(L"report_status_bk", status_bk);
			recordset_sms.MoveNext();

			sql.Format(L"working on sms %06d", ademco_id);
			call(add_up(std::make_shared<update_progress>(ndx + int(step * i), 100, (LPCTSTR)sql)));

			sql.Format(L"insert into table_sms_config \
values(%d,%d,%d,%d,%d,%d,%d,%d,%d,%d)",
sms_id, is_sub_machine, ademco_id, zone_value,
alarm, except, status, alarm_bk, except_bk, status_bk);

			dbdst->exec(utf8::w2a((LPCTSTR)sql));
		}
	}
	recordset_sms.Close();
}

void migrate_map(std::shared_ptr<CDbOper>& dbsrc, std::shared_ptr<SQLite::Database>& dbdst) {
	CString sql;
	sql.Format(L"select * from MapInfo order by id");
	ado::CADORecordset recordset(dbsrc->GetDatabase());
	recordset.Open(dbsrc->GetDatabase()->m_pConnection, sql);
	DWORD count = recordset.GetRecordCount();
	if (count > 0) {
		recordset.MoveFirst();
		for (DWORD i = 0; i < count; i++) {
			long id, type, machine_id;
			CString alias, path;
			recordset.GetFieldValue(L"id", id);
			recordset.GetFieldValue(L"type", type);
			recordset.GetFieldValue(L"machine_id", machine_id);
			recordset.GetFieldValue(L"alias", alias);
			if (alias.IsEmpty()) { alias = L"no name map"; }
			recordset.GetFieldValue(L"path", path);
			recordset.MoveNext();

			sql.Format(L"insert into table_map values(%d,%d,%d,'%s','%s')",
					   id, type, machine_id, alias, path);

			dbdst->exec(utf8::w2a((LPCTSTR)sql));
		}
	}
	recordset.Close();
}


void migrate_machine(std::shared_ptr<CDbOper>& dbsrc, std::shared_ptr<SQLite::Database>& dbdst) {	
	CString sql; sql = L"select * from AlarmMachine order by id";
	ado::CADORecordset recordset(dbsrc->GetDatabase());
	JLOG(L"CADORecordset recordset %p\n", &recordset);
	BOOL ret = recordset.Open(dbsrc->GetDatabase()->m_pConnection, sql);
	VERIFY(ret); JLOG(L"recordset.Open() return %d\n", ret);
	DWORD count = recordset.GetRecordCount();

	JLOG(L"recordset.GetRecordCount() return %d\n", count);
	if (count > 0) {
		recordset.MoveFirst();
		int ndx = 26;
		double step = 24.0 / count;
		for (DWORD i = 0; i < count; i++) {
			long id, ademco_id, group_id;
			int banned, type, has_video, status;
			CString alias, contact, address, phone, phone_bk;
			COleDateTime expire_time;
			double x, y;
			int auto_show_map_while_alarm = 1, zoom_level = 14;

			recordset.GetFieldValue(L"id", id);
			recordset.GetFieldValue(L"ademco_id", ademco_id);
			recordset.GetFieldValue(L"machine_type", type);
			recordset.GetFieldValue(L"banned", banned);
			recordset.GetFieldValue(L"banned", has_video);
			recordset.GetFieldValue(L"machine_status", status);
			recordset.GetFieldValue(L"alias", alias);
			recordset.GetFieldValue(L"contact", contact);
			recordset.GetFieldValue(L"address", address);
			recordset.GetFieldValue(L"phone", phone);
			recordset.GetFieldValue(L"phone_bk", phone_bk);
			recordset.GetFieldValue(L"expire_time", expire_time);
			recordset.GetFieldValue(L"group_id", group_id);
			recordset.GetFieldValue(L"baidu_x", x);
			recordset.GetFieldValue(L"baidu_y", y);
			recordset.MoveNext();

			sql.Format(L"working on machine %06d", ademco_id);
			call(add_up(std::make_shared<update_progress>(ndx + int(step * i), 100, (LPCTSTR)sql)));

			// get baidu map info from json
			migrate_baidu(dbdst, ademco_id, 0, -1, auto_show_map_while_alarm, zoom_level);

			if (expire_time.GetStatus() == COleDateTime::invalid) {
				expire_time = COleDateTime::GetCurrentTime();
			}

			sql.Format(L"insert into [table_machine] \
values(%d,%d,%d,%d,%d,%d,\
'%s','%s','%s','%s','%s',\
'%s',\
%f,%f,%d,%d)",
id, ademco_id, group_id, banned, status, type,
alias, contact, address, phone, phone_bk,
expire_time.Format(L"%Y-%m-%d %H:%M:%S"),
x, y, zoom_level, auto_show_map_while_alarm);

			dbdst->exec(utf8::w2a((LPCTSTR)sql));
		}
	}
	recordset.Close();
}

void migrate_detector(std::shared_ptr<CDbOper>& dbsrc, std::shared_ptr<SQLite::Database>& dbdst) {	
	CString sql; sql = L"select * from DetectorInfo order by id";
	ado::CADORecordset recordset(dbsrc->GetDatabase());
	JLOG(L"CADORecordset recordset %p\n", &recordset);
	BOOL ret = recordset.Open(dbsrc->GetDatabase()->m_pConnection, sql);
	VERIFY(ret); JLOG(L"recordset.Open() return %d\n", ret);
	DWORD count = recordset.GetRecordCount();

	JLOG(L"recordset.GetRecordCount() return %d\n", count);
	if (count > 0) {
		recordset.MoveFirst();
		int ndx = 16;
		double step = 76.0 / count;
		for (DWORD i = 0; i < count; i++) {
			long id, map_id, zone_info_id, x, y, distance, angle, detector_lib_id;
			recordset.GetFieldValue(L"id", id);
			recordset.GetFieldValue(L"map_id", map_id);
			recordset.GetFieldValue(L"zone_info_id", zone_info_id);
			recordset.GetFieldValue(L"x", x);
			recordset.GetFieldValue(L"y", y);
			recordset.GetFieldValue(L"distance", distance);
			recordset.GetFieldValue(L"angle", angle);
			recordset.GetFieldValue(L"detector_lib_id", detector_lib_id);

			sql.Format(L"insert into table_detector values(%d,%d,%d,%d,%d,%d,%d,%d)",
					   id, map_id, zone_info_id, x, y, distance, angle, detector_lib_id);
			dbdst->exec(utf8::w2a((LPCTSTR)sql));
		}
	}
	recordset.Close();
}

void migrate_camera(std::shared_ptr<CDbOper>& dbsrc, std::shared_ptr<SQLite::Database>& dbdst) {	
	CString sql; sql = L"select * from DetectorInfoOfCamera order by id";
	ado::CADORecordset recordset(dbsrc->GetDatabase());
	JLOG(L"CADORecordset recordset %p\n", &recordset);
	BOOL ret = recordset.Open(dbsrc->GetDatabase()->m_pConnection, sql);
	VERIFY(ret); JLOG(L"recordset.Open() return %d\n", ret);
	DWORD count = recordset.GetRecordCount();

	JLOG(L"recordset.GetRecordCount() return %d\n", count);
	if (count > 0) {
		recordset.MoveFirst();
		int ndx = 16;
		double step = 76.0 / count;
		for (DWORD i = 0; i < count; i++) {
			long id, ademco_id, sub_machine_id, map_id, x, y, distance, angle, detector_lib_id, device_info_id, device_productor;
			recordset.GetFieldValue(L"id", id);
			recordset.GetFieldValue(L"ademco_id", ademco_id);
			recordset.GetFieldValue(L"sub_machine_id", sub_machine_id);
			recordset.GetFieldValue(L"map_id", map_id);
			recordset.GetFieldValue(L"x", x);
			recordset.GetFieldValue(L"y", y);
			recordset.GetFieldValue(L"distance", distance);
			recordset.GetFieldValue(L"angle", angle);
			recordset.GetFieldValue(L"detector_lib_id", detector_lib_id);
			recordset.GetFieldValue(L"device_info_id", device_info_id);
			recordset.GetFieldValue(L"device_productor", device_productor);
			recordset.MoveNext();

			sql.Format(L"insert into table_camera \
values(%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d)",
id, ademco_id, sub_machine_id, map_id, x, y, distance, angle, detector_lib_id, device_info_id, device_productor);
			dbdst->exec(utf8::w2a((LPCTSTR)sql));
		}
	}
	recordset.Close();
}

void migrate_sub_machine(std::shared_ptr<CDbOper>& dbsrc, std::shared_ptr<SQLite::Database>& dbdst, int id, int ademco_id, int zone_value) {
	CString sql; sql.Format(L"select * from SubMachine where id=%d", id);
	ado::CADORecordset recordset(dbsrc->GetDatabase());
	JLOG(L"CADORecordset recordset %p\n", &recordset);
	BOOL ret = recordset.Open(dbsrc->GetDatabase()->m_pConnection, sql);
	VERIFY(ret); JLOG(L"recordset.Open() return %d\n", ret);
	DWORD count = recordset.GetRecordCount();

	JLOG(L"recordset.GetRecordCount() return %d\n", count);
	if (count == 1) {
		recordset.MoveFirst();
		for (DWORD i = 0; i < count; i++) {
			long id, status;
			CString /*alias, */contact, address, phone, phone_bk;
			COleDateTime expire_time; double x, y;
			//recordset.GetFieldValue(L"alias", alias);
			recordset.GetFieldValue(L"id", id);
			recordset.GetFieldValue(L"contact", contact);
			recordset.GetFieldValue(L"address", address);
			recordset.GetFieldValue(L"phone", phone);
			recordset.GetFieldValue(L"phone_bk", phone_bk);
			recordset.GetFieldValue(L"expire_time", expire_time);
			recordset.GetFieldValue(L"baidu_x", x);
			recordset.GetFieldValue(L"baidu_y", y);
			recordset.GetFieldValue(L"machine_status", status);

			if (expire_time.GetStatus() != COleDateTime::valid) {
				expire_time = COleDateTime::GetCurrentTime();
			}

			// get baidu map info from json
			int auto_show_map_while_alarm, zoom_level;
			migrate_baidu(dbdst, ademco_id, 1, zone_value, auto_show_map_while_alarm, zoom_level);

			sql.Format(L"insert into table_sub_machine values(%d,%d,'%s','%s','%s','%s','%s',%f,%f,%d,%d)",
					   id, status, contact, address, phone, phone_bk,
					   expire_time.Format(L"%Y-%m-%d %H:%M:%S"), 
					   x, y, zoom_level, auto_show_map_while_alarm);

			dbdst->exec(utf8::w2a((LPCTSTR)sql));
		}
	}
	recordset.Close();
}


void migrate_zone(std::shared_ptr<CDbOper>& dbsrc, std::shared_ptr<SQLite::Database>& dbdst) {
	CString sql; sql = L"select * from ZoneInfo order by id";
	ado::CADORecordset recordset(dbsrc->GetDatabase());
	JLOG(L"CADORecordset recordset %p\n", &recordset);
	BOOL ret = recordset.Open(dbsrc->GetDatabase()->m_pConnection, sql);
	VERIFY(ret); JLOG(L"recordset.Open() return %d\n", ret);
	DWORD count = recordset.GetRecordCount();

	JLOG(L"recordset.GetRecordCount() return %d\n", count);
	if (count > 0) {
		recordset.MoveFirst();
		int ndx = 16;
		double step = 76.0 / count;
		for (DWORD i = 0; i < count; i++) {
			long id, ademco_id, zone_value, type,
				status_or_property, detector_id, sub_machine_id, addr;
			CString alias;
			recordset.GetFieldValue(L"id", id);
			recordset.GetFieldValue(L"ademco_id", ademco_id);
			recordset.GetFieldValue(L"zone_value", zone_value);
			recordset.GetFieldValue(L"type", type);
			recordset.GetFieldValue(L"alias", alias);
			recordset.GetFieldValue(L"detector_info_id", detector_id);
			recordset.GetFieldValue(L"sub_machine_id", sub_machine_id);
			recordset.GetFieldValue(L"status_or_property", status_or_property);
			recordset.GetFieldValue(L"physical_addr", addr);
			recordset.MoveNext();

			if (type == 1) {
				migrate_sub_machine(dbsrc, dbdst, sub_machine_id, ademco_id, zone_value);
			}
			
			sql.Format(L"insert into table_zone values(%d,%d,%d,%d,%d,'%s',%d,%d,%d)",
					   id, ademco_id, sub_machine_id, zone_value, type, alias,
					   status_or_property, addr, detector_id);
					   
			dbdst->exec(utf8::w2a((LPCTSTR)sql));
		}
	}
	recordset.Close();
}

void migrate_sub_zone(std::shared_ptr<CDbOper>& dbsrc, std::shared_ptr<SQLite::Database>& dbdst) {
	CString query;
	query.Format(L"select * from SubZone order by id");
	ado::CADORecordset recordset(dbsrc->GetDatabase());
	recordset.Open(dbsrc->GetDatabase()->m_pConnection, query);
	DWORD count = recordset.GetRecordCount();
	if (count > 0) {
		recordset.MoveFirst();
		for (DWORD i = 0; i < count; i++) {
			CString alias;
			long id, sub_zone_value, detector_info_id;
			recordset.GetFieldValue(L"id", id);
			recordset.GetFieldValue(L"sub_zone", sub_zone_value);
			recordset.GetFieldValue(L"detector_info_id", detector_info_id);
			recordset.GetFieldValue(L"alias", alias);
			recordset.MoveNext();

			query.Format(L"insert into table_sub_zone ([sub_machine_id],[sub_zone_value],[zone_name],[detector_info_id]) values(%d,%d,'%s',%d)",
						 id, sub_zone_value, alias, detector_info_id);
			dbdst->exec(utf8::w2a((LPCTSTR)query));
		}
	}
	recordset.Close();
}


void migrate_hisroty(int ndx) {
	auto dbsrc = std::shared_ptr<CDbOper>(new CDbOper);
	auto dstdb_path = get_exe_path_a() + "\\data\\config\\history.db3";
	std::remove(dstdb_path.c_str());
	auto dbdst = std::shared_ptr<SQLite::Database>(new SQLite::Database(dstdb_path.c_str(), SQLITE_OPEN_READWRITE | SQLITE_OPEN_CREATE));
	dbdst->exec("drop table if exists table_history_record");
	dbdst->exec("create table table_history_record (id integer primary key AUTOINCREMENT, \
ademco_id integer, \
zone_value integer, \
user_id integer, \
level integer, \
record text, \
time text)");

	if (dbsrc->Open(L"HistoryRecord.mdb")) {
		CString query = _T("");
		query.Format(_T("select * from HistoryRecord order by id"));
		ado::CADORecordset dataGridRecord(dbsrc->GetDatabase());
		dataGridRecord.Open(dbsrc->GetDatabase()->m_pConnection, query);
		ULONG count = dataGridRecord.GetRecordCount();
		if (count > 0) {
			double step = 25. / count;
			int prev_pos = 0;
			dataGridRecord.MoveFirst();
			for (ULONG i = 0; i < count; i++) {
				int id = -1, ademco_id = -1, zone_value = -1, user_id = -1, level = -1;
				CString record_content = _T("");
				COleDateTime record_time;
				dataGridRecord.GetFieldValue(_T("id"), id);
				dataGridRecord.GetFieldValue(_T("ademco_id"), ademco_id);
				dataGridRecord.GetFieldValue(_T("zone_value"), zone_value);
				dataGridRecord.GetFieldValue(_T("user_id"), user_id);
				dataGridRecord.GetFieldValue(_T("record"), record_content);
				dataGridRecord.GetFieldValue(_T("time"), record_time);
				dataGridRecord.GetFieldValue(_T("level"), level);

				if (record_time.GetStatus() != COleDateTime::valid) {
					record_time = COleDateTime::GetCurrentTime();
				}

				query.Format(_T("insert into [table_history_record] values(%d,%d,%d,%d,%d,'%s','%s')"),
						   id, ademco_id, zone_value, user_id, level, record_content, record_time.Format(L"%Y-%m-%d %H:%M:%S"));
				dbdst->exec(utf8::w2a((LPCTSTR)query));

				int pos = i * 100 / count;
				if (pos > prev_pos) {
					prev_pos = pos;
					query.Format(L"%2d/100", pos);
					call(add_up(std::make_shared<update_progress>(ndx + int(step * i), 100, (LPCTSTR)query)));
				}
				dataGridRecord.MoveNext();
			}
			query.Format(L"100/100");
			call(add_up(std::make_shared<update_progress>(ndx + 25, 100, (LPCTSTR)query)));
		}
		dataGridRecord.Close();
	}
}


void migrate_service(int ndx) {
	auto dbsrc = std::shared_ptr<CDbOper>(new CDbOper);
	auto dstdb_path = get_exe_path_a() + "\\data\\config\\service.db3";
	std::remove(dstdb_path.c_str());
	auto dbdst = std::shared_ptr<SQLite::Database>(new SQLite::Database(dstdb_path.c_str(), SQLITE_OPEN_READWRITE | SQLITE_OPEN_CREATE));
	
	dbdst->exec("drop table if exists table_consumer_type");
	dbdst->exec("drop table if exists table_consumers");
	dbdst->exec("create table table_consumer_type (id integer primary key AUTOINCREMENT, type_name text)");
	dbdst->exec("create table table_consumers (id integer primary key AUTOINCREMENT, ademco_id integer, zone_value integer, type_id integer, receivable_amount integer, paid_amount integer, remind_time text)");

	if (dbsrc->Open(L"service.mdb")) {
		CString query = _T("");

		// consumer types
		{
			query = L"select * from consumer_type";
			ado::CADORecordset recordset(dbsrc->GetDatabase());
			recordset.Open(dbsrc->GetDatabase()->m_pConnection, query);
			DWORD count = recordset.GetRecordCount();
			if (count > 0) {
				recordset.MoveFirst();
				for (DWORD i = 0; i < count; i++) {
					long id; CString name;
					recordset.GetFieldValue(L"ID", id);
					recordset.GetFieldValue(L"type_name", name);

					query.Format(L"insert into table_consumer_type values(%d,'%s')", id, name);
					dbdst->exec(utf8::w2a((LPCTSTR)query));

					recordset.MoveNext();
				}
			}
			recordset.Close();
		}

		// consumers
		query.Format(_T("select * from consumers order by id"));
		ado::CADORecordset recordset(dbsrc->GetDatabase());
		recordset.Open(dbsrc->GetDatabase()->m_pConnection, query);
		ULONG count = recordset.GetRecordCount();
		if (count > 0) {
			double step = 5. / count;
			int prev_pos = 0;
			recordset.MoveFirst();
			for (ULONG i = 0; i < count; i++) {
				long id, ademco_id, zone_value, type_id, receivable_amount, paid_amount;
				COleDateTime remind_time;
				recordset.GetFieldValue(L"ID", id);
				recordset.GetFieldValue(L"ademco_id", ademco_id);
				recordset.GetFieldValue(L"zone_value", zone_value);
				recordset.GetFieldValue(L"type_id", type_id);
				recordset.GetFieldValue(L"receivable_amount", receivable_amount);
				recordset.GetFieldValue(L"paid_amount", paid_amount);
				recordset.GetFieldValue(L"remind_time", remind_time);
				if (remind_time.GetStatus() != COleDateTime::valid) {
					remind_time = COleDateTime::GetTickCount();
				}

				query.Format(L"insert into table_consumers values(%d,%d,%d,%d,%d,%d,'%s')",
							 id, ademco_id, zone_value, type_id, receivable_amount, paid_amount, remind_time.Format(L"%Y-%m-%d %H:%M:%S"));
				dbdst->exec(utf8::w2a((LPCTSTR)query));

				int pos = i * 100 / count;
				if (pos > prev_pos) {
					prev_pos = pos;
					query.Format(L"%2d/100", pos);
					call(add_up(std::make_shared<update_progress>(ndx + int(step * i), 100, (LPCTSTR)query)));
				}
				recordset.MoveNext();
			}
			query.Format(L"100/100");
			call(add_up(std::make_shared<update_progress>(ndx + 5, 100, (LPCTSTR)query)));
		}
		recordset.Close();
	}
}



void update_database() {
	int progress = 3;

	call(add_up(std::make_shared<update_progress>(progress++, 100, L"migrating database ...")));

	auto dbsrc = std::shared_ptr<CDbOper>(new CDbOper);
	auto dstdb_path = get_exe_path_a() + "\\data\\config\\center.db3";
	std::remove(dstdb_path.c_str());
	auto dbdst = std::shared_ptr<SQLite::Database>(new SQLite::Database(dstdb_path.c_str(), SQLITE_OPEN_READWRITE | SQLITE_OPEN_CREATE));

	if (dbsrc->Open(L"AlarmCenter.mdb")) {
		call(add_up(std::make_shared<update_progress>(progress++, 100, L"migrating database AlarmCenter ...")));
		CString sql;

		// generate new table
		try {
			call(add_up(std::make_shared<update_progress>(progress++, 100, L"generating new sqlite3 db file ...")));
			create_dst_db3(dbdst);
			call(add_up(std::make_shared<update_progress>(progress++, 100, L"ok")));
		} catch (std::exception& e) {
			JLOGA(e.what());
		}

		// migrate csr to center.json
		{
			call(add_up(std::make_shared<update_progress>(progress++, 100, L"migrating csr to center.json ...")));
			migrate_csr(dbsrc);
			call(add_up(std::make_shared<update_progress>(progress++, 100, L"ok")));
		}

		// migrate group info
		{
			call(add_up(std::make_shared<update_progress>(progress++, 100, L"migrating group info ...")));
			migrate_group(dbsrc, dbdst);
			call(add_up(std::make_shared<update_progress>(progress++, 100, L"ok")));
		}

		// migrate map info
		{
			call(add_up(std::make_shared<update_progress>(progress++, 100, L"migrating map info ...")));
			migrate_map(dbsrc, dbdst);
			call(add_up(std::make_shared<update_progress>(progress++, 100, L"ok")));
		}

		// migrate sms config
		{
			call(add_up(std::make_shared<update_progress>(progress++, 100, L"migrating sms info ...")));
			migrate_sms(dbdst);
			call(add_up(std::make_shared<update_progress>(progress++, 100, L"ok")));
		}

		// migrate machine
		{
			progress = 25;
			call(add_up(std::make_shared<update_progress>(progress++, 100, L"migrating machine info ...")));
			migrate_machine(dbsrc, dbdst);
			progress = 50;
			call(add_up(std::make_shared<update_progress>(progress++, 100, L"ok")));
		}

		// migrate detector
		{
			call(add_up(std::make_shared<update_progress>(progress++, 100, L"migrating detector info ...")));
			migrate_detector(dbsrc, dbdst);
			call(add_up(std::make_shared<update_progress>(progress++, 100, L"ok")));
		}
		
		// migrate camera
		{
			call(add_up(std::make_shared<update_progress>(progress++, 100, L"migrating camera info ...")));
			migrate_camera(dbsrc, dbdst);
			call(add_up(std::make_shared<update_progress>(progress++, 100, L"ok")));
		}

		// migrate zone
		{
			call(add_up(std::make_shared<update_progress>(progress++, 100, L"migrating zone info ...")));
			migrate_zone(dbsrc, dbdst);
			call(add_up(std::make_shared<update_progress>(progress++, 100, L"ok")));
		}
		
		// migrate sub_zone
		{
			call(add_up(std::make_shared<update_progress>(progress++, 100, L"migrating sub_zone info ...")));
			migrate_sub_zone(dbsrc, dbdst);
			call(add_up(std::make_shared<update_progress>(progress++, 100, L"ok")));
		}
		
		// migrate hisroty
		{
			call(add_up(std::make_shared<update_progress>(progress++, 100, L"migrating hisroty record ...")));
			migrate_hisroty(progress);
			call(add_up(std::make_shared<update_progress>(progress++, 100, L"ok")));
		}
		
		// migrate service
		{
			call(add_up(std::make_shared<update_progress>(progress++, 100, L"migrating service ...")));
			migrate_service(progress);
			call(add_up(std::make_shared<update_progress>(progress++, 100, L"ok")));
		}
	}
}




void do_backup() {
	call(add_up(std::make_shared<update_progress>(1, 100, get_string_from_resouce(IDS_STRING_BACKUP))));

	auto exe_7z = get_exe_path_a() + "\\7-Zip\\7z.exe";
	auto dst_7z = get_exe_path_a() + "\\backup.7z";
	auto exclude_path = get_exe_path_a() + "\\data\\video_records\\";
	auto output_path = get_exe_path_a() + "\\update_log\\update.txt";

	// test if dst already exsist
	{
		std::ifstream in(dst_7z);
		if (in) {
			in.close();
			std::remove(dst_7z.c_str());
			auto s = now_to_string();
			std::replace(s.begin(), s.end(), ' ', '_');
			std::replace(s.begin(), s.end(), ':', '-');
			dst_7z = get_exe_path_a() + "\\" + s + "backup.7z";
		}
	}

	// build command
	std::string params = " a -t7z"; // compress mod
	params += " \"" + dst_7z + "\""  // dst 7z file path
		+ " \"" + get_exe_path_a() + "\\data\\\""
		+ " -x!\"" + exclude_path + "\""  // exclude path
		+ " -m0=BCJ2 -m1=LZMA:d=21 -mmt -aoa" ;// other compress params
		//+ " -so > \"" + output_path + "\""; // redirect output to file
	auto cmd = exe_7z + params;
	JLOGA(cmd.c_str());


	//execute(cmd);
	/*char buffer[1024] = {};
	std::string result;
	std::shared_ptr<FILE> pipe(_popen(cmd.c_str(), "r"), _pclose);
	bool ok = false;
	do {
		if (!pipe) break;
		const auto npos = std::string::size_type(-1);
		while (!feof(pipe.get())) {
			if (fgets(buffer, 1024, pipe.get()) != nullptr) {
				result += buffer;
				auto pos = result.find_first_of("\r\n");
				while (pos != npos) {
					std::string sub = result.substr(0, pos);
					if (pos != result.size() - 1)
						result = result.substr(pos);
					else
						result.clear();

					if(!sub.empty())
						call(add_up(std::make_shared<update_progress>(2, 100, utf8::a2w(sub))));

					pos = result.find_first_of("\r\n");
				}
			}
		}

		ok = true;
	} while (0);*/
	

	int ret = execute(cmd);
	if (ret <= 1) { // ok
		call(add_up(std::make_shared<update_progress>(2, 100, get_string_from_resouce(IDS_STRING_BACKUP_OK) + utf8::a2w(dst_7z))));
	} else {

		call(add_up(std::make_shared<update_progress>(2, 100, get_string_from_resouce(IDS_STRING_BACKUP_FAIL))));
	}
}


void do_update() {
	call(add_up(std::make_shared<update_progress>(1, 100, get_string_from_resouce(IDS_STRING_START))));
	
	do_backup();

	update_database();

	call(add_up(std::make_shared<update_progress>(100, 100, get_string_from_resouce(IDS_STRING_DONE))));
}

}
using namespace detail;

update_tool::update_tool()
{
	g_instance = this;
}


update_tool::~update_tool()
{
	g_instance = nullptr;
}


void update_tool::add_up(const up_ptr& up)
{
	std::lock_guard<std::mutex> lock(mutex_);
	ups_.push_back(up);
	JLOG(up->msg_.c_str());
}


void update_tool::add_up(int progress, int total, const std::wstring& msg)
{
	std::lock_guard<std::mutex> lock(mutex_);
	ups_.push_back(std::make_shared<update_progress>(progress, total, msg));
	JLOG(msg.c_str());
}


up_ptr update_tool::get_up()
{
	std::lock_guard<std::mutex> lock(mutex_);
	if (!ups_.empty()) {
		auto head = ups_.front();
		ups_.pop_front();
		return head;
	}

	return nullptr;
}


void update_tool::start()
{
	std::thread t(do_update);
	t.detach();
}
