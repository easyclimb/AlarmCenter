#include "stdafx.h"
#include "VideoManager.h"
#include "VideoUserInfoEzviz.h"
#include "VideoUserInfoJovision.h"
#include "VideoDeviceInfoEzviz.h"
#include "VideoDeviceInfoJovision.h"
#include "SdkMgrEzviz.h"
#include "PrivateCloudConnector.h"

#include <iterator>
#include "AlarmCenter.h"
#include "ConfigHelper.h"

#include "sqlitecpp/SQLiteCpp.h"
using namespace SQLite;



namespace video {

//static productor_info ProductorEzviz(EZVIZ, L"", L"", "52c8edc727cd4d4a81bb1d6c7e884fb5");

const productor_info video_manager::GetProductorInfo(int productor)
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

IMPLEMENT_SINGLETON(video_manager)

video_manager::video_manager()
	: _userList()
	, _userListLock()
	, _deviceList()
	, _ezvizDeviceList()
	, _bindMap()
	, _bindMapLock()
	, ProductorEzviz(EZVIZ, L"", L"")
	, ProductorJovision(JOVISION, L"", L"")
	, m_hThread(INVALID_HANDLE_VALUE)
	, m_hEvent(INVALID_HANDLE_VALUE)
{
	auto path = get_config_path() + "\\video.db3";
	db_ = std::make_shared<Database>(path, SQLITE_OPEN_READWRITE | SQLITE_OPEN_CREATE);
	assert(db_);
	if (!db_) { return; }

	try {
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
device_ipv4 integer, \
device_port integer, \
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

	CString ez, ezdesc;
	ez = GetStringFromAppResource(IDS_STRING_EZVIZ);
	ezdesc = GetStringFromAppResource(IDS_STRING_EZVIZ_DESC);
	ProductorEzviz.set_name(ez.LockBuffer());
	ProductorEzviz.set_description(ezdesc.LockBuffer());
	ez.UnlockBuffer();
	ezdesc.UnlockBuffer();

	ez = GetStringFromAppResource(IDS_STRING_JOVISION);
	ezdesc = GetStringFromAppResource(IDS_STRING_JOVISION_DESC);
	ProductorJovision.set_name(ez.LockBuffer());
	ProductorJovision.set_description(ezdesc.LockBuffer());
	ez.UnlockBuffer();
	ezdesc.UnlockBuffer();
}


video_manager::~video_manager()
{
	AUTO_LOG_FUNCTION;
	SetEvent(m_hEvent);
	WaitForSingleObject(m_hThread, INFINITE);
	CLOSEHANDLE(m_hEvent);
	CLOSEHANDLE(m_hThread);

	ezviz::sdk_mgr_ezviz::ReleaseObject();
	ezviz::private_cloud_connector::ReleaseObject();

	_userList.clear();
	_bindMap.clear();
	_deviceList.clear();
	_ezvizDeviceList.clear();
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
	LoadEzvizPrivateCloudInfoFromDB();
	LoadUserInfoFromDB();
	LoadBindInfoFromDB();

	m_hEvent = CreateEvent(nullptr, FALSE, FALSE, nullptr);
	m_hThread = CreateThread(nullptr, 0, ThreadWorker, this, 0, nullptr);
}


DWORD WINAPI video_manager::ThreadWorker(LPVOID lp)
{
	AUTO_LOG_FUNCTION;
	video_manager* mgr = reinterpret_cast<video_manager*>(lp);
	while (true) {
		if (WAIT_OBJECT_0 == WaitForSingleObject(mgr->m_hEvent, 60 * 1000)) { break; }
		mgr->CheckUserAcctkenTimeout();
	}
	return 0;
}


int video_manager::LoadDeviceInfoEzvizFromDB(ezviz::video_user_info_ezviz_ptr userInfo)
{
	AUTO_LOG_FUNCTION;
	assert(userInfo);
	CString sql;
	sql.Format(L"select * from table_device_info_ezviz where user_info_id=%d order by ID",
			   userInfo->get_id());

	Statement query(*db_, utf8::w2a((LPCTSTR)sql));
	int count = 0;
	while (query.executeStep()) {
		ezviz::video_device_info_ezviz_ptr deviceInfo = std::make_shared<ezviz::video_device_info_ezviz>();
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
		userInfo->AddDevice(deviceInfo);
		_deviceList.push_back(deviceInfo);
		_ezvizDeviceList.push_back(deviceInfo);
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
			auto user = std::make_shared<ezviz::video_user_info_ezviz>();
			user->set_id(id);
			user->set_real_user_id(real_user_id);
			user->set_productorInfo(ProductorEzviz);
			user->set_user_name(utf8::a2w(user_name));
			user->set_user_phone(user_phone);

			if (LoadUserInfoEzvizFromDB(user)) {
				_userList.push_back(user);
			}

		}
			break;

		case video::JOVISION:
		{
			auto user = std::make_shared<jovision::video_user_info_jovision>();
			user->set_id(id);
			user->set_real_user_id(real_user_id);
			user->set_productorInfo(ProductorJovision);
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


bool video_manager::LoadUserInfoJovisinoFromDB(const jovision::video_user_info_jovision_ptr& user)
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
		return true;
	}

	return false;
}


bool video_manager::LoadUserInfoEzvizFromDB(const ezviz::video_user_info_ezviz_ptr& user)
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
		RefreshUserEzvizDeviceList(user);
		return true;
	}

