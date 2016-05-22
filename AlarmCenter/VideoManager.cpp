#include "stdafx.h"
#include "VideoManager.h"
#include "DbOper.h"
#include "VideoUserInfoEzviz.h"
#include "VideoUserInfoNormal.h"
#include "VideoDeviceInfoEzviz.h"
#include "VideoDeviceInfoNormal.h"
#include "SdkMgrEzviz.h"
#include "PrivateCloudConnector.h"

#include <iterator>
#include "AlarmCenter.h"
#include "ConfigHelper.h"


namespace video {

//static CProductorInfo ProductorEzviz(EZVIZ, L"", L"", "52c8edc727cd4d4a81bb1d6c7e884fb5");

const CProductorInfo CVideoManager::GetProductorInfo(int productor)
{
	switch (productor) {
		case EZVIZ:
			return ProductorEzviz;
			break;
		default:
			return ProductorUnknown;
			break;

	}
}

IMPLEMENT_SINGLETON(CVideoManager)

CVideoManager::CVideoManager()
	: m_db(nullptr)
	, _userList()
	, _userListLock()
	, _deviceList()
	, _ezvizDeviceList()
	, _bindMap()
	, _bindMapLock()
	, ProductorEzviz(EZVIZ, L"", L"", "52c8edc727cd4d4a81bb1d6c7e884fb5")
	, m_hThread(INVALID_HANDLE_VALUE)
	, m_hEvent(INVALID_HANDLE_VALUE)
{
	m_db = std::make_unique<ado::CDbOper>();
	m_db->Open(L"video.mdb");

	// 
	CString ez, ezdesc;
	ez = GetStringFromAppResource(IDS_STRING_EZVIZ);
	ezdesc = GetStringFromAppResource(IDS_STRING_EZVIZ_DESC);
	ProductorEzviz.set_name(ez.LockBuffer());
	ProductorEzviz.set_description(ezdesc.LockBuffer());
	ez.UnlockBuffer();
	ezdesc.UnlockBuffer();
}


CVideoManager::~CVideoManager()
{
	SetEvent(m_hEvent);
	WaitForSingleObject(m_hThread, INFINITE);
	CLOSEHANDLE(m_hEvent);
	CLOSEHANDLE(m_hThread);

	ezviz::CSdkMgrEzviz::ReleaseObject();
	ezviz::CPrivateCloudConnector::ReleaseObject();

	_userList.clear();
	_bindMap.clear();
	_deviceList.clear();
	_ezvizDeviceList.clear();
}


BOOL CVideoManager::Execute(const CString& sql)
{
	return m_db->Execute(sql);
}


int CVideoManager::AddAutoIndexTableReturnID(const CString& query)
{
	return m_db->AddAutoIndexTableReturnID(query);
}


void CVideoManager::LoadFromDB()
{
	LoadEzvizPrivateCloudInfoFromDB();
	LoadUserInfoEzvizFromDB();
	LoadBindInfoFromDB();

	m_hEvent = CreateEvent(nullptr, FALSE, FALSE, nullptr);
	m_hThread = CreateThread(nullptr, 0, ThreadWorker, this, 0, nullptr);
}


DWORD WINAPI CVideoManager::ThreadWorker(LPVOID lp)
{
	AUTO_LOG_FUNCTION;
	CVideoManager* mgr = reinterpret_cast<CVideoManager*>(lp);
	while (true) {
		if (WAIT_OBJECT_0 == WaitForSingleObject(mgr->m_hEvent, 60 * 1000)) { break; }
		mgr->CheckUserAcctkenTimeout();
	}
	return 0;
}


int CVideoManager::LoadDeviceInfoEzvizFromDB(ezviz::CVideoUserInfoEzvizPtr userInfo)
{
	AUTO_LOG_FUNCTION;
	USES_CONVERSION;
	CString query;
	query.Format(L"select * from device_info_ezviz where user_info_id=%d order by ID",
				 userInfo->get_id());
	ado::CADORecordset recordset(m_db->GetDatabase());
	JLOG(L"CADORecordset recordset %p\n", &recordset);
	BOOL ret = recordset.Open(m_db->GetDatabase()->m_pConnection, query);
	VERIFY(ret); JLOG(L"recordset.Open() return %d\n", ret);
	DWORD count = recordset.GetRecordCount();
	JLOG(L"recordset.GetRecordCount() return %d\n", count);
	std::list<int> unresolvedDeviceIdList;
	if (count > 0) {
		recordset.MoveFirst();
		for (DWORD i = 0; i < count; i++) {
			DEFINE_AND_GET_FIELD_VALUE_INTEGER(id);
			DEFINE_AND_GET_FIELD_VALUE_CSTRING(cameraId);
			DEFINE_AND_GET_FIELD_VALUE_CSTRING(cameraName);
			DEFINE_AND_GET_FIELD_VALUE_INTEGER(cameraNo);
			DEFINE_AND_GET_FIELD_VALUE_INTEGER(defence);
			DEFINE_AND_GET_FIELD_VALUE_CSTRING(deviceId);
			DEFINE_AND_GET_FIELD_VALUE_CSTRING(deviceName);
			DEFINE_AND_GET_FIELD_VALUE_CSTRING(deviceSerial);
			DEFINE_AND_GET_FIELD_VALUE_INTEGER(isEncrypt);
			DEFINE_AND_GET_FIELD_VALUE_CSTRING(isShared);
			DEFINE_AND_GET_FIELD_VALUE_CSTRING(picUrl);
			DEFINE_AND_GET_FIELD_VALUE_INTEGER(status);
			DEFINE_AND_GET_FIELD_VALUE_CSTRING(secure_code);
			DEFINE_AND_GET_FIELD_VALUE_CSTRING(device_note);
			DEFINE_AND_GET_FIELD_VALUE_INTEGER(user_info_id);
			//DEFINE_AND_GET_FIELD_VALUE_INTEGER(detector_info_id);
			recordset.MoveNext();

			ezviz::CVideoDeviceInfoEzvizPtr deviceInfo = std::make_shared<ezviz::CVideoDeviceInfoEzviz>();
			SET_DEVICE_INFO_DATA_MEMBER_INTEGER(id);
			SET_DEVICE_INFO_DATA_MEMBER_STRING(cameraId);
			SET_DEVICE_INFO_DATA_MEMBER_WCSTRING(cameraName);
			SET_DEVICE_INFO_DATA_MEMBER_INTEGER(cameraNo);
			SET_DEVICE_INFO_DATA_MEMBER_INTEGER(defence);
			SET_DEVICE_INFO_DATA_MEMBER_STRING(deviceId);
			SET_DEVICE_INFO_DATA_MEMBER_STRING(deviceName);
			SET_DEVICE_INFO_DATA_MEMBER_STRING(deviceSerial);
			SET_DEVICE_INFO_DATA_MEMBER_INTEGER(isEncrypt);
			SET_DEVICE_INFO_DATA_MEMBER_STRING(isShared);
			SET_DEVICE_INFO_DATA_MEMBER_STRING(picUrl);
			SET_DEVICE_INFO_DATA_MEMBER_INTEGER(status);
			SET_DEVICE_INFO_DATA_MEMBER_STRING(secure_code);
			SET_DEVICE_INFO_DATA_MEMBER_WCSTRING(device_note);
			//SET_DEVICE_INFO_DATA_MEMBER_INTEGER(detector_info_id);
			//if (ezviz::CSdkMgrEzviz::GetInstance()->VerifyDeviceInfo(userInfo, deviceInfo)) {
			//	
			//} else {
			//	//unresolvedDeviceIdList.push_back(id);
			//}
			deviceInfo->set_userInfo(userInfo);
			userInfo->AddDevice(deviceInfo);
			_deviceList.push_back(deviceInfo);
			_ezvizDeviceList.push_back(deviceInfo);
		}
	}
	recordset.Close();

	// resolve illegal device info in db
	for(auto id : unresolvedDeviceIdList) {
		query.Format(L"delete from device_info_ezviz where id=%d", id);
		m_db->Execute(query);
	}

	return count;
}


void CVideoManager::LoadUserInfoEzvizFromDB()
{
	AUTO_LOG_FUNCTION;
	USES_CONVERSION;
	CString query;
	query.Format(L"select id,user_phone,user_name,user_accToken,tokenTime from user_info where productor_info_id=%d order by id",
				 video::EZVIZ);
	ado::CADORecordset recordset(m_db->GetDatabase());
	JLOG(L"CADORecordset recordset %p\n", &recordset);
	BOOL ret = recordset.Open(m_db->GetDatabase()->m_pConnection, query);
	VERIFY(ret); JLOG(L"recordset.Open() return %d\n", ret);
	DWORD count = recordset.GetRecordCount();
	JLOG(L"recordset.GetRecordCount() return %d\n", count);
	//bool ok = false;
	if (count > 0)
		recordset.MoveFirst();
	for (DWORD i = 0; i < count; i++) {
		DEFINE_AND_GET_FIELD_VALUE_INTEGER(id);
		DEFINE_AND_GET_FIELD_VALUE_CSTRING(user_name);
		DEFINE_AND_GET_FIELD_VALUE_CSTRING(user_phone);
		DEFINE_AND_GET_FIELD_VALUE_CSTRING(user_accToken);
		COleDateTime tokenTime;
		recordset.GetFieldValue(L"tokenTime", tokenTime);
		recordset.MoveNext();
		//DEFINE_AND_GET_FIELD_VALUE_CSTRING(user_acct);
		//DEFINE_AND_GET_FIELD_VALUE_CSTRING(user_passwd);

		ezviz::CVideoUserInfoEzvizPtr userInfo = std::make_shared<ezviz::CVideoUserInfoEzviz>();
		SET_USER_INFO_DATA_MEMBER_INTEGER(id);
		SET_USER_INFO_DATA_MEMBER_WSTRING(user_name);
		SET_USER_INFO_DATA_MEMBER_STRING(user_phone);
		SET_USER_INFO_DATA_MEMBER_STRING(user_accToken);
		userInfo->set_productorInfo(ProductorEzviz);
		if (tokenTime.GetStatus() == COleDateTime::invalid) {
			assert(0);
			tokenTime = COleDateTime::GetCurrentTime();
		}
		userInfo->set_user_tokenTime(tokenTime);

		if (LoadDeviceInfoEzvizFromDB(userInfo) == 0) {
			
		}
		RefreshUserEzvizDeviceList(userInfo);
		_userList.push_back(userInfo);
		//ok = true;
	}
	recordset.Close();

	// resolve dev list from ezviz cloud
	CheckUserAcctkenTimeout();

	return;
}


void CVideoManager::LoadEzvizPrivateCloudInfoFromDB()
{
	AUTO_LOG_FUNCTION;
	//USES_CONVERSION;
	//CString query;
	//query.Format(L"select * from private_cloud_info");
	//ado::CADORecordset recordset(m_db->GetDatabase());
	//JLOG(L"CADORecordset recordset %p\n", &recordset);
	//BOOL ret = recordset.Open(m_db->GetDatabase()->m_pConnection, query);
	//VERIFY(ret); JLOG(L"recordset.Open() return %d\n", ret);
	//DWORD count = recordset.GetRecordCount();
	//JLOG(L"recordset.GetRecordCount() return %d\n", count);
	////bool ok = false;
	//if (count == 1) {
	//	recordset.MoveFirst();
	//	//DEFINE_AND_GET_FIELD_VALUE_CSTRING(private_cloud_ip);
	//	//DEFINE_AND_GET_FIELD_VALUE_INTEGER(private_cloud_port);
	//	DEFINE_AND_GET_FIELD_VALUE_CSTRING(private_cloud_app_key);
	//	//ok = true;
	//	ezviz::CPrivateCloudConnector* connector = ezviz::CPrivateCloudConnector::GetInstance();
	//	auto cfg = util::CConfigHelper::GetInstance();
	//	connector->set_ip(cfg->get_ezviz_private_cloud_ip());
	//	connector->set_port(cfg->get_ezviz_private_cloud_port());
	//	//connector->set_ip(W2A(private_cloud_ip));
	//	//connector->set_port(private_cloud_port);
	//	connector->set_appKey(W2A(private_cloud_app_key));
	//	ezviz::CSdkMgrEzviz::GetInstance()->Init(connector->get_appKey());
	//}
	//recordset.Close();
	////return ok;

	auto cfg = util::CConfigHelper::GetInstance();
	if (!ezviz::CSdkMgrEzviz::GetInstance()->Init(cfg->get_ezviz_private_cloud_app_key())) {
		AfxMessageBox(IDS_STRING_INIT_EZVIZ_SDK_ERROR, MB_ICONEXCLAMATION);
		QuitApplication(0);
	}
}


void CVideoManager::LoadBindInfoFromDB()
{
	AUTO_LOG_FUNCTION;
	USES_CONVERSION;
	CString query;
	query.Format(L"select * from bind_info order by ID");
	ado::CADORecordset recordset(m_db->GetDatabase());
	JLOG(L"CADORecordset recordset %p\n", &recordset);
	BOOL ret = recordset.Open(m_db->GetDatabase()->m_pConnection, query);
	VERIFY(ret); JLOG(L"recordset.Open() return %d\n", ret);
	DWORD count = recordset.GetRecordCount();
	JLOG(L"recordset.GetRecordCount() return %d\n", count);
	//bool ok = false;
	if (count > 0)
		recordset.MoveFirst();
	for (DWORD i = 0; i < count; i++) {
		DEFINE_AND_GET_FIELD_VALUE_INTEGER(id);
		DEFINE_AND_GET_FIELD_VALUE_INTEGER(ademco_id);
		DEFINE_AND_GET_FIELD_VALUE_INTEGER(zone_value);
		DEFINE_AND_GET_FIELD_VALUE_INTEGER(gg_value);
		DEFINE_AND_GET_FIELD_VALUE_INTEGER(device_info_id);
		DEFINE_AND_GET_FIELD_VALUE_INTEGER(productor_info_id);
		DEFINE_AND_GET_FIELD_VALUE_INTEGER(auto_play_video);
		recordset.MoveNext();

		ZoneUuid zoneUuid(ademco_id, zone_value, gg_value);
		CVideoDeviceInfoPtr device = nullptr;
		if (GetVideoDeviceInfo(device_info_id, GetProductorInfo(productor_info_id).get_productor(), device) && device) {
			device->add_zoneUuid(zoneUuid);
			BindInfo bindInfo(id, device, 1);
			_bindMap[zoneUuid] = bindInfo;
		}

	}
	recordset.Close();

	// resolve dev list from ezviz cloud


	return;
}


BindInfo CVideoManager::GetBindInfo(const ZoneUuid& zone)
{
	std::lock_guard<std::mutex> lock(_bindMapLock);
	BindInfo bi(-1, nullptr, 0);
	auto i = _bindMap.find(zone);
	if (i != _bindMap.end()) {
		bi = i->second;
	}
	return bi;
}


void CVideoManager::GetVideoUserList(CVideoUserInfoList& list)
{
	std::copy(_userList.begin(), _userList.end(), std::back_inserter(list));
}


void CVideoManager::GetVideoDeviceList(CVideoDeviceInfoList& list)
{
	std::copy(_deviceList.begin(), _deviceList.end(), std::back_inserter(list));
}


void CVideoManager::GetVideoDeviceEzvizWithDetectorList(ezviz::CVideoDeviceInfoEzvizList& list)
{
	for (auto dev : _deviceList) {
		if (dev->get_userInfo()->get_productorInfo().get_productor() == EZVIZ) {
			list.push_back(std::dynamic_pointer_cast<video::ezviz::CVideoDeviceInfoEzviz>(dev));
		}
	}
}


ezviz::CVideoDeviceInfoEzvizPtr CVideoManager::GetVideoDeviceInfoEzviz(int id)
{
	ezviz::CVideoDeviceInfoEzvizPtr res;
	for (auto dev : _deviceList) {
		if (dev->get_id() == id) {
			res = std::dynamic_pointer_cast<ezviz::CVideoDeviceInfoEzviz>(dev);
			break;
		}
	}
	return res;
}


bool CVideoManager::GetVideoDeviceInfo(int id, PRODUCTOR productor, CVideoDeviceInfoPtr& device)
{
	for (auto dev : _deviceList) {
		if (dev->get_id() == id && dev->get_userInfo()->get_productorInfo().get_productor() == productor) {
			device = dev;
			return true;
		}
	}
	return false;
}


bool CVideoManager::DeleteVideoUser(ezviz::CVideoUserInfoEzvizPtr userInfo)
{
	std::lock_guard<std::mutex> lock(_userListLock);
	assert(userInfo);
	CVideoDeviceInfoList list;
	userInfo->GetDeviceList(list);
	for (auto dev : list) {
		ezviz::CVideoDeviceInfoEzvizPtr device = std::dynamic_pointer_cast<ezviz::CVideoDeviceInfoEzviz>(dev);
		userInfo->DeleteVideoDevice(device);
		_deviceList.remove(device);
		_ezvizDeviceList.remove(device);
	}
	if (_ezvizDeviceList.size() == 0) {
		Execute(L"alter table device_info_ezviz alter column id counter(1,1)");
	}

	CString sql;
	sql.Format(L"delete from user_info where ID=%d", userInfo->get_id());
	if (Execute(sql)) {
		ezviz::CSdkMgrEzviz::GetInstance()->FreeUserSession(userInfo->get_user_phone());
		_userList.remove(userInfo);
		if (_userList.size() == 0)
			Execute(L"alter table user_info alter column id counter(1,1)");
		return true;
	}
	return false;
}


bool CVideoManager::BindZoneAndDevice(const ZoneUuid& zoneUuid, ezviz::CVideoDeviceInfoEzvizPtr device)
{
	std::lock_guard<std::mutex> lock(_bindMapLock);
	bool ok = true;
	do {
		assert(device);
		if (/*device->get_binded() || */_bindMap.find(zoneUuid) != _bindMap.end()) {
			ok = false; break;
		}

		CString sql;
		sql.Format(L"insert into bind_info([ademco_id],[zone_value],[gg_value],[device_info_id],[productor_info_id],[auto_play_video]) values(%d,%d,%d,%d,%d,%d)",
				   zoneUuid._ademco_id, zoneUuid._zone_value, zoneUuid._gg,
				   device->get_id(), device->get_userInfo()->get_productorInfo().get_productor(), 1);
		int id = AddAutoIndexTableReturnID(sql);
		if (id == -1) {
			ok = false; break;
		}

		device->add_zoneUuid(zoneUuid);
		BindInfo bi(id, device, 1);
		_bindMap[zoneUuid] = bi;
		ok = true;
	} while (0);

	return ok;
}


bool CVideoManager::UnbindZoneAndDevice(const ZoneUuid& zoneUuid)
{
	std::lock_guard<std::mutex> lock(_bindMapLock);
	bool ok = false;
	do {
		auto iter = _bindMap.find(zoneUuid);
		if (iter == _bindMap.end()) { ok = true; break; }

		BindInfo bi = iter->second;
		CVideoDeviceInfoPtr dev = bi._device;
		if (!dev) {
			_bindMap.erase(iter);
			if (_bindMap.size() == 0) {
				Execute(L"alter table bind_info alter column id counter(1,1)");
			}
			ok = true; break;
		}

		/*if (!(dev->get_zoneUuid() == zoneUuid)) {
			_bindMap.erase(iter);
			if (_bindMap.size() == 0) {
				Execute(L"alter table bind_info alter column id counter(1,1)");
			}
			ok = true; break;
		}*/

		CVideoUserInfoPtr usr = dev->get_userInfo();
		assert(usr);

		if (usr->get_productorInfo().get_productor() == EZVIZ) {
			ezviz::CVideoDeviceInfoEzvizPtr device = std::dynamic_pointer_cast<ezviz::CVideoDeviceInfoEzviz>(dev);
			CString sql;
			sql.Format(L"delete from bind_info where ID=%d", bi._id);
			if (Execute(sql)) {
				//device->set_binded(false);
				device->del_zoneUuid(zoneUuid);
				_bindMap.erase(zoneUuid);
				if (_bindMap.size() == 0) {
					Execute(L"alter table bind_info alter column id counter(1,1)");
				}
				ok = true; break;
			}
		} else if (usr->get_productorInfo().get_productor() == NORMAL) {
			// TODO: 2015-9-1015:18:43 video::NORMAL

		}

		ok = false;
	} while (0);

	return ok;
}


bool CVideoManager::CheckIfUserEzvizPhoneExists(const std::string& user_phone)
{
	for (auto i : _userList) {
		if (i->get_productorInfo().get_productor() == EZVIZ) {
			ezviz::CVideoUserInfoEzvizPtr user = std::dynamic_pointer_cast<ezviz::CVideoUserInfoEzviz>(i);
			if (user->get_user_phone().compare(user_phone) == 0) {
				return true;
			}
		}
	}
	return false;
}


CVideoManager::VideoEzvizResult CVideoManager::AddVideoUserEzviz(ezviz::CVideoUserInfoEzvizPtr user)
{
	AUTO_LOG_FUNCTION;
	USES_CONVERSION;
	std::lock_guard<std::mutex> lock(_userListLock);
	VideoEzvizResult result = RESULT_OK;
	do {
		COleDateTime now = COleDateTime::GetCurrentTime();
		CString sql;
		sql.Format(L"insert into user_info ([user_phone],[user_name],[user_accToken],[productor_info_id],[tokenTime]) values('%s','%s','%s',%d,'%s')",
				   A2W(user->get_user_phone().c_str()), user->get_user_name().c_str(), A2W(user->get_user_accToken().c_str()), 
				   EZVIZ, now.Format(L"%Y-%m-%d %H:%M:%S"));
		int id = AddAutoIndexTableReturnID(sql);
		if (id == -1) {
			result = RESULT_INSERT_TO_DB_FAILED; break;
		}
		user->set_id(id);
		user->set_productorInfo(ProductorEzviz);
		user->set_user_tokenTime(now);
		_userList.push_back(user);

		RefreshUserEzvizDeviceList(user);

	} while (0);

	return result;
}


ezviz::CVideoUserInfoEzvizPtr CVideoManager::GetVideoUserEzviz(int id)
{
	ezviz::CVideoUserInfoEzvizPtr res;
	std::lock_guard<std::mutex> lock(_userListLock);
	for (auto user : _userList) {
		if (user->get_id() == id) {
			res = std::dynamic_pointer_cast<ezviz::CVideoUserInfoEzviz>(user);
			break;
		}
	}
	return res;
}


CVideoManager::VideoEzvizResult CVideoManager::RefreshUserEzvizDeviceList(ezviz::CVideoUserInfoEzvizPtr user)
{
	ezviz::CVideoDeviceInfoEzvizList list;
	if (ezviz::CSdkMgrEzviz::GetInstance()->GetUsersDeviceList(user, list) && list.size() > 0) {
		CVideoDeviceInfoList localList;
		user->GetDeviceList(localList);
		std::list<int> outstandingDevIdList;
		
		for (auto& localDev : localList) {
			ezviz::CVideoDeviceInfoEzvizPtr ezvizDevice = std::dynamic_pointer_cast<ezviz::CVideoDeviceInfoEzviz>(localDev);
			bool exsist = false;
			for (auto dev : list) {
				if (ezvizDevice->get_deviceId().compare(dev->get_deviceId()) == 0) {
					
					// 2016-5-11 15:26:08 might need to update camera id
					if (ezvizDevice->get_cameraId() != dev->get_cameraId()) {
						ezvizDevice->set_cameraId(dev->get_cameraId());
						ezvizDevice->execute_update_info();
					}

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
		//		ezviz::CVideoDeviceInfoEzvizPtr ezvizDevice = std::dynamic_pointer_cast<ezviz::CVideoDeviceInfoEzviz>(localDev);
		//		if (ezvizDevice->get_id() == id) {
		//			_deviceList.remove(localDev);
		//			_ezvizDeviceList.remove(ezvizDevice);
		//			
		//			std::list<ZoneUuid> zoneList;
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


bool CVideoManager::SetBindInfoAutoPlayVideoOnAlarm(const ZoneUuid& zone, int auto_play_video)
{
	std::lock_guard<std::mutex> lock(_bindMapLock);
	bool ok = true;
	do {
		auto&& iter = _bindMap.find(zone);
		if (iter == _bindMap.end()) { ok = false; break; }
		CString sql;
		sql.Format(L"update bind_info set auto_play_video=%d where ID=%d", auto_play_video, iter->second._id);
		if (Execute(sql)) {
			iter->second._auto_play_video = auto_play_video;
			 ok = true; break; 
		}

		ok = false;
	} while (0);
	return ok;
}


void CVideoManager::CheckUserAcctkenTimeout()
{
	std::lock_guard<std::mutex> lock(_userListLock);
	for (auto user : _userList) {
		if (user->get_productorInfo().get_productor() == EZVIZ) {
			ezviz::CVideoUserInfoEzvizPtr userEzviz = std::dynamic_pointer_cast<ezviz::CVideoUserInfoEzviz>(user);
			COleDateTime now = COleDateTime::GetCurrentTime();
			COleDateTimeSpan span = now - userEzviz->get_user_tokenTime();
#if 0
			if (span.GetTotalDays() > 1) {
				JLOG(L"CVideoManager::CheckUserAcctkenTimeout(), old %s, now %s, %d days has passed, the user %s's accToken should be re-get\n", 
					userEzviz->get_user_tokenTime().Format(L"%Y-%m-%d %H:%M:%S"), 
					now.Format(L"%Y-%m-%d %H:%M:%S"), 1, userEzviz->get_user_name().c_str());
#else
			if (span.GetTotalDays() > 6) {
				JLOG(L"CVideoManager::CheckUserAcctkenTimeout(), old %s, now %s, %d days has passed, the user %s's accToken should be re-get\n",
					userEzviz->get_user_tokenTime().Format(L"%Y-%m-%d %H:%M:%S"),
					now.Format(L"%Y-%m-%d %H:%M:%S"), 6, userEzviz->get_user_name().c_str());
#endif
				
				video::ezviz::CSdkMgrEzviz* mgr = video::ezviz::CSdkMgrEzviz::GetInstance();
				if (video::ezviz::CSdkMgrEzviz::RESULT_OK == mgr->VerifyUserAccessToken(userEzviz, TYPE_GET)) {
					userEzviz->execute_set_user_token_time(COleDateTime::GetCurrentTime());
				}
			}
		}
	}
}




};
