#include "stdafx.h"
#include "VideoInfo.h"
#include <odbcinst.h>
#include <afxdb.h>
#include <comdef.h>
#include "ado2.h"
#include "VideoUserInfoEzviz.h"
#include "VideoUserInfoNormal.h"
#include "VideoDeviceInfoEzviz.h"
#include "VideoDeviceInfoNormal.h"
#include "PrivateCloudConnector.h"
#include "SdkMgrEzviz.h"

#include <iterator>

namespace core {
namespace video {


IMPLEMENT_SINGLETON(CVideoManager)

CVideoManager::CVideoManager()
{
	try {
		m_pDatabase = new ado::CADODatabase();
		LOG(_T("CVideoManager after new, m_pDatabase %x"), m_pDatabase);
		LPCTSTR pszMdb = L"video.mdb";
		TCHAR szMdbPath[1024];
		_tcscpy_s(szMdbPath, GetModuleFilePath());
		_tcscat_s(szMdbPath, _T("\\config"));
		CreateDirectory(szMdbPath, NULL);
		_tcscat_s(szMdbPath, _T("\\"));
		_tcscat_s(szMdbPath, pszMdb);
		CLog::WriteLog(_T("CVideoManager before pathexists"));
		if (!CFileOper::PathExists(szMdbPath)) {
			MessageBox(NULL, L"File 'video.mdb' missed or broken!", L"Error", MB_OK | MB_ICONERROR);
			ExitProcess(0);
			return;
		}
		TRACE(_T("after pathexists"));

		CString strConn = _T("");
		strConn.Format(_T("Provider=Microsoft.Jet.OLEDB.4.0; Data Source='%s';Jet OLEDB:Database"), szMdbPath);
		CLog::WriteLog(strConn);
		if (!m_pDatabase->Open(strConn)) {
			TRACE(_T("CVideoManager m_pDatabase->Open() error"));
			MessageBox(NULL, L"File video.mdb missed or broken!", L"Error", MB_OK | MB_ICONERROR);
			ExitProcess(0);
		} else {
			LOG(_T("m_pDatabase->Open() ok"));
			LOG(_T("CVideoManager ConnectDB %s success\n"), strConn);
		}
	} catch (...) {
		AfxMessageBox(_T("connect to access error!"));
		ExitProcess(0);
	}
}


CVideoManager::~CVideoManager()
{
	if (m_pDatabase) {
		if (m_pDatabase->IsOpen()) {
			m_pDatabase->Close();
		}
		delete m_pDatabase;
	}
	ezviz::CSdkMgrEzviz::ReleaseObject();
	ezviz::CPrivateCloudConnector::ReleaseObject();

	core::video::CVideoUserInfoListIter userIter = _userList.begin();
	while (userIter != _userList.end()) {
		core::video::CVideoUserInfo* userInfo = *userIter++;
		const core::video::CProductorInfo produtor = userInfo->get_productorInfo();
		if (produtor.get_productor() == core::video::EZVIZ) {
			core::video::ezviz::CVideoUserInfoEzviz* ezvizUserInfo = reinterpret_cast<core::video::ezviz::CVideoUserInfoEzviz*>(userInfo);
			SAFEDELETEP(ezvizUserInfo);
		} else if (produtor.get_productor() == core::video::NORMAL) {
			core::video::normal::CVideoUserInfoNormal* normalUserInfo = reinterpret_cast<core::video::normal::CVideoUserInfoNormal*>(userInfo);
			SAFEDELETEP(normalUserInfo);
		}
	}


}


void CVideoManager::LoadFromDB()
{
	//LoadDeviceInfoEzvizFromDB();
	LoadEzvizPrivateCloudInfoFromDB();
	LoadUserInfoEzvizFromDB();
}


void CVideoManager::LoadDeviceInfoEzvizFromDB(CVideoUserInfo* userInfo)
{
	AUTO_LOG_FUNCTION;
	USES_CONVERSION;
	CString query;
	query.Format(L"select * from device_info_ezviz where user_info_id=%d order by ID",
				 userInfo->get_id());
	ado::CADORecordset recordset(m_pDatabase);
	LOG(L"CADORecordset recordset %p\n", &recordset);
	BOOL ret = recordset.Open(m_pDatabase->m_pConnection, query);
	VERIFY(ret); LOG(L"recordset.Open() return %d\n", ret);
	DWORD count = recordset.GetRecordCount();
	LOG(L"recordset.GetRecordCount() return %d\n", count);
	//std::list<int> unresolvedDeviceIdList;
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
			DEFINE_AND_GET_FIELD_VALUE_CSTRING(secureCode);
			DEFINE_AND_GET_FIELD_VALUE_CSTRING(cameraNote);
			DEFINE_AND_GET_FIELD_VALUE_INTEGER(productor_info_id);
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
			SET_DEVICE_INFO_DATA_MEMBER_STRING(secureCode);
			SET_DEVICE_INFO_DATA_MEMBER_WCSTRING(cameraNote);
			deviceInfo->set_userInfo(userInfo);

			userInfo->AddDevice(deviceInfo);
			_deviceList.push_back(deviceInfo);
		}
	}
	recordset.Close();
}


void CVideoManager::LoadUserInfoEzvizFromDB()
{
	AUTO_LOG_FUNCTION;
	USES_CONVERSION;
	CString query;
	query.Format(L"select id,user_phone,user_name,user_accToken from user_info where productor_info_id=%d order by id",
				 core::video::EZVIZ);
	ado::CADORecordset recordset(m_pDatabase);
	LOG(L"CADORecordset recordset %p\n", &recordset);
	BOOL ret = recordset.Open(m_pDatabase->m_pConnection, query);
	VERIFY(ret); LOG(L"recordset.Open() return %d\n", ret);
	DWORD count = recordset.GetRecordCount();
	LOG(L"recordset.GetRecordCount() return %d\n", count);
	//bool ok = false;
	for (DWORD i = 0; i < count; i++){
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

		LoadDeviceInfoEzvizFromDB(userInfo);
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
	ado::CADORecordset recordset(m_pDatabase);
	LOG(L"CADORecordset recordset %p\n", &recordset);
	BOOL ret = recordset.Open(m_pDatabase->m_pConnection, query);
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


void CVideoManager::LoadBindInfoFromDB()
{

}


void CVideoManager::GetVideoUserList(CVideoUserInfoList& list)
{
	std::copy(_userList.begin(), _userList.end(), std::back_inserter(list));
}


void CVideoManager::GetVideoDeviceList(CVideoDeviceInfoList& list)
{
	std::copy(_deviceList.begin(), _deviceList.end(), std::back_inserter(list));
}


NAMESPACE_END
NAMESPACE_END
