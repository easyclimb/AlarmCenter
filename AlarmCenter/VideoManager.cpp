#include "stdafx.h"
#include "AlarmCenter.h"
#include "VideoManager.h"
#include "../video/ezviz/VideoUserInfoEzviz.h"
#include "../video/jovision/VideoUserInfoJovision.h"
#include "../video/ezviz/VideoDeviceInfoEzviz.h"
#include "../video/jovision/VideoDeviceInfoJovision.h"
#include "alarm_center_video_service.h"
#include <iterator>
#include "AlarmMachineManager.h"

#include "../contrib/sqlitecpp/SQLiteCpp.h"
using namespace SQLite;

namespace video {

//static productor_info ProductorEzviz(EZVIZ, L"", L"", "52c8edc727cd4d4a81bb1d6c7e884fb5");

const productor video_manager::GetProductorInfo(int productor)
{
	switch (productor) {
		case EZVIZ:
			return ProductorEzviz;
			break;
		case JOVISION:
			return ProductorJovision;
			break;
		default:
			return ProductorUnknown;
			break;

	}
}

//IMPLEMENT_SINGLETON(video_manager)

video_manager::video_manager()
	: _userList()
	, _userListLock()
	, device_list_()
	, ezviz_device_list_()
	, jovision_device_list_()
	, _bindMap()
	, _bindMapLock()
	, ProductorEzviz(EZVIZ, L"", L"")
	, ProductorJovision(JOVISION, L"", L"")
{
	auto path = get_config_path_a() + "\\video.db3";
	db_ = std::make_shared<Database>(path, SQLITE_OPEN_READWRITE | SQLITE_OPEN_CREATE);
	assert(db_);
	if (!db_) { return; }

	try {
		bool db_is_empty = true;

		// check if db empty
		{
			Statement query(*db_, "select name from sqlite_master where type='table'");
			if (!query.executeStep()) {
				// init tables
				db_->exec("drop table if exists table_bind_info");
				db_->exec("create table table_bind_info (id integer primary key AUTOINCREMENT, \
ademco_id integer, \
zone_value integer, \
gg_value integer, \
device_info_id integer, \
productor_info_id integer, \
auto_play_when_alarm integer)");

				db_->exec("drop table if exists table_device_info_ezviz");
				db_->exec("create table table_device_info_ezviz (id integer primary key AUTOINCREMENT, \
cameraId text, \
cameraName text, \
cameraNo integer, \
defence integer, \
deviceId integer, \
deviceName text, \
deviceSerial text, \
isEncrypt integer, \
isShared text, \
picUrl text, \
status integer, \
secure_code text, \
device_note text, \
user_info_id integer)");

				db_->exec("drop table if exists table_device_info_jovision");
				db_->exec("create table table_device_info_jovision (id integer primary key AUTOINCREMENT, \
connect_by_sse_or_ip integer, \
cloud_sse_id text, \
device_ipv4 text, \
device_port integer, \
channel_num integer, \
user_name text, \
user_passwd text, \
user_info_id integer, \
device_note text)");

				db_->exec("drop table if exists table_user_info");
				db_->exec("create table table_user_info (id integer primary key AUTOINCREMENT, \
real_user_id integer, \
productor_info_id integer, \
user_name text, \
user_phone text)");

				db_->exec("drop table if exists table_user_info_ezviz");
				db_->exec("create table table_user_info_ezviz (id integer primary key AUTOINCREMENT, \
access_token text, \
token_time text)");

				db_->exec("drop table if exists table_user_info_jovision");
				db_->exec("create table table_user_info_jovision (id integer primary key AUTOINCREMENT, \
global_user_name text, \
global_user_passwd text)");


			} else {
				db_is_empty = false;
			}
		}

		// alter table_device_info_jovision add column channel_num
		bool channel_num_exsits = true;
		if (!db_is_empty) {
			channel_num_exsits = false;
			Statement query(*db_, "select * from table_device_info_jovision limit 0");
			(query.exec());
			{
				for (int i = 0; i < query.getColumnCount(); i++) {
					std::string name = query.getColumnName(i);
					if (name == "channel_num") {
						channel_num_exsits = true;
						break;
					}
				}
			}
		}

		if (!channel_num_exsits) {
			db_->exec("alter table table_device_info_jovision rename to table_device_info_jovision_old");
			db_->exec("create table table_device_info_jovision (id integer primary key AUTOINCREMENT, \
connect_by_sse_or_ip integer, \
cloud_sse_id text, \
device_ipv4 text, \
device_port integer, \
channel_num integer, \
user_name text, \
user_passwd text, \
user_info_id integer, \
device_note text)");

			{
				SQLite::Statement query(*db_, "insert into table_device_info_jovision \
(connect_by_sse_or_ip, cloud_sse_id, device_ipv4, device_port, device_port, user_name, user_passwd, user_info_id, device_note) select \
connect_by_sse_or_ip, cloud_sse_id, device_ipv4, device_port, device_port, user_name, user_passwd, user_info_id, device_note \
from table_device_info_jovision_old");
				query.exec();
				while (!query.isDone()) {
					std::this_thread::sleep_for(std::chrono::seconds(1));
				}
			}
			db_->exec("drop table table_device_info_jovision_old");
			db_->exec("update table_device_info_jovision set channel_num=1");
		}

	} catch (std::exception& e) {
		JLOGA(e.what());
	}

	CString ez, ezdesc;
	ez = TR(IDS_STRING_EZVIZ);
	ezdesc = TR(IDS_STRING_EZVIZ_DESC);
	ProductorEzviz.set_name(ez.LockBuffer());
	ProductorEzviz.set_description(ezdesc.LockBuffer());
	ez.UnlockBuffer();
	ezdesc.UnlockBuffer();

	ez = TR(IDS_STRING_JOVISION);
	ezdesc = TR(IDS_STRING_JOVISION_DESC);
	ProductorJovision.set_name(ez.LockBuffer());
	ProductorJovision.set_description(ezdesc.LockBuffer());
	ez.UnlockBuffer();
	ezdesc.UnlockBuffer();
}


video_manager::~video_manager()
{
	AUTO_LOG_FUNCTION;

	_userList.clear();
	_bindMap.clear();
	device_list_.clear();
	ezviz_device_list_.clear();
	jovision_device_list_.clear();
}


BOOL video_manager::Execute(const CString& sql)
{
	try {
		return db_->exec(utf8::w2a((LPCTSTR)sql)) > 0;
	} catch (std::exception& e) {
		JLOGA(e.what());
	}
	return FALSE;
}


int video_manager::AddAutoIndexTableReturnID(const CString& sql)
{
	try {
		if (db_->exec(utf8::w2a((LPCTSTR)sql)) > 0) {
			return static_cast<int>(db_->getLastInsertRowid());
		}
	} catch (std::exception& e) {
		JLOGA(e.what());
	}
	return -1;
}


void video_manager::LoadFromDB()
{
	_userList.clear();
	device_list_.clear();
	ezviz_device_list_.clear();
	jovision_device_list_.clear();
	_bindMap.clear();

	LoadUserInfoFromDB();
	LoadBindInfoFromDB();
}

int video_manager::LoadDeviceInfoEzvizFromDB(ezviz::ezviz_user_ptr userInfo)
{
	AUTO_LOG_FUNCTION;
	assert(userInfo);
	CString sql;
	sql.Format(L"select * from table_device_info_ezviz where user_info_id=%d order by ID",
			   userInfo->get_id());

	Statement query(*db_, utf8::w2a((LPCTSTR)sql));
	int count = 0;
	while (query.executeStep()) {
		ezviz::ezviz_device_ptr deviceInfo = std::make_shared<ezviz::ezviz_device>();
		int ndx = 0;
		int id = static_cast<int>(query.getColumn(ndx++));
		std::string cameraId = query.getColumn(ndx++).getText();
		std::string cameraName = query.getColumn(ndx++).getText();
		int cameraNo = query.getColumn(ndx++);
		int defence = query.getColumn(ndx++);
		std::string deviceId = query.getColumn(ndx++).getText();
		std::string deviceName = query.getColumn(ndx++).getText();
		std::string deviceSerial = query.getColumn(ndx++).getText();
		int isEncrypt = query.getColumn(ndx++);
		std::string isShared = query.getColumn(ndx++).getText();
		std::string picUrl = query.getColumn(ndx++).getText();
		int status = query.getColumn(ndx++);
		std::string secure_code = query.getColumn(ndx++).getText();
		std::string device_note = query.getColumn(ndx++).getText();

		deviceInfo->set_id(id);
		deviceInfo->set_cameraId(cameraId);

		//wchar_t buffer[1024] = { 0 };
		//utf8::mbcs_to_u16(cameraName.c_str(), buffer, 1024);
		deviceInfo->set_cameraName(utf8::a2w(cameraName));

		deviceInfo->set_cameraNo(cameraNo);
		deviceInfo->set_defence(defence);
		deviceInfo->set_deviceId(deviceId);

		//deviceInfo->set_deviceName(deviceName);
		//utf8::mbcs_to_u16(deviceName.c_str(), buffer, 1024);
		deviceInfo->set_deviceName(utf8::a2w(deviceName));

		deviceInfo->set_deviceSerial(deviceSerial);
		deviceInfo->set_isEncrypt(isEncrypt);
		deviceInfo->set_isShared(isShared);
		deviceInfo->set_picUrl(picUrl);
		deviceInfo->set_status(status);
		deviceInfo->set_secure_code(secure_code);
		deviceInfo->set_device_note(utf8::a2w(device_note));

		deviceInfo->set_userInfo(userInfo);
		userInfo->add_device(deviceInfo);
		device_list_.push_back(deviceInfo);
		ezviz_device_list_.push_back(deviceInfo);
		count++;
	}

	return count;
}


int video_manager::LoadDeviceInfoJovisionFromDB(jovision::jovision_user_ptr userInfo)
{
	AUTO_LOG_FUNCTION;
	assert(userInfo);
	CString sql;
	sql.Format(L"select * from table_device_info_jovision where user_info_id=%d order by ID",
			   userInfo->get_id());

	Statement query(*db_, utf8::w2a((LPCTSTR)sql));
	int count = 0;
	while (query.executeStep()) {
		jovision::jovision_device_ptr deviceInfo = std::make_shared<jovision::jovision_device>();
		int ndx = 0;
		int id = static_cast<int>(query.getColumn(ndx++));
		int connect_by_sse_or_ip = query.getColumn(ndx++);
		std::string cloud_sse_id = query.getColumn(ndx++).getText();
		std::string device_ipv4 = query.getColumn(ndx++).getText();
		int device_port = query.getColumn(ndx++);
		int channel_num = query.getColumn(ndx++);
		std::string user_name = query.getColumn(ndx++).getText();
		std::string user_passwd = query.getColumn(ndx++).getText();
		ndx++; // skip user info id
		std::string device_note = query.getColumn(ndx++).getText();

		deviceInfo->set_id(id);
		deviceInfo->set_by_sse(connect_by_sse_or_ip ? true : false);
		deviceInfo->set_sse(cloud_sse_id);
		deviceInfo->set_ip(device_ipv4);
		deviceInfo->set_port(device_port);
		deviceInfo->set_channel_num(channel_num);
		deviceInfo->set_user_name(utf8::a2w(user_name));
		deviceInfo->set_user_passwd(user_passwd);
		deviceInfo->set_device_note(utf8::a2w(device_note));

		deviceInfo->set_userInfo(userInfo);
		userInfo->add_device(deviceInfo);
		device_list_.push_back(deviceInfo);
		jovision_device_list_.push_back(deviceInfo);
		count++;
	}

	return count;
}


void video_manager::LoadUserInfoFromDB()
{
	Statement query(*db_, "select * from table_user_info order by id");
	while (query.executeStep()) {
		int ndx = 0;
		int id = static_cast<int>(query.getColumn(ndx++));
		int real_user_id = static_cast<int>(query.getColumn(ndx++));
		int productor_info_id = query.getColumn(ndx++);
		std::string user_name = query.getColumn(ndx++).getText();
		std::string user_phone = query.getColumn(ndx++).getText();

		switch (productor_info_id) {
		case video::EZVIZ:
		{
			auto user = std::make_shared<ezviz::ezviz_user>();
			user->set_id(id);
			user->set_real_user_id(real_user_id);
			user->set_productor(ProductorEzviz);
			user->set_user_name(utf8::a2w(user_name));
			user->set_user_phone(user_phone);

			if (LoadUserInfoEzvizFromDB(user)) {
				_userList.push_back(user);
			}

		}
			break;

		case video::JOVISION:
		{
			auto user = std::make_shared<jovision::jovision_user>();
			user->set_id(id);
			user->set_real_user_id(real_user_id);
			user->set_productor(ProductorJovision);
			user->set_user_name(utf8::a2w(user_name));
			user->set_user_phone(user_phone);

			if (LoadUserInfoJovisinoFromDB(user)) {
				_userList.push_back(user);
			}
		}
			break;

		default:
			break;
		}
	}

}


bool video_manager::LoadUserInfoJovisinoFromDB(const jovision::jovision_user_ptr& user)
{
	AUTO_LOG_FUNCTION;
	CString sql;
	sql.Format(L"select global_user_name,global_user_passwd from table_user_info_jovision where id=%d",
			   user->get_real_user_id());
	Statement query(*db_, utf8::w2a((LPCTSTR)sql));
	if (query.executeStep()) {
		std::string global_user_name = query.getColumn(0).getText();
		std::string global_user_passwd = query.getColumn(1).getText();

		user->set_global_user_name(utf8::a2w(global_user_name));
		user->set_global_user_passwd(global_user_passwd);

		LoadDeviceInfoJovisionFromDB(user);
		return true;
	}

	return false;
}


bool video_manager::LoadUserInfoEzvizFromDB(const ezviz::ezviz_user_ptr& user)
{
	AUTO_LOG_FUNCTION;
	CString sql;
	sql.Format(L"select access_token,token_time from table_user_info_ezviz where id=%d",
			   user->get_real_user_id());

	Statement query(*db_, utf8::w2a((LPCTSTR)sql));
	if (query.executeStep()) {
		std::string access_token = query.getColumn(0).getText();
		std::string token_time = query.getColumn(1).getText();

		user->set_acc_token(access_token);
		user->set_token_time(string_to_time_point(token_time));

		LoadDeviceInfoEzvizFromDB(user);		
		return true;
	}

	return false;
}


void video_manager::LoadBindInfoFromDB()
{
	AUTO_LOG_FUNCTION;
	Statement query(*db_, "select * from table_bind_info order by ID");
	while (query.executeStep()) {
		int ndx = 0;
		int id = static_cast<int>(query.getColumn(ndx++));
		int ademco_id = query.getColumn(ndx++);
		int zone_value = query.getColumn(ndx++);
		int gg_value = query.getColumn(ndx++);
		int device_info_id = query.getColumn(ndx++);
		int productor_info_id = query.getColumn(ndx++);
		int auto_play_when_alarm = query.getColumn(ndx++);

		zone_uuid zoneUuid(ademco_id, zone_value, gg_value);
		device_ptr device = nullptr;
		if (GetVideoDeviceInfo(device_info_id, GetProductorInfo(productor_info_id).get_productor_type(), device) && device) {
			device->add_zoneUuid(zoneUuid);
			bind_info bindInfo(id, device, auto_play_when_alarm);
			_bindMap[zoneUuid] = bindInfo;
		}
	}
}


bind_info video_manager::GetBindInfo(const zone_uuid& zone)
{
	std::lock_guard<std::mutex> lock(_bindMapLock);
	bind_info bi(-1, nullptr, 0);
	auto i = _bindMap.find(zone);
	if (i != _bindMap.end()) {
		bi = i->second;
	}
	return bi;
}


void video_manager::GetVideoUserList(user_list& list)
{
	std::copy(_userList.begin(), _userList.end(), std::back_inserter(list));
}


void video_manager::GetVideoDeviceList(device_list& list)
{
	std::copy(device_list_.begin(), device_list_.end(), std::back_inserter(list));
}


ezviz::ezviz_device_ptr video_manager::GetVideoDeviceInfoEzviz(int id)
{
	ezviz::ezviz_device_ptr res;
	for (auto dev : ezviz_device_list_) {
		if (dev->get_id() == id) {
			res = dev;
			break;
		}
	}
	return res;
}


jovision::jovision_device_ptr video_manager::GetVideoDeviceInfoJovision(int id)
{
	jovision::jovision_device_ptr res;
	for (auto dev : jovision_device_list_) {
		if (dev->get_id() == id) {
			res = dev;
			break;
		}
	}
	return res;
}

video::device_ptr video_manager::GetVideoDeviceInfo(video::video_device_identifier * data)
{
	assert(data);
	if (!data) return nullptr;
	for (auto dev : device_list_) {
		if (dev->get_id() == data->dev_id && dev->get_userInfo()->get_productor().get_productor_type() == data->productor_type) {
			return dev;
		}
	}
	return nullptr;
}


bool video_manager::GetVideoDeviceInfo(int id, productor_type productor_type, device_ptr& device)
{
	for (auto dev : device_list_) {
		if (dev->get_id() == id && dev->get_userInfo()->get_productor().get_productor_type() == productor_type) {
			device = dev;
			return true;
		}
	}
	return false;
}




bool video_manager::BindZoneAndDevice(const zone_uuid& zoneUuid, video::device_ptr device)
{
	std::lock_guard<std::mutex> lock(_bindMapLock);
	bool ok = true;
	do {
		assert(device);
		if (/*device->get_binded() || */_bindMap.find(zoneUuid) != _bindMap.end()) {
			ok = false; break;
		}

		CString sql;
		sql.Format(L"insert into table_bind_info ([ademco_id],[zone_value],[gg_value],[device_info_id],[productor_info_id],[auto_play_when_alarm]) values(%d,%d,%d,%d,%d,%d)",
				   zoneUuid._ademco_id, zoneUuid._zone_value, zoneUuid._gg,
				   device->get_id(), device->get_userInfo()->get_productor().get_productor_type(), 1);
		int id = AddAutoIndexTableReturnID(sql);
		if (id == -1) {
			ok = false; break;
		}

		device->add_zoneUuid(zoneUuid);
		bind_info bi(id, device, 1);
		_bindMap[zoneUuid] = bi;

		ipc::alarm_center_video_service::get_instance()->update_db();

		ok = true;
	} while (0);

	return ok;
}


bool video_manager::UnbindZoneAndDevice(const zone_uuid& zoneUuid)
{
	std::lock_guard<std::mutex> lock(_bindMapLock);
	bool ok = false;
	do {
		auto iter = _bindMap.find(zoneUuid);
		if (iter == _bindMap.end()) { ok = true; break; }

		bind_info bi = iter->second;
		device_ptr dev = bi._device;
		if (!dev) {
			_bindMap.erase(iter);
			if (_bindMap.size() == 0) {
				Execute(L"update sqlite_sequence set seq=0 where name='table_bind_info'");
			}
			ok = true; break;
		}

		CString sql; sql.Format(L"delete from table_bind_info where ID=%d", bi._id);

		if (Execute(sql)) {
			dev->del_zoneUuid(zoneUuid);
			_bindMap.erase(zoneUuid);
			if (_bindMap.size() == 0) {
				Execute(L"update sqlite_sequence set seq=0 where name='bind_info'");
			}

			auto mgr = core::alarm_machine_manager::get_instance();
			mgr->DeleteCameraInfo(dev->get_id(), dev->get_userInfo()->get_productor().get_productor_type());

			ok = true; break;
		}

		ok = false;
	} while (0);

	if (ok) {

		ipc::alarm_center_video_service::get_instance()->update_db();
	}

	return ok;
}

ezviz::ezviz_user_ptr video_manager::GetVideoUserEzviz(int id)
{
	ezviz::ezviz_user_ptr res;
	std::lock_guard<std::mutex> lock(_userListLock);
	for (auto user : _userList) {
		if (user->get_id() == id) {
			res = std::dynamic_pointer_cast<ezviz::ezviz_user>(user);
			break;
		}
	}
	return res;
}

jovision::jovision_user_ptr video_manager::GetVideoUserJovision(int id)
{
	jovision::jovision_user_ptr res = nullptr;
	std::lock_guard<std::mutex> lock(_userListLock);
	for (auto user : _userList) {
		if (user->get_id() == id) {
			res = std::dynamic_pointer_cast<jovision::jovision_user>(user);
			break;
		}
	}
	return res;
}

bool video_manager::SetBindInfoAutoPlayVideoOnAlarm(const zone_uuid& zone, int auto_play_when_alarm)
{
	std::lock_guard<std::mutex> lock(_bindMapLock);
	bool ok = true;
	do {
		auto&& iter = _bindMap.find(zone);
		if (iter == _bindMap.end()) { ok = false; break; }
		CString sql;
		sql.Format(L"update table_bind_info set auto_play_when_alarm=%d where ID=%d", auto_play_when_alarm, iter->second._id);
		if (Execute(sql)) {
			iter->second.auto_play_when_alarm_ = auto_play_when_alarm;
			 ok = true; break; 
		}

		ok = false;
	} while (0);

	if (ok) {
		ipc::alarm_center_video_service::get_instance()->update_db();
	}

	return ok;
}


};