	return false;
}


void video_manager::LoadEzvizPrivateCloudInfoFromDB()
{
	AUTO_LOG_FUNCTION;

	auto cfg = util::CConfigHelper::GetInstance();
	if (!ezviz::sdk_mgr_ezviz::GetInstance()->Init(cfg->get_ezviz_private_cloud_app_key())) {
		AfxMessageBox(IDS_STRING_INIT_EZVIZ_SDK_ERROR, MB_ICONEXCLAMATION);
		QuitApplication(0);
	}
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
		video_device_info_ptr device = nullptr;
		if (GetVideoDeviceInfo(device_info_id, GetProductorInfo(productor_info_id).get_productor(), device) && device) {
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


void video_manager::GetVideoUserList(video_user_info_list& list)
{
	std::copy(_userList.begin(), _userList.end(), std::back_inserter(list));
}


void video_manager::GetVideoDeviceList(video_device_info_list& list)
{
	std::copy(_deviceList.begin(), _deviceList.end(), std::back_inserter(list));
}


void video_manager::GetVideoDeviceEzvizWithDetectorList(ezviz::video_device_info_ezviz_list& list)
{
	for (auto dev : _deviceList) {
		if (dev->get_userInfo()->get_productorInfo().get_productor() == EZVIZ) {
			list.push_back(std::dynamic_pointer_cast<video::ezviz::video_device_info_ezviz>(dev));
		}
	}
}


ezviz::video_device_info_ezviz_ptr video_manager::GetVideoDeviceInfoEzviz(int id)
{
	ezviz::video_device_info_ezviz_ptr res;
	for (auto dev : _deviceList) {
		if (dev->get_id() == id) {
			res = std::dynamic_pointer_cast<ezviz::video_device_info_ezviz>(dev);
			break;
		}
	}
	return res;
}


bool video_manager::GetVideoDeviceInfo(int id, productor productor, video_device_info_ptr& device)
{
	for (auto dev : _deviceList) {
		if (dev->get_id() == id && dev->get_userInfo()->get_productorInfo().get_productor() == productor) {
			device = dev;
			return true;
		}
	}
	return false;
}


bool video_manager::DeleteVideoUser(ezviz::video_user_info_ezviz_ptr userInfo)
{
	std::lock_guard<std::mutex> lock(_userListLock);
	assert(userInfo);
	video_device_info_list list;
	userInfo->GetDeviceList(list);
	for (auto dev : list) {
		ezviz::video_device_info_ezviz_ptr device = std::dynamic_pointer_cast<ezviz::video_device_info_ezviz>(dev);
		userInfo->DeleteVideoDevice(device);
		_deviceList.remove(device);
		_ezvizDeviceList.remove(device);
	}
	if (_ezvizDeviceList.size() == 0) {
		Execute(L"update sqlite_sequence set seq=0 where name='table_device_info_ezviz'");
	}

	CString sql;
	sql.Format(L"delete from table_user_info_ezviz where id=%d", userInfo->get_real_user_id());
	if (Execute(sql)) {
		sql.Format(L"delete from table_user_info where ID=%d", userInfo->get_id());
		if (Execute(sql)) {
			ezviz::sdk_mgr_ezviz::GetInstance()->FreeUserSession(userInfo->get_user_phone());
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


bool video_manager::BindZoneAndDevice(const zone_uuid& zoneUuid, ezviz::video_device_info_ezviz_ptr device)
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
				   device->get_id(), device->get_userInfo()->get_productorInfo().get_productor(), 1);
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
		video_device_info_ptr dev = bi._device;
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
			ok = true; break;
		}

		ok = false;
	} while (0);

	return ok;
}


bool video_manager::CheckIfUserEzvizPhoneExists(const std::string& user_phone)
{
	for (auto i : _userList) {
		if (i->get_productorInfo().get_productor() == EZVIZ) {
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
		if (i->get_productorInfo().get_productor() == JOVISION) {
			if (i->get_user_name().compare(user_name) == 0) {
				return true;
			}
		}
	}
	return false;
}


video_manager::VideoEzvizResult video_manager::AddVideoUserEzviz(ezviz::video_user_info_ezviz_ptr user)
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
		user->set_productorInfo(ProductorEzviz);

		_userList.push_back(user);

		RefreshUserEzvizDeviceList(user);

	} while (0);

	return result;
}


