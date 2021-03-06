#include "stdafx.h"
#include "SdkMgrEzviz.h"
#include "../video/ezviz/VideoUserInfoEzviz.h"
#include "../video/ezviz/VideoDeviceInfoEzviz.h"
#include "../contrib/json/json.h"
#include "PrivateCloudConnector.h"
#include "ConfigHelper.h"
#include "AlarmCenterVideo.h"
#include "VideoManager.h"

namespace video {
namespace ezviz {


//IMPLEMENT_SINGLETON(sdk_mgr_ezviz)
sdk_mgr_ezviz::sdk_mgr_ezviz()
	: _sessionMap()
	//, m_dll()
{

}


sdk_mgr_ezviz::~sdk_mgr_ezviz()
{
	AUTO_LOG_FUNCTION;
	for (auto iter : _sessionMap) {
		for (auto session : iter.second) {
			m_dll.freeSession(session.second);
		}
	}
	_sessionMap.clear();
	m_dll.releaseLibrary();
}


bool sdk_mgr_ezviz::InitLibrary()
{
	return m_dll.InitLibrary();
}


#pragma region CSdkMgrEzvizPrivate
sdk_mgr_ezviz::CSdkMgrEzvizPrivate::CSdkMgrEzvizPrivate()
{

}


bool sdk_mgr_ezviz::CSdkMgrEzvizPrivate::InitLibrary()
{
	AUTO_LOG_FUNCTION;
	CString path; path.Format(L"%s\\contrib\\ezviz", get_exe_path().c_str());
	SetCurrentDirectory(path);
	path += L"\\OpenNetStream.dll";
	// 2015-11-11 20:53:34 sometimes it will failed, so retry some times.
	for (int i = 0; i < 1; i++) {
		m_library = LoadLibrary(path);
		if (m_library == nullptr) {
			JLOG(L"load %s falied #%d, err: %d\n", path, i, GetLastError());
		} else {
			break;
		}
	}
	if (m_library == nullptr) {
		JLOG(L"load %s falied, err: %d\n", path, GetLastError());
		ExitProcess(9958);
		return false;
	}
	SetCurrentDirectory(get_exe_path().c_str());

	bool ok = false;
#define GET_PROC(proc_name) m_apis.p##proc_name = (proc_name)GetProcAddress(m_library, #proc_name); if(m_apis.p##proc_name == nullptr){JLOGA("get proc addr faild: %s", #proc_name); break;}
	do {
		GET_PROC(OpenSDK_InitLib);
		GET_PROC(OpenSDK_FiniLib);
		GET_PROC(OpenSDK_AllocSession);
		GET_PROC(OpenSDK_FreeSession);
		GET_PROC(OpenSDK_StartRealPlay);
		GET_PROC(OpenSDK_StopRealPlay);
		GET_PROC(OpenSDK_StartPlayBack);
		GET_PROC(OpenSDK_PlayBackResume);
		GET_PROC(OpenSDK_PlayBackPause);
		GET_PROC(OpenSDK_StopPlayBack);
		GET_PROC(OpenSDK_SetDataCallBack);
		GET_PROC(OpenSDK_StartSearch);
		GET_PROC(OpenSDK_GetOSDTime);
		GET_PROC(OpenSDK_OpenSound);
		GET_PROC(OpenSDK_CloseSound);
		GET_PROC(OpenSDK_GetVolume);
		GET_PROC(OpenSDK_SetVolume);
		GET_PROC(OpenSDK_StartVoiceTalk);
		GET_PROC(OpenSDK_StopVoiceTalk);
		GET_PROC(OpenSDK_CapturePicture);
		GET_PROC(OpenSDK_Mid_Login);
		GET_PROC(OpenSDK_Mid_Device_Add);
		GET_PROC(OpenSDK_Mid_Device_Oper);
		GET_PROC(OpenSDK_Data_GetDevList);
		GET_PROC(OpenSDK_Data_GetDeviceInfo);
		GET_PROC(OpenSDK_Data_GetAlarmList);
		GET_PROC(OpenSDK_Data_SetAlarmRead);
		GET_PROC(OpenSDK_Data_DeleteDevice);
		GET_PROC(OpenSDK_Data_Free);
		GET_PROC(OpenSDK_Alarm_SetMsgCallBack);
		GET_PROC(OpenSDK_Alarm_StartRecv);
		GET_PROC(OpenSDK_Alarm_StopRecv);
		GET_PROC(OpenSDK_PTZCtrl);
		GET_PROC(OpenSDK_DevDefence);
		GET_PROC(OpenSDK_DevDefenceByDev);
		GET_PROC(OpenSDK_GetAccessTokenSmsCode);
		GET_PROC(OpenSDK_VerifyAccessTokenSmsCode);
		GET_PROC(OpenSDK_GetHdSignSmsCode);
		GET_PROC(OpenSDK_VerifyHdSignSmsCode);
		GET_PROC(OpenSDK_UpdateCameraInfoToLocal);
		GET_PROC(OpenSDK_RequestPassThrough);
		/*GET_PROC(OpenSDK_InitLib);
		GET_PROC(OpenSDK_InitLib);
		GET_PROC(OpenSDK_InitLib);*/
		ok = true;
	} while (0);

	if (!ok) {
		ExitProcess(9958);
		return false;
	}
	return true;
}


sdk_mgr_ezviz::CSdkMgrEzvizPrivate::~CSdkMgrEzvizPrivate()
{
	AUTO_LOG_FUNCTION;
	if (m_library) {
		releaseLibrary();
		FreeLibrary(m_library);
		m_library = nullptr;
	}
}


int sdk_mgr_ezviz::CSdkMgrEzvizPrivate::initLibrary(const std::string& authAddr, const std::string& platform, const std::string& appKey)
{
	return m_apis.pOpenSDK_InitLib(authAddr.c_str(), platform.c_str(), appKey.c_str());
}

int sdk_mgr_ezviz::CSdkMgrEzvizPrivate::releaseLibrary()
{
	return m_apis.pOpenSDK_FiniLib();
}

std::string sdk_mgr_ezviz::CSdkMgrEzvizPrivate::oauth_login()
{
	char* pToken = nullptr;
	int length = 0;
	m_apis.pOpenSDK_Mid_Login(&pToken, &length);
	JLOGA("OpenSDK_Mid_Login token: %s\n", pToken);
	std::string token = pToken;
	return token;
}

int sdk_mgr_ezviz::CSdkMgrEzvizPrivate::deviceAdd(const std::string& accessId)
{
	return m_apis.pOpenSDK_Mid_Device_Add(accessId.c_str());
}

int sdk_mgr_ezviz::CSdkMgrEzvizPrivate::deviceOper(const std::string& accessId, const std::string& deviceId)
{
	return m_apis.pOpenSDK_Mid_Device_Oper(accessId.c_str(), deviceId.c_str());
}

std::string sdk_mgr_ezviz::CSdkMgrEzvizPrivate::allocSession(OpenSDK_MessageHandler handle, void* pUser, const bool bSync, const unsigned int iTimeOut)
{
	std::string session;
	char* pSession = nullptr;
	int length = 0;
	m_apis.pOpenSDK_AllocSession(handle, pUser, &pSession, &length, bSync, iTimeOut);
	if (pSession) {
		session = pSession;
		freeData(pSession);
	}

	return session;
}

int sdk_mgr_ezviz::CSdkMgrEzvizPrivate::freeSession(const std::string& session)
{
	return m_apis.pOpenSDK_FreeSession(session.c_str());
}

int sdk_mgr_ezviz::CSdkMgrEzvizPrivate::startRealPlay(const std::string& session, HWND hPlayWnd, const std::string& cameraId, const std::string& accessId, const std::string& safekey, const std::string& appKey, int videoLevel, LP_NSCBMsg pNSCBMsg)
{
	return m_apis.pOpenSDK_StartRealPlay(session.c_str(), hPlayWnd,
										 cameraId.c_str(), accessId.c_str(),
										 videoLevel, safekey.c_str(), appKey.c_str(), pNSCBMsg);
}

int sdk_mgr_ezviz::CSdkMgrEzvizPrivate::stopRealPlay(const std::string& session, LP_NSCBMsg pNSCBMsg)
{
	return m_apis.pOpenSDK_StopRealPlay(session.c_str(), pNSCBMsg);
}

int sdk_mgr_ezviz::CSdkMgrEzvizPrivate::startPlayBack(const std::string& session, HWND hPlayWnd, const std::string& cameraId, const std::string& accessId, const std::string& safekey, const std::string& startTime, const std::string& stopTime, const std::string& appKey, LP_NSCBMsg pNSCBMsg)
{
	return m_apis.pOpenSDK_StartPlayBack(session.c_str(), hPlayWnd,
										 cameraId.c_str(), accessId.c_str(),
										 safekey.c_str(), startTime.c_str(),
										 stopTime.c_str(), appKey.c_str(), pNSCBMsg);
}

int sdk_mgr_ezviz::CSdkMgrEzvizPrivate::playBackResume(const std::string& session)
{
	return m_apis.pOpenSDK_PlayBackResume(session.c_str());
}

int sdk_mgr_ezviz::CSdkMgrEzvizPrivate::playBackPause(const std::string& session)
{
	return m_apis.pOpenSDK_PlayBackPause(session.c_str());
}

int sdk_mgr_ezviz::CSdkMgrEzvizPrivate::stopPlayBack(const std::string& session, LP_NSCBMsg pNSCBMsg)
{
	return m_apis.pOpenSDK_StopPlayBack(session.c_str(), pNSCBMsg);
}

int sdk_mgr_ezviz::CSdkMgrEzvizPrivate::setDataCallBack(const std::string& szSessionId, OpenSDK_DataCallBack pDataCallBack, void* pUser)
{
	return m_apis.pOpenSDK_SetDataCallBack(szSessionId.c_str(), pDataCallBack, pUser);
}

int sdk_mgr_ezviz::CSdkMgrEzvizPrivate::startSearch(const std::string& session, const std::string& cameraId, const std::string& accessId, const std::string& startTime, const std::string& stopTime, LP_NSCBMsg pNSCBMsg)
{
	return m_apis.pOpenSDK_StartSearch(session.c_str(), cameraId.c_str(),
									   accessId.c_str(), startTime.c_str(),
									   stopTime.c_str(), pNSCBMsg);
}

int sdk_mgr_ezviz::CSdkMgrEzvizPrivate::getOSDTime(const std::string& session, STREAM_TIME* pTime)
{
	return m_apis.pOpenSDK_GetOSDTime(session.c_str(), pTime);
}

int sdk_mgr_ezviz::CSdkMgrEzvizPrivate::getDevList(const std::string& accessId, int pageStart, int pageSize, void** pBuf, int* length)
{
	return m_apis.pOpenSDK_Data_GetDevList(accessId.c_str(), pageStart, pageSize, pBuf, length);
}

int sdk_mgr_ezviz::CSdkMgrEzvizPrivate::getDevInfo(const std::string& accessId, const std::string& devSerial, void** pBuf, int* length)
{
	if (m_apis.pOpenSDK_Data_GetDeviceInfo) {
		return m_apis.pOpenSDK_Data_GetDeviceInfo(accessId.c_str(), devSerial.c_str(), pBuf, length);
	}
	return -1;
}

int sdk_mgr_ezviz::CSdkMgrEzvizPrivate::getAlarmList(const std::string& accessId, const std::string& cameraId, const std::string& startTime, const std::string& endTime, int alarmType, int status, int pageStart, int pageSize, void** pBuf, int* length)
{
	if (m_apis.pOpenSDK_Data_GetAlarmList) {
		return m_apis.pOpenSDK_Data_GetAlarmList(accessId.c_str(), cameraId.c_str(),
												 startTime.c_str(), endTime.c_str(),
												 alarmType, status, pageStart, pageSize, pBuf, length);
	}
	return -1;
}

int sdk_mgr_ezviz::CSdkMgrEzvizPrivate::setAlarmRead(const std::string& accessId, const std::string& alarmId)
{
	if (m_apis.pOpenSDK_Data_SetAlarmRead) {
		return m_apis.pOpenSDK_Data_SetAlarmRead(accessId.c_str(), alarmId.c_str());
	}
	return -1;
}

int sdk_mgr_ezviz::CSdkMgrEzvizPrivate::deleteDevice(const std::string& accessId, const std::string& cameraId)
{
	return m_apis.pOpenSDK_Data_DeleteDevice(accessId.c_str(), cameraId.c_str());
}

int sdk_mgr_ezviz::CSdkMgrEzvizPrivate::freeData(void* pBuf)
{
	return m_apis.pOpenSDK_Data_Free(pBuf);
}

int sdk_mgr_ezviz::CSdkMgrEzvizPrivate::openSound(const std::string& session)
{
	if (m_apis.pOpenSDK_OpenSound) {
		return m_apis.pOpenSDK_OpenSound(session.c_str());
	}
	return -1;
}

int sdk_mgr_ezviz::CSdkMgrEzvizPrivate::closeSound(const std::string& session)
{
	if (m_apis.pOpenSDK_CloseSound) {
		return m_apis.pOpenSDK_CloseSound(session.c_str());
	}
	return -1;
}

int sdk_mgr_ezviz::CSdkMgrEzvizPrivate::getVolume(const std::string& session)
{
	if (m_apis.pOpenSDK_GetVolume) {
		return m_apis.pOpenSDK_GetVolume(session.c_str());
	}
	return -1;
}

int sdk_mgr_ezviz::CSdkMgrEzvizPrivate::setVolume(const std::string& session, unsigned short uVolume)
{
	if (m_apis.pOpenSDK_SetVolume) {
		return m_apis.pOpenSDK_SetVolume(session.c_str(), uVolume);
	}
	return -1;
}

int sdk_mgr_ezviz::CSdkMgrEzvizPrivate::startVoiceTalk(const std::string& session, const std::string& accessId, const std::string& cameraId)
{
	if (m_apis.pOpenSDK_StartVoiceTalk) {
		return m_apis.pOpenSDK_StartVoiceTalk(session.c_str(), accessId.c_str(), cameraId.c_str());
	}
	return -1;
}

int sdk_mgr_ezviz::CSdkMgrEzvizPrivate::stopVoiceTalk(const std::string& session)
{
	if (m_apis.pOpenSDK_StopVoiceTalk) {
		return m_apis.pOpenSDK_StopVoiceTalk(session.c_str());
	}
	return -1;
}

int sdk_mgr_ezviz::CSdkMgrEzvizPrivate::capturePicture(const std::string& session, const std::string& szFileName)
{
	if (m_apis.pOpenSDK_CapturePicture) {
		return m_apis.pOpenSDK_CapturePicture(session.c_str(), szFileName.c_str());
	}
	return -1;
}

int sdk_mgr_ezviz::CSdkMgrEzvizPrivate::setAlarmMsgCallBack(OpenSDK_Alarm_MessageHandler handler, OpenSDK_Publish_MessageHandler publishHandler, void* pUser)
{
	if (m_apis.pOpenSDK_Alarm_SetMsgCallBack) {
		return m_apis.pOpenSDK_Alarm_SetMsgCallBack(handler, publishHandler, pUser);
	}
	return -1;
}

int sdk_mgr_ezviz::CSdkMgrEzvizPrivate::startAlarmRecv(const std::string& accessId)
{
	if (m_apis.pOpenSDK_Alarm_StartRecv) {
		return m_apis.pOpenSDK_Alarm_StartRecv(accessId.c_str());
	}
	return -1;
}

int sdk_mgr_ezviz::CSdkMgrEzvizPrivate::stopAlarmRecv()
{
	if (m_apis.pOpenSDK_Alarm_StopRecv) {
		return m_apis.pOpenSDK_Alarm_StopRecv();
	}
	return -1;
}

int sdk_mgr_ezviz::CSdkMgrEzvizPrivate::PTZCtrl(const std::string& szSessionId, const std::string& szAccessToken, const std::string& szCameraId, PTZCommand enCommand, PTZAction enAction, int iSpeed, LP_NSCBMsg pNSCBMsg)
{
	if (m_apis.pOpenSDK_PTZCtrl) {
		return m_apis.pOpenSDK_PTZCtrl(szSessionId.c_str(), szAccessToken.c_str(),
									   szCameraId.c_str(), enCommand, enAction, iSpeed, pNSCBMsg);
	}
	return -1;
}

int sdk_mgr_ezviz::CSdkMgrEzvizPrivate::DevDefence(const std::string& szSessionId, const std::string& szAccessToken, const std::string& szCameraId, DefenceType enType, DefenceStatus enStatus, DefenceActor enActor, LP_NSCBMsg pNSCBMsg)
{
	AUTO_LOG_FUNCTION;
	if (m_apis.pOpenSDK_DevDefence) {
		return m_apis.pOpenSDK_DevDefence(szSessionId.c_str(), szAccessToken.c_str(),
										  szCameraId.c_str(), enType, enStatus, enActor, pNSCBMsg);
	}
	return -1;
}

int sdk_mgr_ezviz::CSdkMgrEzvizPrivate::DevDefenceByDev(const std::string& szSessionId, const std::string& szAccessToken, const std::string& szDeviceId, int iCameraNo, DefenceType enType, DefenceStatus enStatus, DefenceActor enActor, LP_NSCBMsg pNSCBMsg)
{
	AUTO_LOG_FUNCTION;
	if (m_apis.pOpenSDK_DevDefenceByDev) {
		return m_apis.pOpenSDK_DevDefenceByDev(szSessionId.c_str(), szAccessToken.c_str(),
											   szDeviceId.c_str(), iCameraNo, enType, enStatus, enActor, pNSCBMsg);
	}
	return -1;
}

int sdk_mgr_ezviz::CSdkMgrEzvizPrivate::GetAccessTokenSmsCode(const std::string& szSignString)
{
	if (m_apis.pOpenSDK_GetAccessTokenSmsCode) {
		return m_apis.pOpenSDK_GetAccessTokenSmsCode(szSignString.c_str());
	}
	return -1;
}

int sdk_mgr_ezviz::CSdkMgrEzvizPrivate::VerifyAccessTokenSmsCode(const std::string& szSmsCode, const std::string& szUserId, const std::string& szPhoneNumber, const std::string& szAppKey)
{
	if (m_apis.pOpenSDK_VerifyAccessTokenSmsCode) {
		return m_apis.pOpenSDK_VerifyAccessTokenSmsCode(szSmsCode.c_str(), szUserId.c_str(), szPhoneNumber.c_str(), szAppKey.c_str());
	}
	return -1;
}

int sdk_mgr_ezviz::CSdkMgrEzvizPrivate::GetHdSignSmsCode(const std::string& szAccessToken, const std::string& szSignString)
{
	if (m_apis.pOpenSDK_GetHdSignSmsCode) {
		return m_apis.pOpenSDK_GetHdSignSmsCode(szAccessToken.c_str(), szSignString.c_str());
	}
	return -1;
}

int sdk_mgr_ezviz::CSdkMgrEzvizPrivate::VerifyHdSignSmsCode(const std::string& szAccessToken, const std::string& szSmsCode, const std::string& szUserId, const std::string& szAppKey)
{
	if (m_apis.pOpenSDK_VerifyHdSignSmsCode) {
		return m_apis.pOpenSDK_VerifyHdSignSmsCode(szAccessToken.c_str(), szSmsCode.c_str(), szUserId.c_str(), szAppKey.c_str());
	}
	return -1;
}

int sdk_mgr_ezviz::CSdkMgrEzvizPrivate::UpdateCameraInfo(const std::string& szCamera, const std::string& szAccessToken, bool& isEncrypt)
{
	if (m_apis.pOpenSDK_UpdateCameraInfoToLocal) {
		return m_apis.pOpenSDK_UpdateCameraInfoToLocal(szCamera.c_str(), szAccessToken.c_str(), isEncrypt);
	}
	return -1;
}


int sdk_mgr_ezviz::CSdkMgrEzvizPrivate::RequestPassThrough(const std::string& reqStr, char** pBuf, int* iLength)
{
	if (m_apis.pOpenSDK_RequestPassThrough) {
		return m_apis.pOpenSDK_RequestPassThrough(reqStr.c_str(), pBuf, iLength);
	}
	return -1;
}
#pragma endregion




bool sdk_mgr_ezviz::Init(const std::string& appKey)
{
	do {
		int ret = 0;
		ret = m_dll.initLibrary("https://auth.ys7.com", "https://open.ys7.com", appKey);
		if (ret != 0) {
			JLOG(L"init failed: %d\n", ret);
			break;
		}
		return true;
	} while (0);

	return false;
}


std::string sdk_mgr_ezviz::GetSessionId(const std::string& user_phone, const std::string& cameraId,
										sdk_mgr_ezviz::OpenSDK_MessageHandler messageHandler, void* data)
{
	bool bExists = false;
	std::string sessionId;
	auto iter1 = _sessionMap.find(user_phone);
	if (iter1 == _sessionMap.end()) {
		bExists = false;
	} else {
		auto iter2 = iter1->second.find(cameraId);
		bExists = iter2 != iter1->second.end();
	}

	if (bExists) {
		sessionId = _sessionMap[user_phone][cameraId];
	} else {
		sessionId = m_dll.allocSession(messageHandler, data);
		if (!sessionId.empty())
			_sessionMap[user_phone][cameraId] = sessionId;
	}
	return sessionId;
}


void sdk_mgr_ezviz::FreeUserSession(const std::string& user_phone)
{
	auto iter = _sessionMap.find(user_phone);
	if (iter != _sessionMap.end()) {
		for (auto iter2 : iter->second) {
			m_dll.freeSession(iter2.second);
		}
		_sessionMap.erase(user_phone);
	}
}


void sdk_mgr_ezviz::FreeSession(const std::string& sesson_id)
{
	for (auto& iter : _sessionMap) {
		for (auto& iter2 : iter.second) {
			if (iter2.second == sesson_id) {
				m_dll.freeSession(sesson_id);
				iter.second.erase(iter2.first);
				return;
			}
		}
	}
}

//inline bool u16_to_utf8(const wchar_t* u16, char* utf8buffer, size_t u8size) {
//	size_t request_size = WideCharToMultiByte(CP_UTF8, 0, u16, -1, NULL, 0, 0, 0);
//	if (0 < request_size && request_size < u8size) {
//		WideCharToMultiByte(CP_UTF8, 0, u16, -1, utf8buffer, request_size, 0, 0);
//		return true;
//	}
//	return false;
//};

bool sdk_mgr_ezviz::GetUsersDeviceList(ezviz_user_ptr user,
									   ezviz_device_list& devList)
{
	AUTO_LOG_FUNCTION;
	assert(user);
	auto vmgr = video::video_manager::get_instance();
	if (user->get_acc_token().size() == 0) {
		if (RESULT_OK != VerifyUserAccessToken(user, TYPE_GET)) {
			return false;
		}
		vmgr->execute_set_ezviz_users_acc_token(user, user->get_acc_token());
	}
	int ret = 0;
	void* buff = nullptr;
	int l = 0;
	ret = m_dll.getDevList(user->get_acc_token(), 0, 1500, &buff, &l);
	if (ret != 0) {
		/*assert(0); */JLOG(L"getDevList faild %d\n", ret); return false;
	}
	std::string json = static_cast<char*>(buff);
	JLOGA(json.c_str());
	m_dll.freeData(buff);
	Json::Reader reader;
	Json::Value	value;
	if (reader.parse(json.data(), value) && value["resultCode"].asString() == "200") {
		Json::Value &cameraListVal = value["cameraList"];
		CString txt;
		if (cameraListVal.isArray()) {
			int cameraCount = cameraListVal.size();
			for (int i = 0; i < cameraCount; i++) {
#define GetUsersDeviceList_GET_AS_STRING(VAL) { device->set_##VAL(cameraListVal[i][#VAL].asString().c_str());  }
#define GetUsersDeviceList_GET_AS_INT(VAL) { device->set_##VAL(cameraListVal[i][#VAL].asInt());  }

				ezviz_device_ptr device = std::make_shared<ezviz_device>();
				GetUsersDeviceList_GET_AS_STRING(cameraId);

				std::string cameraName = cameraListVal[i]["cameraName"].asString();
				/*std::wstring wname = A2W(cameraName.c_str());
				const char* c8 = cameraListVal[i]["cameraName"].asCString();
				if (cameraName.size() > 8) {
					size_t pos = 4;
					while (isdigit((int)(unsigned char)c8[pos]) && pos < cameraName.size()) {
						pos++;
					}
					if (c8[pos] == '@') {
						auto p = std::unique_ptr<char[]>(new char[cameraName.size() - 4 + 1]);
						memcpy(p.get(), &c8[4], cameraName.size() - 4);
						p[cameraName.size() - 4] = 0;
						std::string s = "camera";
						s += p.get();
						wname = A2W(s.c_str());
					}
				}*/
				JLOGA(cameraName.c_str());
				JLOGA(cameraName.c_str());
				wchar_t buffer[1024] = { 0 };
				if (utf8::mbcs_to_u16(cameraName.c_str(), buffer, 1024)) {
					device->set_cameraName(buffer);
				} else {
					device->set_cameraName(utf8::a2w(cameraName));
				}

				GetUsersDeviceList_GET_AS_INT(cameraNo);
				GetUsersDeviceList_GET_AS_INT(defence);
				GetUsersDeviceList_GET_AS_STRING(deviceId);
				//GetUsersDeviceList_GET_AS_STRING(deviceName); 
				std::string deviceName = cameraListVal[i]["deviceName"].asString();
				JLOGA(deviceName.c_str());
				if (utf8::mbcs_to_u16(deviceName.c_str(), buffer, 1024)) {
					/*char ansi[1024] = { 0 };
					if (u16_to_utf8(buffer, ansi, 1024)) {

						device->set_deviceName(ansi);
					} else {
						device->set_deviceName(W2A(buffer));
					}*/

					device->set_deviceName(buffer);
				} else {
					device->set_deviceName(utf8::a2w(deviceName));
				}

				GetUsersDeviceList_GET_AS_STRING(deviceSerial);
				GetUsersDeviceList_GET_AS_INT(isEncrypt);
				GetUsersDeviceList_GET_AS_STRING(isShared);
				GetUsersDeviceList_GET_AS_STRING(picUrl);
				GetUsersDeviceList_GET_AS_INT(status);

				devList.push_back(device);
			}
			return true;
		}
	}
	return false;
}


bool sdk_mgr_ezviz::VerifyDeviceInfo(ezviz_user_ptr user, ezviz_device_ptr device)
{
	AUTO_LOG_FUNCTION;
	assert(user); assert(device);
	auto vmgr = video::video_manager::get_instance();
	if (user->get_acc_token().size() == 0) {
		if (RESULT_OK != VerifyUserAccessToken(user, TYPE_GET)) {
			return false;
		}
		vmgr->execute_set_ezviz_users_acc_token(user, user->get_acc_token());
	}

	void* buff = nullptr;
	int l = 0;
	int ret = m_dll.getDevInfo(user->get_acc_token(), device->get_deviceSerial(), &buff, &l);
	if (ret != 0) {
		assert(0); JLOG(L"getDevInfo faild %d\n", ret); return false;
	}
	std::string json = static_cast<char*>(buff);
	m_dll.freeData(buff);
	Json::Reader reader;
	Json::Value	value;
	if (reader.parse(json.data(), value) && value["result"]["code"].asString() == "200") {
		Json::Value &cameraListVal = value["result"]["data"];
		CString txt;
		if (cameraListVal.isArray()) {
			assert(cameraListVal.size() == 1);
			int ndx = 0;
			bool bChanged = false;
#define VerifyDeviceInfo_GET_AS_STRING(VAL) { if(device->get_##VAL() != cameraListVal[ndx][#VAL].asString()) { bChanged = true; device->set_##VAL(cameraListVal[ndx][#VAL].asString().c_str()); } }
#define VerifyDeviceInfo_GET_AS_INT(VAL) { if(device->get_##VAL() != cameraListVal[ndx][#VAL].asInt()) { bChanged = true; device->set_##VAL(cameraListVal[ndx][#VAL].asInt()); } }

			VerifyDeviceInfo_GET_AS_STRING(cameraId);
			//VerifyDeviceInfo_GET_AS_STRING(cameraName);
			wchar_t buffer[1024] = { 0 };

			std::string cameraName = cameraListVal[ndx]["cameraName"].asString();
			utf8::mbcs_to_u16(cameraName.c_str(), buffer, 1024);
			if (device->get_cameraName() != buffer) {
				bChanged = true;
				device->set_cameraName(buffer);
			}

			VerifyDeviceInfo_GET_AS_INT(cameraNo);
			VerifyDeviceInfo_GET_AS_INT(defence);
			VerifyDeviceInfo_GET_AS_STRING(deviceId);
			//VerifyDeviceInfo_GET_AS_STRING(deviceName);
			std::string deviceName = cameraListVal[ndx]["deviceName"].asString();
			utf8::mbcs_to_u16(deviceName.c_str(), buffer, 1024);
			if (device->get_deviceName() != buffer) {
				bChanged = true;
				device->set_deviceName(buffer);
			}
			VerifyDeviceInfo_GET_AS_STRING(deviceSerial);
			VerifyDeviceInfo_GET_AS_INT(isEncrypt);
			VerifyDeviceInfo_GET_AS_STRING(isShared);
			VerifyDeviceInfo_GET_AS_STRING(picUrl);
			VerifyDeviceInfo_GET_AS_INT(status);

			if (bChanged) {
				vmgr->execute_update_ezviz_dev(device);
			}

			return true;
		}
	}
	return false;
}


sdk_mgr_ezviz::SdkEzvizResult sdk_mgr_ezviz::VerifyUserAccessToken(ezviz_user_ptr user, msg_type type)
{
	AUTO_LOG_FUNCTION;
	std::string accToken = user->get_acc_token();
	auto cfg = util::CConfigHelper::get_instance();
	auto connector = private_cloud_connector::get_instance();
	if (connector->get_accToken(cfg->get_ezviz_private_cloud_ip(),
								cfg->get_ezviz_private_cloud_port(),
								cfg->get_ezviz_app_key(),
								accToken,
								user->get_user_phone(),
								user->get_user_phone(),
								type)) {
		user->set_acc_token(accToken);

		return RESULT_OK;
	} else {
		return RESULT_PRIVATE_CLOUD_CONNECT_FAILED_OR_USER_NOT_EXSIST;
	}
}



};
};
