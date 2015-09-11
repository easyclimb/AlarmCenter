#include "stdafx.h"
#include "VideoInfo.h"
#include "DbOper.h"
#include "VideoUserInfoEzviz.h"
#include "VideoUserInfoNormal.h"
#include "VideoDeviceInfoEzviz.h"
#include "VideoDeviceInfoNormal.h"
#include "PrivateCloudConnector.h"
#include "SdkMgrEzviz.h"

#include <iterator>

namespace video {


IMPLEMENT_SINGLETON(CVideoManager)

CVideoManager::CVideoManager()
	: m_db(NULL)
	, _userList()
	, _deviceList()
	, _bindMap()
{
	m_db = new ado::CDbOper();
	m_db->Open(L"video.mdb");
}


CVideoManager::~CVideoManager()
{
	SAFEDELETEP(m_db);
	ezviz::CSdkMgrEzviz::ReleaseObject();
	ezviz::CPrivateCloudConnector::ReleaseObject();

	for (auto &userInfo : _userList) {
		const video::CProductorInfo produtor = userInfo->get_productorInfo();
		if (produtor.get_productor() == video::EZVIZ) {
			video::ezviz::CVideoUserInfoEzviz* ezvizUserInfo = reinterpret_cast<video::ezviz::CVideoUserInfoEzviz*>(userInfo);
			SAFEDELETEP(ezvizUserInfo);
		} else if (produtor.get_productor() == video::NORMAL) {
			video::normal::CVideoUserInfoNormal* normalUserInfo = reinterpret_cast<video::normal::CVideoUserInfoNormal*>(userInfo);
			SAFEDELETEP(normalUserInfo);
		}
	}
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
	//LoadDeviceInfoEzvizFromDB();
	LoadEzvizPrivateCloudInfoFromDB();
	LoadUserInfoEzvizFromDB();
}


int CVideoManager::LoadDeviceInfoEzvizFromDB(ezviz::CVideoUserInfoEzviz* userInfo)
{
	AUTO_LOG_FUNCTION;
	USES_CONVERSION;
	CString query;
	query.Format(L"select * from device_info_ezviz where user_info_id=%d order by ID",
					userInfo->get_id());
	ado::CADORecordset recordset(m_db->GetDatabase());
	LOG(L"CADORecordset recordset %p\n", &recordset);
	BOOL ret = recordset.Open(m_db->GetDatabase()->m_pConnection, query);
	VERIFY(ret); LOG(L"recordset.Open() return %d\n", ret);
	DWORD count = recordset.GetRecordCount();
	LOG(L"recordset.GetRecordCount() return %d\n", count);
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
			recordset.MoveNext();

			ezviz::CVideoDeviceInfoEzviz* deviceInfo = new ezviz::CVideoDeviceInfoEzviz();
			SET_DEVICE_INFO_DATA_MEMBER_INTEGER(id);
			SET_DEVICE_INFO_DATA_MEMBER_STRING(cameraId);
			SET_DEVICE_INFO_DATA_MEMBER_STRING(cameraName);
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

			if (ezviz::CSdkMgrEzviz::GetInstance()->VerifyDeviceInfo(userInfo, deviceInfo)) {
				
			} else {
				//unresolvedDeviceIdList.push_back(id);
			}
			deviceInfo->set_userInfo(userInfo);
			userInfo->AddDevice(deviceInfo);
			_deviceList.push_back(deviceInfo);
		}
	}
	recordset.Close();

	// resolve illegal device info in db
	std::list<int>::iterator iter = unresolvedDeviceIdList.begin();
	while (iter != unresolvedDeviceIdList.end()) {
		int theId = *iter++;
		query.Format(L"delete from device_info_ezviz where id=%d", theId);
		m_db->Execute(query);
	}

	return count;
}