video_manager::VideoEzvizResult video_manager::AddVideoUserJovision(jovision::video_user_info_jovision_ptr user)
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
		user->set_productorInfo(ProductorJovision);

		_userList.push_back(user);

	} while (0);

	return result;
}


ezviz::video_user_info_ezviz_ptr video_manager::GetVideoUserEzviz(int id)
{
	ezviz::video_user_info_ezviz_ptr res;
	std::lock_guard<std::mutex> lock(_userListLock);
	for (auto user : _userList) {
		if (user->get_id() == id) {
			res = std::dynamic_pointer_cast<ezviz::video_user_info_ezviz>(user);
			break;
		}
	}
	return res;
}


video_manager::VideoEzvizResult video_manager::RefreshUserEzvizDeviceList(ezviz::video_user_info_ezviz_ptr user)
{
	ezviz::video_device_info_ezviz_list list;
	if (ezviz::sdk_mgr_ezviz::GetInstance()->GetUsersDeviceList(user, list) && list.size() > 0) {
		video_device_info_list localList;
		user->GetDeviceList(localList);
		std::list<int> outstandingDevIdList;
		
		for (auto& localDev : localList) {
			ezviz::video_device_info_ezviz_ptr ezvizDevice = std::dynamic_pointer_cast<ezviz::video_device_info_ezviz>(localDev);
			bool exsist = false;
			for (auto dev : list) {
				if (ezvizDevice->get_deviceId().compare(dev->get_deviceId()) == 0) {
					
					// 2016-5-11 15:26:08 might need to update camera id
					//if (ezvizDevice->get_cameraId() != dev->get_cameraId()) {
						ezvizDevice->set_cameraId(dev->get_cameraId());
						ezvizDevice->set_cameraName(dev->get_cameraName());
						ezvizDevice->set_deviceName(dev->get_deviceName());
						ezvizDevice->execute_update_info();
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
			_deviceList.push_back(dev);
			_ezvizDeviceList.push_back(dev);
			user->execute_add_device(dev);
		}
		
		// 2016-4-18 18:02:16 不再删除设备，让用户手动删
		//for (auto id : outstandingDevIdList) {
		//	for (auto localDev : localList) {
		//		ezviz::video_device_info_ezviz_ptr ezvizDevice = std::dynamic_pointer_cast<ezviz::video_device_info_ezviz>(localDev);
		//		if (ezvizDevice->get_id() == id) {
		//			_deviceList.remove(localDev);
		//			_ezvizDeviceList.remove(ezvizDevice);
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
		if (user->get_productorInfo().get_productor() == EZVIZ) {
			ezviz::video_user_info_ezviz_ptr userEzviz = std::dynamic_pointer_cast<ezviz::video_user_info_ezviz>(user);
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
				
				video::ezviz::sdk_mgr_ezviz* mgr = video::ezviz::sdk_mgr_ezviz::GetInstance();
				if (RESULT_OK == mgr->VerifyUserAccessToken(userEzviz, TYPE_GET)) {
					userEzviz->execute_set_acc_token(userEzviz->get_acc_token());
				}
			}
		}
	}
}




};
