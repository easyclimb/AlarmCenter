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


void update_database() {
	call(add_up(std::make_shared<update_progress>(3, 100, L"migrating database ...")));

	auto dbsrc = std::unique_ptr<CDbOper>(new CDbOper);
	auto dstdb_path = get_exe_path_a() + "\\data\\config\\alarm_center.db3";
	std::remove(dstdb_path.c_str());
	auto dbdst = std::unique_ptr<SQLite::Database>(new SQLite::Database(dstdb_path.c_str(), SQLITE_OPEN_READWRITE | SQLITE_OPEN_CREATE));

	if (dbsrc->Open(L"AlarmCenter.mdb")) {
		call(add_up(std::make_shared<update_progress>(4, 100, L"migrating database AlarmCenter ...")));
		CString sql;

		// migrate csr to center.json
		{
			call(add_up(std::make_shared<update_progress>(5, 100, L"migrating database AlarmCenter migrate csr to center.json...")));
			sql = L"select * from CsrInfo";
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
					call(add_up(std::make_shared<update_progress>(5, 100, L"migrating database AlarmCenter migrate csr to center.json ok")));
				}
			}
		}

		// migrate group info
		{
			call(add_up(std::make_shared<update_progress>(6, 100, L"migrating database AlarmCenter migrate group info...")));

			// generate new table
			try {
				dbdst->exec("drop table if exists table_group");
				dbdst->exec("create table table_group (id integer primary key AUTOINCREMENT, \
group_name text, \
parent_group_id integer)");
			} catch (std::exception& e) {
				JLOGA(e.what());
			}
			sql = L"select * from GroupInfo order by id";
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
			call(add_up(std::make_shared<update_progress>(5, 100, L"migrating database AlarmCenter migrate group info ok")));
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
	
	//do_backup();

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