void CVideoManager::LoadUserInfoEzvizFromDB()
{
	AUTO_LOG_FUNCTION;
	USES_CONVERSION;
	CString query;
	query.Format(L"select id,user_phone,user_name,user_accToken from user_info where productor_info_id=%d order by id",
					video::EZVIZ);
	ado::CADORecordset recordset(m_db->GetDatabase());
	LOG(L"CADORecordset recordset %p\n", &recordset);
	BOOL ret = recordset.Open(m_db->GetDatabase()->m_pConnection, query);
	VERIFY(ret); LOG(L"recordset.Open() return %d\n", ret);
	DWORD count = recordset.GetRecordCount();
	LOG(L"recordset.GetRecordCount() return %d\n", count);
	//bool ok = false;
	for (DWORD i = 0; i < count; i++) {
		recordset.MoveFirst();
		DEFINE_AND_GET_FIELD_VALUE_INTEGER(id);
		DEFINE_AND_GET_FIELD_VALUE_CSTRING(user_name);
		DEFINE_AND_GET_FIELD_VALUE_CSTRING(user_phone);
		DEFINE_AND_GET_FIELD_VALUE_CSTRING(user_accToken);
		recordset.MoveNext();
		//DEFINE_AND_GET_FIELD_VALUE_CSTRING(user_acct);
		//DEFINE_AND_GET_FIELD_VALUE_CSTRING(user_passwd);

		ezviz::CVideoUserInfoEzviz* userInfo = new ezviz::CVideoUserInfoEzviz();
		SET_USER_INFO_DATA_MEMBER_INTEGER(id);
		SET_USER_INFO_DATA_MEMBER_WSTRING(user_name);
		SET_USER_INFO_DATA_MEMBER_STRING(user_phone);
		SET_USER_INFO_DATA_MEMBER_STRING(user_accToken);

		int count = LoadDeviceInfoEzvizFromDB(userInfo);
		if (count == 0) {
			// no device loaded, get device list from ezviz cloud.
			ezviz::CVideoDeviceInfoEzvizList list;
			if (ezviz::CSdkMgrEzviz::GetInstance()->GetUsersDeviceList(userInfo, list) && list.size() > 0) {
				/*ezviz::CVideoDeviceInfoEzvizListIter iter = list.begin();
				while (iter != list.end()) {
					ezviz::CVideoDeviceInfoEzviz* device = *iter++;
					userInfo->execute_add_device(device);
				}*/
				for (auto &iter : list) {
					userInfo->execute_add_device(iter);
				}
			}
		}
		_userList.push_back(userInfo);
		//ok = true;
	}
	recordset.Close();

	// resolve dev list from ezviz cloud


	return;
}


void CVideoManager::LoadEzvizPrivateCloudInfoFromDB()
{
	AUTO_LOG_FUNCTION;
	USES_CONVERSION;
	CString query;
	query.Format(L"select * from private_cloud_info");
	ado::CADORecordset recordset(m_db->GetDatabase());
	LOG(L"CADORecordset recordset %p\n", &recordset);
	BOOL ret = recordset.Open(m_db->GetDatabase()->m_pConnection, query);
	VERIFY(ret); LOG(L"recordset.Open() return %d\n", ret);
	DWORD count = recordset.GetRecordCount();
	LOG(L"recordset.GetRecordCount() return %d\n", count);
	//bool ok = false;
	if (count == 1) {
		recordset.MoveFirst();
		DEFINE_AND_GET_FIELD_VALUE_CSTRING(private_cloud_ip);
		DEFINE_AND_GET_FIELD_VALUE_INTEGER(private_cloud_port);
		DEFINE_AND_GET_FIELD_VALUE_CSTRING(private_cloud_app_key);
		//ok = true;
		ezviz::CPrivateCloudConnector* connector = ezviz::CPrivateCloudConnector::GetInstance();
		connector->set_ip(W2A(private_cloud_ip));
		connector->set_port(private_cloud_port);
		connector->set_appKey(W2A(private_cloud_app_key));
		ezviz::CSdkMgrEzviz::GetInstance()->Init(connector->get_appKey());
	}
	recordset.Close();
	//return ok;
}


