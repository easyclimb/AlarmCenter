#include "stdafx.h"
#include "AlarmCenterVideo.h"
#include "VideoManager.h"
#include "../video/ezviz/VideoUserInfoEzviz.h"
#include "../video/jovision/VideoUserInfoJovision.h"
#include "../video/ezviz/VideoDeviceInfoEzviz.h"
#include "../video/jovision/VideoDeviceInfoJovision.h"
#include "SdkMgrEzviz.h"
#include <iterator>
#include "alarm_center_video_client.h"

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
	auto path = get_config_path() + "\\video.db3";
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

	if (running_) {
		{
			std::lock_guard<std::mutex> lg(mutex_);
			running_ = false;
		}
		
		condvar_.notify_one();
		thread_.join();
	}

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


void video_manager::LoadFromDB(bool refresh_ezviz_users_device_list)
{
	_userList.clear();
	device_list_.clear();
	ezviz_device_list_.clear();
	jovision_device_list_.clear();
	_bindMap.clear();

	LoadUserInfoFromDB(refresh_ezviz_users_device_list);
	LoadBindInfoFromDB();

	if (!running_) {
		running_ = true;
		thread_ = std::thread(&video_manager::ThreadWorker, this);
	}
}


void video_manager::ThreadWorker()
{
	AUTO_LOG_FUNCTION;
	while (running_) {
		{
			std::unique_lock<std::mutex> ul(mutex_);
			condvar_.wait_for(ul, std::chrono::milliseconds(60 * 1000), [this]() {return !running_; });
		}

		if (!running_) {
			break;
		}

		CheckUserAcctkenTimeout();
	}
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


void video_manager::LoadUserInfoFromDB(bool refresh_ezviz_users_device_list)
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

			if (LoadUserInfoEzvizFromDB(user, refresh_ezviz_users_device_list)) {
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

	// resolve dev list from ezviz cloud
	CheckUserAcctkenTimeout();
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


bool video_manager::LoadUserInfoEzvizFromDB(const ezviz::ezviz_user_ptr& user, bool refresh_ezviz_users_device_list)
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

		if (refresh_ezviz_users_device_list) {
			RefreshUserEzvizDeviceList(user);
		}

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


bool video_manager::DeleteVideoUserEzviz(ezviz::ezviz_user_ptr userInfo)
{
	std::lock_guard<std::mutex> lock(_userListLock);
	assert(userInfo);
	device_list list = userInfo->get_device_list();
	for (auto dev : list) {
		ezviz::ezviz_device_ptr device = std::dynamic_pointer_cast<ezviz::ezviz_device>(dev);
		execute_del_ezviz_users_device(userInfo, device);
		device_list_.remove(device);
		ezviz_device_list_.remove(device);
	}

	if (ezviz_device_list_.size() == 0) {
		Execute(L"update sqlite_sequence set seq=0 where name='table_device_info_ezviz'");
	}

	CString sql;
	sql.Format(L"delete from table_user_info_ezviz where id=%d", userInfo->get_real_user_id());
	if (Execute(sql)) {
		sql.Format(L"delete from table_user_info where ID=%d", userInfo->get_id());
		if (Execute(sql)) {
			ezviz::sdk_mgr_ezviz::get_instance()->FreeUserSession(userInfo->get_user_phone());

			_userList.remove(userInfo);
			if (_userList.size() == 0) {
				Execute(L"update sqlite_sequence set seq=0 where name='table_user_info'");
				Execute(L"update sqlite_sequence set seq=0 where name='table_user_info_ezviz'");
			}
			return true;
		}
	}
	
	return false;
}


bool video_manager::DeleteVideoUserJovision(jovision::jovision_user_ptr userInfo)
{
	std::lock_guard<std::mutex> lock(_userListLock);
	assert(userInfo);
	device_list list = userInfo->get_device_list();
	for (auto dev : list) {
		jovision::jovision_device_ptr device = std::dynamic_pointer_cast<jovision::jovision_device>(dev);
		execute_del_jovision_users_device(userInfo, device);
		device_list_.remove(device);
		jovision_device_list_.remove(device);
	}

	if (ezviz_device_list_.size() == 0) {
		Execute(L"update sqlite_sequence set seq=0 where name='table_device_info_jovision'");
	}

	CString sql;
	sql.Format(L"delete from table_user_info_jovision where id=%d", userInfo->get_real_user_id());
	if (Execute(sql)) {
		sql.Format(L"delete from table_user_info where ID=%d", userInfo->get_id());
		if (Execute(sql)) {
			ezviz::sdk_mgr_ezviz::get_instance()->FreeUserSession(userInfo->get_user_phone());
			_userList.remove(userInfo);
			if (_userList.size() == 0) {
				Execute(L"update sqlite_sequence set seq=0 where name='table_user_info'");
				Execute(L"update sqlite_sequence set seq=0 where name='table_user_info_jovision'");
			}
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

			//auto hr = core::history_record_manager::get_instance();

			ok = true; break;
		}

		ok = false;
	} while (0);

	return ok;
}


bool video_manager::CheckIfUserEzvizPhoneExists(const std::string& user_phone)
{
	for (auto i : _userList) {
		if (i->get_productor().get_productor_type() == EZVIZ) {
			if (i->get_user_phone().compare(user_phone) == 0) {
				return true;
			}
		}
	}
	return false;
}


bool video_manager::CheckIfUserJovisionNameExists(const std::wstring& user_name)
{
	for (auto i : _userList) {
		if (i->get_productor().get_productor_type() == JOVISION) {
			if (i->get_user_name().compare(user_name) == 0) {
				return true;
			}
		}
	}
	return false;
}


video_manager::VideoEzvizResult video_manager::AddVideoUserEzviz(ezviz::ezviz_user_ptr user)
{
	AUTO_LOG_FUNCTION;
	std::lock_guard<std::mutex> lock(_userListLock);
	VideoEzvizResult result = RESULT_OK;
	do {
		user->set_token_time(std::chrono::system_clock::now());
		CString sql; // [user_phone],[user_name],[[productor_info_id],
		sql.Format(L"insert into table_user_info_ezviz ([access_token],[token_time]) values('%s','%s')",
				   utf8::a2w(user->get_acc_token()).c_str(), time_point_to_wstring(user->get_token_time()).c_str());
		int id = AddAutoIndexTableReturnID(sql);
		if (id == -1) {
			result = RESULT_INSERT_TO_DB_FAILED; break;
		}
		user->set_real_user_id(id);

		sql.Format(L"insert into table_user_info ([real_user_id], [productor_info_id], [user_name], [user_phone]) values(%d,%d,'%s','%s')",
				   id, EZVIZ, user->get_user_name().c_str(), utf8::a2w(user->get_user_phone()).c_str());
		id = AddAutoIndexTableReturnID(sql);
		if (id == -1) {
			result = RESULT_INSERT_TO_DB_FAILED; break;
		}

		user->set_id(id);
		user->set_productor(ProductorEzviz);

		_userList.push_back(user);

		RefreshUserEzvizDeviceList(user);

	} while (0);

	return result;
}


video_manager::VideoEzvizResult video_manager::AddVideoUserJovision(jovision::jovision_user_ptr user)
{
	AUTO_LOG_FUNCTION;
	std::lock_guard<std::mutex> lock(_userListLock);
	VideoEzvizResult result = RESULT_OK;
	do {
		CString sql; // [user_phone],[user_name],[[productor_info_id],
		sql.Format(L"insert into table_user_info_jovision ([global_user_name],[global_user_passwd]) values('%s','%s')",
				   user->get_global_user_name().c_str(), utf8::a2w(user->get_global_user_passwd()).c_str());

		int id = AddAutoIndexTableReturnID(sql);
		if (id == -1) {
			result = RESULT_INSERT_TO_DB_FAILED; break;
		}
		user->set_real_user_id(id);

		sql.Format(L"insert into table_user_info ([real_user_id], [productor_info_id], [user_name], [user_phone]) values(%d,%d,'%s','%s')",
				   id, JOVISION, user->get_user_name().c_str(), utf8::a2w(user->get_user_phone()).c_str());
		id = AddAutoIndexTableReturnID(sql);
		if (id == -1) {
			result = RESULT_INSERT_TO_DB_FAILED; break;
		}

		user->set_id(id);
		user->set_productor(ProductorJovision);

		_userList.push_back(user);

	} while (0);

	return result;
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


video_manager::VideoEzvizResult video_manager::RefreshUserEzvizDeviceList(ezviz::ezviz_user_ptr user)
{
	ezviz::ezviz_device_list list;
	if (ezviz::sdk_mgr_ezviz::get_instance()->GetUsersDeviceList(user, list) && list.size() > 0) {
		device_list localList = user->get_device_list();
		std::list<int> outstandingDevIdList;
		
		for (auto& localDev : localList) {
			ezviz::ezviz_device_ptr ezvizDevice = std::dynamic_pointer_cast<ezviz::ezviz_device>(localDev);
			bool exsist = false;
			for (auto dev : list) {
				if (ezvizDevice->get_deviceId().compare(dev->get_deviceId()) == 0) {
					
					// 2016-5-11 15:26:08 might need to update camera id
					//if (ezvizDevice->get_cameraId() != dev->get_cameraId()) {
						ezvizDevice->set_cameraId(dev->get_cameraId());
						ezvizDevice->set_cameraName(dev->get_cameraName());
						ezvizDevice->set_deviceName(dev->get_deviceName());
						execute_update_ezviz_dev(ezvizDevice);
					//}

					exsist = true;
					list.remove(dev);
					break;
				} 
			}
			if (!exsist) {
				// delete dev
				outstandingDevIdList.push_back(ezvizDevice->get_id());
			}
		}

		for (auto dev : list) {
			device_list_.push_back(dev);
			ezviz_device_list_.push_back(dev);
			execute_add_device_for_ezviz_user(user, dev);
		}
		
		// 2016-4-18 18:02:16 不再删除设备，让用户手动删
		//for (auto id : outstandingDevIdList) {
		//	for (auto localDev : localList) {
		//		ezviz::ezviz_device_ptr ezvizDevice = std::dynamic_pointer_cast<ezviz::ezviz_device>(localDev);
		//		if (ezvizDevice->get_id() == id) {
		//			device_list_.remove(localDev);
		//			ezviz_device_list_.remove(ezvizDevice);
		//			
		//			std::list<zone_uuid> zoneList;
		//			for (auto bi : _bindMap) {
		//				if (bi.second._device == ezvizDevice) {
		//					zoneList.push_back(bi.first);
		//				}
		//			}
		//			for (auto zoneUuid : zoneList) {
		//				_bindMap.erase(zoneUuid);
		//			}
		//			user->DeleteVideoDevice(ezvizDevice); // it will delete memory.
		//			break;
		//		}
		//	}
		//
		//	CString query;
		//	query.Format(L"delete from device_info_ezviz where id=%d", id);
		//	m_db->Execute(query);
		//	query.Format(L"delete from bind_info where device_info_id=%d", id);
		//	m_db->Execute(query);
		//}
		return RESULT_OK;
	}
	return RESULT_PRIVATE_CLOUD_CONNECT_FAILED_OR_USER_NOT_EXIST;
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
	return ok;
}


void video_manager::CheckUserAcctkenTimeout()
{
	std::lock_guard<std::mutex> lock(_userListLock);
	for (auto user : _userList) {
		if (user->get_productor().get_productor_type() == EZVIZ) {
			ezviz::ezviz_user_ptr userEzviz = std::dynamic_pointer_cast<ezviz::ezviz_user>(user);
			auto now = std::chrono::system_clock::now();
			auto diff = now - userEzviz->get_token_time();
#if 0
			if (span.GetTotalDays() > 1) {
				JLOG(L"video_manager::CheckUserAcctkenTimeout(), old %s, now %s, %d days has passed, the user %s's accToken should be re-get\n", 
					userEzviz->get_user_tokenTime().Format(L"%Y-%m-%d %H:%M:%S"), 
					now.Format(L"%Y-%m-%d %H:%M:%S"), 1, userEzviz->get_user_name().c_str());
#else
			if (std::chrono::duration_cast<std::chrono::hours>(diff).count() > 6 * 24) {
				JLOG(L"video_manager::CheckUserAcctkenTimeout(), old %s, now %s, %d days has passed, the user %s's accToken should be re-get\n",
					time_point_to_wstring(userEzviz->get_token_time()).c_str(),
					 time_point_to_wstring(now).c_str(), 6, userEzviz->get_user_name().c_str());
#endif
				
				auto mgr = video::ezviz::sdk_mgr_ezviz::get_instance();
				auto vmgr = video::video_manager::get_instance();
				if (RESULT_OK == mgr->VerifyUserAccessToken(userEzviz, TYPE_GET)) {
					vmgr->execute_set_ezviz_users_acc_token(userEzviz, userEzviz->get_acc_token());
				}
			}
		}
	}
}


bool video_manager::execute_set_user_name(const user_ptr & user, const std::wstring & name)
{
	AUTO_LOG_FUNCTION;
	CString sql;
	sql.Format(L"update table_user_info set user_name='%s' where ID=%d",
			   name.c_str(), user->get_id());
	if (Execute(sql)) {
		user->set_user_name(name);
		return true;
	}
	return false;
}

bool video_manager::execute_update_dev(const device_ptr & device)
{
	auto type = device->get_userInfo()->get_productor().get_productor_type();
	switch (type) {
	case video::EZVIZ:
		return execute_update_ezviz_dev(std::dynamic_pointer_cast<video::ezviz::ezviz_device>(device));
		break;
	case video::JOVISION:
		return execute_update_jovision_dev(std::dynamic_pointer_cast<video::jovision::jovision_device>(device));
		break;
	default:
		assert(0);
		return false;
		break;
	}
}

bool video_manager::execute_set_ezviz_users_acc_token(const video::ezviz::ezviz_user_ptr & user, const std::string & accToken)
{
	CString sql;
	sql.Format(L"update table_user_info_ezviz set access_token='%s' where ID=%d",
			   utf8::a2w(accToken).c_str(), user->get_real_user_id());
	if (Execute(sql)) {
		user->set_acc_token(accToken);
		return execute_set_ezviz_users_token_time(user, std::chrono::system_clock::now());;
	}
	return false;
}

bool video_manager::execute_set_ezviz_users_token_time(const video::ezviz::ezviz_user_ptr & user, const std::chrono::system_clock::time_point & tp)
{
	CString sql;
	sql.Format(L"update table_user_info_ezviz set token_time='%s' where ID=%d", time_point_to_wstring(tp).c_str(), user->get_real_user_id());
	if (video_manager::get_instance()->Execute(sql)) {
		user->set_token_time(tp);
		return true;
	}
	return false;
}

bool video_manager::execute_add_device_for_ezviz_user(const video::ezviz::ezviz_user_ptr & user, const video::ezviz::ezviz_device_ptr & device)
{
	CString sql;
	sql.Format(L"insert into table_device_info_ezviz \
([cameraId],[cameraName],[cameraNo],[defence],[deviceId],[deviceName],[deviceSerial],\
[isEncrypt],[isShared],[picUrl],[status],[secure_code],[device_note],[user_info_id]) \
values('%s','%s',%d,%d,'%s','%s','%s',%d,'%s','%s',%d,'%s','%s',%d)",
utf8::a2w(device->get_cameraId()).c_str(),
device->get_cameraName().c_str(),
device->get_cameraNo(),
device->get_defence(),
utf8::a2w(device->get_deviceId()).c_str(),
device->get_deviceName().c_str(),
utf8::a2w(device->get_deviceSerial()).c_str(),
device->get_isEncrypt(),
utf8::a2w(device->get_isShared()).c_str(),
utf8::a2w(device->get_picUrl()).c_str(),
device->get_status(),
utf8::a2w(device->get_secure_code()).c_str(),
device->get_device_note().c_str(),
user->get_real_user_id());

	int id = video_manager::get_instance()->AddAutoIndexTableReturnID(sql);
	if (id != -1) {
		device->set_id(id);
		device->set_userInfo(user);
		user->add_device(device);
		return true;
	}
	return false;
}

bool video_manager::execute_del_ezviz_users_device(const video::ezviz::ezviz_user_ptr & user, const video::ezviz::ezviz_device_ptr & device)
{
	assert(device);
	bool ok = true;
	std::list<zone_uuid> zoneList;
	device->get_zoneUuidList(zoneList);
	for (auto zone : zoneList) {
		ok = video_manager::get_instance()->UnbindZoneAndDevice(zone);
		if (!ok) {
			return ok;
		}
	}
	if (ok) {
		CString sql;
		sql.Format(L"delete from table_device_info_ezviz where ID=%d", device->get_id());
		ok = video_manager::get_instance()->Execute(sql) ? true : false;
	}
	if (ok) {
		ipc::alarm_center_video_client::get_instance()->delete_camera_info(device->get_id(), device->get_userInfo()->get_productor().get_productor_type());
		user->rm_device(device);
		ezviz_device_list_.remove(device);
		device_list_.remove(device);
	}
	return ok;
}

bool video_manager::execute_update_ezviz_dev(const video::ezviz::ezviz_device_ptr & device)
{
	CString sql;
	sql.Format(L"update table_device_info_ezviz set \
cameraId='%s',cameraName='%s',cameraNo=%d,defence=%d,deviceId='%s', \
deviceName='%s',deviceSerial='%s',isEncrypt=%d,isShared='%s',picUrl='%s',\
status=%d,secure_code='%s',device_note='%s',user_info_id=%d where ID=%d", // detector_info_id=%d 
utf8::a2w(device->get_cameraId()).c_str(),
device->get_cameraName().c_str(),
device->get_cameraNo(),
device->get_defence(),
utf8::a2w(device->get_deviceId()).c_str(),
device->get_deviceName().c_str(),
utf8::a2w(device->get_deviceSerial()).c_str(),
device->get_isEncrypt(),
utf8::a2w(device->get_isShared()).c_str(),
utf8::a2w(device->get_picUrl()).c_str(),
device->get_status(),
utf8::a2w(device->get_secure_code()).c_str(),
device->get_device_note().c_str(),
device->get_userInfo()->get_id(),
device->get_id());

	return Execute(sql) ? true : false;
}

bool video_manager::execute_add_device_for_jovision_user(const video::jovision::jovision_user_ptr & user, const video::jovision::jovision_device_ptr & dev)
{
	CString sql;
	sql.Format(L"insert into table_device_info_jovision values(NULL,%d,'%s','%s',%d,%d,'%s','%s',%d,'%s')",
			   dev->get_by_sse(), utf8::a2w(dev->get_sse()).c_str(), utf8::a2w(dev->get_ip()).c_str(),
			   dev->get_port(), dev->get_channel_num(), dev->get_user_name().c_str(), utf8::a2w(dev->get_user_passwd()).c_str(),
			   user->get_id(), dev->get_device_note().c_str());
	int id = AddAutoIndexTableReturnID(sql);
	if (id < 0) return false;
	dev->set_id(id);
	user->add_device(dev);
	return true;
}

bool video_manager::execute_set_jovision_users_global_user_name(const video::jovision::jovision_user_ptr & user, const std::wstring & name)
{
	std::wstringstream ss;
	ss << L"update table_user_info_jovision set global_user_name='" << name << L"' where id=" << user->get_real_user_id();
	if (Execute(ss.str().c_str())) {
		user->set_global_user_name(name);
		video::device_list list = user->get_device_list();
		for (auto device : list) {
			auto dev = std::dynamic_pointer_cast<jovision::jovision_device>(device);
			dev->set_user_name(name);
			execute_update_jovision_dev(dev);
		}
		return true;
	}
	return false;
}

bool video_manager::execute_set_jovision_users_global_user_passwd(const video::jovision::jovision_user_ptr & user, const std::string & passwd)
{
	std::wstringstream ss;
	ss << L"update table_user_info_jovision set global_user_passwd='" << utf8::a2w(passwd) << L"' where id=" << user->get_real_user_id();
	if (Execute(ss.str().c_str())) {
		user->set_global_user_passwd(passwd);
		video::device_list list = user->get_device_list();
		for (auto device : list) {
			auto dev = std::dynamic_pointer_cast<jovision::jovision_device>(device);
			dev->set_user_passwd(passwd);
			execute_update_jovision_dev(dev);
		}
		return true;
	}
	return false;
}

bool video_manager::execute_del_jovision_users_device(video::jovision::jovision_user_ptr & user, video::jovision::jovision_device_ptr & device)
{
	assert(device);
	bool ok = true;
	std::list<zone_uuid> zoneList;
	device->get_zoneUuidList(zoneList);

	for (auto zone : zoneList) {
		ok = UnbindZoneAndDevice(zone);
		if (!ok) {
			return ok;
		}
	}

	if (ok) {
		CString sql;
		sql.Format(L"delete from table_device_info_jovision where ID=%d", device->get_id());
		ok = Execute(sql) ? true : false;
	}

	if (ok) {
		ipc::alarm_center_video_client::get_instance()->delete_camera_info(device->get_id(), device->get_userInfo()->get_productor().get_productor_type());
		user->rm_device(device);
		jovision_device_list_.remove(device);
		device_list_.remove(device);
	}

	return ok;
}

bool video_manager::execute_update_jovision_dev(const video::jovision::jovision_device_ptr & dev)
{
	CString sql;
	sql.Format(L"update table_device_info_jovision set \
connect_by_sse_or_ip=%d,\
cloud_sse_id='%s',\
device_ipv4='%s',\
device_port=%d,\
channel_num=%d,\
user_name='%s',\
user_passwd='%s',\
user_info_id=%d,\
device_note='%s' where id=%d",
dev->get_by_sse() ? 1 : 0,
utf8::a2w(dev->get_sse()).c_str(),
utf8::a2w(dev->get_ip()).c_str(),
dev->get_port(),
dev->get_channel_num(),
dev->get_user_name().c_str(),
utf8::a2w(dev->get_user_passwd()).c_str(),
dev->get_userInfo()->get_id(),
dev->get_device_note().c_str(),
dev->get_id());

	return Execute(sql) ? true : false;
}

bool video_manager::AddVideoDeviceJovision(const jovision::jovision_user_ptr& user, const jovision::jovision_device_ptr& device)
{
	if (execute_add_device_for_jovision_user(user, device)) {
		jovision_device_list_.push_back(device);
		device_list_.push_back(device);
		device->set_userInfo(user);
		return true;
	}
	return false;
}

};