BOOL CVideoManager::UpdatePrivateCloudInfo(const std::string& ip, int port)
{
	USES_CONVERSION;
	CString sql; sql.Format(L"update private_cloud_info set private_cloud_ip='%s',private_cloud_port=%d",
							A2W(ip.c_str()), port);
	if (Execute(sql)) {
		ezviz::CPrivateCloudConnector* connector = ezviz::CPrivateCloudConnector::GetInstance();
		connector->set_ip(ip);
		connector->set_port(port);
		return TRUE;
	}
	return FALSE;
}


void CVideoManager::LoadBindInfoFromDB()
{
	AUTO_LOG_FUNCTION;
	USES_CONVERSION;
	CString query;
	query.Format(L"select * from bind_info");
	ado::CADORecordset recordset(m_db->GetDatabase());
	LOG(L"CADORecordset recordset %p\n", &recordset);
	BOOL ret = recordset.Open(m_db->GetDatabase()->m_pConnection, query);
	VERIFY(ret); LOG(L"recordset.Open() return %d\n", ret);
	DWORD count = recordset.GetRecordCount();
	LOG(L"recordset.GetRecordCount() return %d\n", count);
	//bool ok = false;
	for (DWORD i = 0; i < count; i++) {
		recordset.MoveFirst();
		DEFINE_AND_GET_FIELD_VALUE_INTEGER(id);
		DEFINE_AND_GET_FIELD_VALUE_INTEGER(ademco_id);
		DEFINE_AND_GET_FIELD_VALUE_INTEGER(zone_value);
		DEFINE_AND_GET_FIELD_VALUE_INTEGER(gg_value);
		DEFINE_AND_GET_FIELD_VALUE_INTEGER(device_info_id);
		DEFINE_AND_GET_FIELD_VALUE_INTEGER(productor_info_id);
		DEFINE_AND_GET_FIELD_VALUE_INTEGER(auto_play_video);
		recordset.MoveNext();

		ZoneUuid zoneUuid(ademco_id, zone_value, gg_value);
		CVideoDeviceInfo* device = NULL;
		if (GetVideoDeviceInfo(device_info_id, GetProductorInfo(productor_info_id).get_productor(), device) && device) {
			device->set_zoneUuid(zoneUuid);
			DeviceInfo deviceInfo(id, device, 1);
			_bindMap[zoneUuid] = deviceInfo;
		}

	}
	recordset.Close();

	// resolve dev list from ezviz cloud


	return;
}


void CVideoManager::GetVideoUserList(CVideoUserInfoList& list)
{
	std::copy(_userList.begin(), _userList.end(), std::back_inserter(list));
}


void CVideoManager::GetVideoDeviceList(CVideoDeviceInfoList& list)
{
	std::copy(_deviceList.begin(), _deviceList.end(), std::back_inserter(list));
}


bool CVideoManager::GetVideoDeviceInfo(int id, PRODUCTOR productor, CVideoDeviceInfo*& device)
{
	for (auto &i : _deviceList) {
		if (i->get_id() == id && i->get_userInfo()->get_productorInfo().get_productor() == productor) {
			device = i;
			return true;
		}
	}
	return false;
}


bool CVideoManager::DeleteVideoUser(ezviz::CVideoUserInfoEzviz* userInfo)
{
	assert(userInfo);
	CVideoDeviceInfoList list;
	userInfo->GetDeviceList(list);
	for (auto &dev : list) {
		ezviz::CVideoDeviceInfoEzviz* device = reinterpret_cast<ezviz::CVideoDeviceInfoEzviz*>(dev);
		userInfo->DeleteVideoDevice(device);
		_deviceList.remove(device);
	}
	CString sql;
	sql.Format(L"delete from user_info where ID=%d", userInfo->get_id());
	if (Execute(sql)) {
		ezviz::CSdkMgrEzviz::GetInstance()->FreeUserSession(userInfo->get_user_phone());
		_userList.remove(userInfo);
		SAFEDELETEP(userInfo);
		if (_userList.size() == 0)
			Execute(L"alter table user_info alter column id counter(1,1)");
		return true;
	}
	return false;
}


bool CVideoManager::BindZoneAndDevice(ZoneUuid zoneUuid, ezviz::CVideoDeviceInfoEzviz* device)
{
	assert(device);
	if (device->get_binded() || _bindMap.find(zoneUuid) != _bindMap.end()) return false;
	
	CString sql; 
	sql.Format(L"insert into bind_info([ademco_id],[zone_value],[gg_value],[device_info_id],[productor_info_id],[auto_play_video] values(%d,%d,%d,%d,%d,%d)",
			   zoneUuid._ademco_id, zoneUuid._zone_value, zoneUuid._gg,
			   device->get_id(), device->get_userInfo()->get_productorInfo().get_productor(), 1);
	int id = AddAutoIndexTableReturnID(sql);
	if (id == -1) return false;

	device->set_zoneUuid(zoneUuid);
	DeviceInfo di(id, device, 1);
	_bindMap[zoneUuid] = di;
	return true;
}


bool CVideoManager::UnbindZoneAndDevice(ZoneUuid zoneUuid)
{
	auto iter = _bindMap.find(zoneUuid);
	if (iter == _bindMap.end()) return true;

	DeviceInfo di = iter->second;
	CVideoDeviceInfo* dev = di._device;
	if (!dev) {
		_bindMap.erase(iter);
		return true;
	}

	CmpZoneUuid cmp;
	if (!cmp(dev->get_zoneUuid(), zoneUuid)) {
		_bindMap.erase(iter);
		return true;
	}

	CVideoUserInfo* usr = dev->get_userInfo();
	assert(usr);

	if (usr->get_productorInfo().get_productor() == EZVIZ) {
		ezviz::CVideoDeviceInfoEzviz* device = reinterpret_cast<ezviz::CVideoDeviceInfoEzviz*>(dev);
		CString sql;
		sql.Format(L"delete from bind_info where ID=%d", di._id);
		if (Execute(sql)) {
			device->set_binded(false);
			_bindMap.erase(zoneUuid);
			return true;
		}
	} else if (usr->get_productorInfo().get_productor() == NORMAL) {
		// TODO: 2015年9月10日15:18:43
	}
	return false;
}


bool CVideoManager::CheckIfUserEzvizPhoneExists(const std::string& user_phone)
{
	for (auto& i : _userList) {
		if (i->get_productorInfo().get_productor() == EZVIZ) {
			ezviz::CVideoUserInfoEzviz* user = reinterpret_cast<ezviz::CVideoUserInfoEzviz*>(i);
			if (user->get_user_phone().compare(user_phone) == 0) {
				return true;
			}
		}
	}
	return false;
}


bool CVideoManager::AddVideoUserEzviz(const std::wstring& user_name, const std::string& user_phone)
{
	USES_CONVERSION;
	CString sql;
	sql.Format(L"insert into user_info ([user_phone],[user_name],[productor_info_id]) values('%s','%s',%d)",
			   A2W(user_phone.c_str()), user_name.c_str(), EZVIZ);
	int id = AddAutoIndexTableReturnID(sql);
	if (id == -1)return false;

	ezviz::CVideoUserInfoEzviz* user = new ezviz::CVideoUserInfoEzviz();
	user->set_id(id);
	user->set_user_name(user_name);
	user->set_user_phone(user_phone);
	_userList.push_back(user);

	ezviz::CVideoDeviceInfoEzvizList list;
	if (ezviz::CSdkMgrEzviz::GetInstance()->GetUsersDeviceList(user, list) && list.size() > 0) {
		for (auto &iter : list) {
			user->execute_add_device(iter);
		}
	}
	return true;
}



NAMESPACE_END
