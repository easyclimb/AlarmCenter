#include "stdafx.h"
#include "SdkMgrEzviz.h"
#include "VideoUserInfoEzviz.h"
#include "VideoDeviceInfoEzviz.h"
#include "json/json.h"

namespace core {
namespace video {
namespace ezviz {


IMPLEMENT_SINGLETON(CSdkMgrEzviz)
CSdkMgrEzviz::CSdkMgrEzviz()
	: m_curSessionId()
	, m_dll()
{
	
}


CSdkMgrEzviz::~CSdkMgrEzviz()
{
	if (m_curSessionId.size() > 0) {
		m_dll.freeSession(m_curSessionId);
	}
	m_dll.releaseLibrary();
}


#pragma region CSdkMgrEzvizPrivate
CSdkMgrEzviz::CSdkMgrEzvizPrivate::CSdkMgrEzvizPrivate()
{
	AUTO_LOG_FUNCTION;
	CString path; path.Format(L"%s\\3rdparty\\ezviz", GetModuleFilePath());
	SetCurrentDirectory(path);
	path += L"\\OpenNetStream.dll";
	m_library = LoadLibrary(path);
	if (m_library == NULL) {
		LOG(L"load %s falied, err: %d\n", path, GetLastError());
		ExitProcess(ERROR_FILE_NOT_FOUND);
	}
	SetCurrentDirectory(GetModuleFilePath());

	bool ok = false;
#define GET_PROC(proc_name) m_apis.p##proc_name = (proc_name)GetProcAddress(m_library, #proc_name); if(m_apis.p##proc_name == NULL){LOGA("get proc addr faild: %s", #proc_name); break;}
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
		GET_PROC(OpenSDK_HttpSendWithWait);
		/*GET_PROC(OpenSDK_InitLib);
		GET_PROC(OpenSDK_InitLib);
		GET_PROC(OpenSDK_InitLib);*/
		ok = true;
	} while (0);

	if (!ok) {
		ExitProcess(2);
	}
}


CSdkMgrEzviz::CSdkMgrEzvizPrivate::~CSdkMgrEzvizPrivate()
{
	AUTO_LOG_FUNCTION;
	if (m_library) {
		FreeLibrary(m_library);
		m_library = NULL;
	}
}


int CSdkMgrEzviz::CSdkMgrEzvizPrivate::initLibrary(const std::string& authAddr, const std::string& platform, const std::string& appKey)
{
	return m_apis.pOpenSDK_InitLib(authAddr.c_str(), platform.c_str(), appKey.c_str());
}

int CSdkMgrEzviz::CSdkMgrEzvizPrivate::releaseLibrary()
{
	return m_apis.pOpenSDK_FiniLib();
}

std::string CSdkMgrEzviz::CSdkMgrEzvizPrivate::oauth_login()
{
	char* pToken = NULL;
	int length = 0;
	m_apis.pOpenSDK_Mid_Login(&pToken, &length);
	LOGA("OpenSDK_Mid_Login token: %s\n", pToken);
	std::string token = pToken;
	return token;
}

int CSdkMgrEzviz::CSdkMgrEzvizPrivate::deviceAdd(const std::string& accessId)
{
	return m_apis.pOpenSDK_Mid_Device_Add(accessId.c_str());
}

int CSdkMgrEzviz::CSdkMgrEzvizPrivate::deviceOper(const std::string& accessId, const std::string& deviceId)
{
	return m_apis.pOpenSDK_Mid_Device_Oper(accessId.c_str(), deviceId.c_str());
}

std::string CSdkMgrEzviz::CSdkMgrEzvizPrivate::allocSession(OpenSDK_MessageHandler handle, void* pUser, const bool bSync, const unsigned int iTimeOut)
{
	char* pSession = NULL;
	int length = 0;
	m_apis.pOpenSDK_AllocSession(handle, pUser, &pSession, &length, bSync, iTimeOut);
	std::string session = pSession;
	freeData(pSession);
	return session;
}

int CSdkMgrEzviz::CSdkMgrEzvizPrivate::freeSession(const std::string& session)
{
	return m_apis.pOpenSDK_FreeSession(session.c_str());
}

int CSdkMgrEzviz::CSdkMgrEzvizPrivate::startRealPlay(const std::string& session, HWND hPlayWnd, const std::string& cameraId, const std::string& accessId, const std::string& safekey, const std::string& appKey, int videoLevel, LP_NSCBMsg pNSCBMsg)
{
	return m_apis.pOpenSDK_StartRealPlay(session.c_str(), hPlayWnd,
											cameraId.c_str(), accessId.c_str(),
											videoLevel, safekey.c_str(), appKey.c_str(), pNSCBMsg);
}

int CSdkMgrEzviz::CSdkMgrEzvizPrivate::stopRealPlay(const std::string& session, LP_NSCBMsg pNSCBMsg)
{
	return m_apis.pOpenSDK_StopRealPlay(session.c_str(), pNSCBMsg);
}

int CSdkMgrEzviz::CSdkMgrEzvizPrivate::startPlayBack(const std::string& session, HWND hPlayWnd, const std::string& cameraId, const std::string& accessId, const std::string& safekey, const std::string& startTime, const std::string& stopTime, const std::string& appKey, LP_NSCBMsg pNSCBMsg)
{
	return m_apis.pOpenSDK_StartPlayBack(session.c_str(), hPlayWnd,
											cameraId.c_str(), accessId.c_str(),
											safekey.c_str(), startTime.c_str(),
											stopTime.c_str(), appKey.c_str(), pNSCBMsg);
}

int CSdkMgrEzviz::CSdkMgrEzvizPrivate::playBackResume(const std::string& session)
{
	return m_apis.pOpenSDK_PlayBackResume(session.c_str());
}

int CSdkMgrEzviz::CSdkMgrEzvizPrivate::playBackPause(const std::string& session)
{
	return m_apis.pOpenSDK_PlayBackPause(session.c_str());
}

int CSdkMgrEzviz::CSdkMgrEzvizPrivate::stopPlayBack(const std::string& session, LP_NSCBMsg pNSCBMsg)
{
	return m_apis.pOpenSDK_StopPlayBack(session.c_str(), pNSCBMsg);
}

int CSdkMgrEzviz::CSdkMgrEzvizPrivate::setDataCallBack(const std::string& szSessionId, OpenSDK_DataCallBack pDataCallBack, void* pUser)
{
	return m_apis.pOpenSDK_SetDataCallBack(szSessionId.c_str(), pDataCallBack, pUser);
}

int CSdkMgrEzviz::CSdkMgrEzvizPrivate::startSearch(const std::string& session, const std::string& cameraId, const std::string& accessId, const std::string& startTime, const std::string& stopTime, LP_NSCBMsg pNSCBMsg)
{
	return m_apis.pOpenSDK_StartSearch(session.c_str(), cameraId.c_str(),
										  accessId.c_str(), startTime.c_str(),
										  stopTime.c_str(), pNSCBMsg);
}

int CSdkMgrEzviz::CSdkMgrEzvizPrivate::getOSDTime(const std::string& session, STREAM_TIME* pTime)
{
	return m_apis.pOpenSDK_GetOSDTime(session.c_str(), pTime);
}

int CSdkMgrEzviz::CSdkMgrEzvizPrivate::getDevList(const std::string& accessId, int pageStart, int pageSize, void** pBuf, int* length)
{
	return m_apis.pOpenSDK_Data_GetDevList(accessId.c_str(), pageStart, pageSize, pBuf, length);
}

int CSdkMgrEzviz::CSdkMgrEzvizPrivate::getDevInfo(const std::string& accessId, const std::string& devSerial, void** pBuf, int* length)
{
	if (m_apis.pOpenSDK_Data_GetDeviceInfo) {
		return m_apis.pOpenSDK_Data_GetDeviceInfo(accessId.c_str(), devSerial.c_str(), pBuf, length);
	}
	return -1;
}

int CSdkMgrEzviz::CSdkMgrEzvizPrivate::getAlarmList(const std::string& accessId, const std::string& cameraId, const std::string& startTime, const std::string& endTime, int alarmType, int status, int pageStart, int pageSize, void** pBuf, int* length)
{
	if (m_apis.pOpenSDK_Data_GetAlarmList) {
		return m_apis.pOpenSDK_Data_GetAlarmList(accessId.c_str(), cameraId.c_str(),
													startTime.c_str(), endTime.c_str(),
													alarmType, status, pageStart, pageSize, pBuf, length);
	}
	return -1;
}

int CSdkMgrEzviz::CSdkMgrEzvizPrivate::setAlarmRead(const std::string& accessId, const std::string& alarmId)
{
	if (m_apis.pOpenSDK_Data_SetAlarmRead) {
		return m_apis.pOpenSDK_Data_SetAlarmRead(accessId.c_str(), alarmId.c_str());
	}
	return -1;
}

int CSdkMgrEzviz::CSdkMgrEzvizPrivate::deleteDevice(const std::string& accessId, const std::string& cameraId)
{
	return m_apis.pOpenSDK_Data_DeleteDevice(accessId.c_str(), cameraId.c_str());
}

int CSdkMgrEzviz::CSdkMgrEzvizPrivate::freeData(void* pBuf)
{
	return m_apis.pOpenSDK_Data_Free(pBuf);
}

int CSdkMgrEzviz::CSdkMgrEzvizPrivate::openSound(const std::string& session)
{
	if (m_apis.pOpenSDK_OpenSound) {
		return m_apis.pOpenSDK_OpenSound(session.c_str());
	}
	return -1;
}

int CSdkMgrEzviz::CSdkMgrEzvizPrivate::closeSound(const std::string& session)
{
	if (m_apis.pOpenSDK_CloseSound) {
		return m_apis.pOpenSDK_CloseSound(session.c_str());
	}
	return -1;
}

int CSdkMgrEzviz::CSdkMgrEzvizPrivate::getVolume(const std::string& session)
{
	if (m_apis.pOpenSDK_GetVolume) {
		return m_apis.pOpenSDK_GetVolume(session.c_str());
	}
	return -1;
}

int CSdkMgrEzviz::CSdkMgrEzvizPrivate::setVolume(const std::string& session, unsigned short uVolume)
{
	if (m_apis.pOpenSDK_SetVolume) {
		return m_apis.pOpenSDK_SetVolume(session.c_str(), uVolume);
	}
	return -1;
}

int CSdkMgrEzviz::CSdkMgrEzvizPrivate::startVoiceTalk(const std::string& session, const std::string& accessId, const std::string& cameraId)
{
	if (m_apis.pOpenSDK_StartVoiceTalk) {
		return m_apis.pOpenSDK_StartVoiceTalk(session.c_str(), accessId.c_str(), cameraId.c_str());
	}
	return -1;
}

int CSdkMgrEzviz::CSdkMgrEzvizPrivate::stopVoiceTalk(const std::string& session)
{
	if (m_apis.pOpenSDK_StopVoiceTalk) {
		return m_apis.pOpenSDK_StopVoiceTalk(session.c_str());
	}
	return -1;
}

int CSdkMgrEzviz::CSdkMgrEzvizPrivate::capturePicture(const std::string& session, const std::string& szFileName)
{
	if (m_apis.pOpenSDK_CapturePicture) {
		return m_apis.pOpenSDK_CapturePicture(session.c_str(), szFileName.c_str());
	}
	return -1;
}

int CSdkMgrEzviz::CSdkMgrEzvizPrivate::setAlarmMsgCallBack(OpenSDK_Alarm_MessageHandler handler, OpenSDK_Publish_MessageHandler publishHandler, void* pUser)
{
	if (m_apis.pOpenSDK_Alarm_SetMsgCallBack) {
		return m_apis.pOpenSDK_Alarm_SetMsgCallBack(handler, publishHandler, pUser);
	}
	return -1;
}

int CSdkMgrEzviz::CSdkMgrEzvizPrivate::startAlarmRecv(const std::string& accessId)
{
	if (m_apis.pOpenSDK_Alarm_StartRecv) {
		return m_apis.pOpenSDK_Alarm_StartRecv(accessId.c_str());
	}
	return -1;
}

int CSdkMgrEzviz::CSdkMgrEzvizPrivate::stopAlarmRecv()
{
	if (m_apis.pOpenSDK_Alarm_StopRecv) {
		return m_apis.pOpenSDK_Alarm_StopRecv();
	}
	return -1;
}

int CSdkMgrEzviz::CSdkMgrEzvizPrivate::PTZCtrl(const std::string& szSessionId, const std::string& szAccessToken, const std::string& szCameraId, PTZCommand enCommand, PTZAction enAction, int iSpeed, LP_NSCBMsg pNSCBMsg)
{
	if (m_apis.pOpenSDK_PTZCtrl) {
		return m_apis.pOpenSDK_PTZCtrl(szSessionId.c_str(), szAccessToken.c_str(),
										  szCameraId.c_str(), enCommand, enAction, iSpeed, pNSCBMsg);
	}
	return -1;
}

int CSdkMgrEzviz::CSdkMgrEzvizPrivate::DevDefence(const std::string& szSessionId, const std::string& szAccessToken, const std::string& szCameraId, DefenceType enType, DefenceStatus enStatus, DefenceActor enActor, LP_NSCBMsg pNSCBMsg)
{
	AUTO_LOG_FUNCTION;
	if (m_apis.pOpenSDK_DevDefence) {
		return m_apis.pOpenSDK_DevDefence(szSessionId.c_str(), szAccessToken.c_str(),
											 szCameraId.c_str(), enType, enStatus, enActor, pNSCBMsg);
	}
	return -1;
}

int CSdkMgrEzviz::CSdkMgrEzvizPrivate::DevDefenceByDev(const std::string& szSessionId, const std::string& szAccessToken, const std::string& szDeviceId, int iCameraNo, DefenceType enType, DefenceStatus enStatus, DefenceActor enActor, LP_NSCBMsg pNSCBMsg)
{
	AUTO_LOG_FUNCTION;
	if (m_apis.pOpenSDK_DevDefenceByDev) {
		return m_apis.pOpenSDK_DevDefenceByDev(szSessionId.c_str(), szAccessToken.c_str(),
												  szDeviceId.c_str(), iCameraNo, enType, enStatus, enActor, pNSCBMsg);
	}
	return -1;
}

int CSdkMgrEzviz::CSdkMgrEzvizPrivate::GetAccessTokenSmsCode(const std::string& szSignString)
{
	if (m_apis.pOpenSDK_GetAccessTokenSmsCode) {
		return m_apis.pOpenSDK_GetAccessTokenSmsCode(szSignString.c_str());
	}
	return -1;
}

int CSdkMgrEzviz::CSdkMgrEzvizPrivate::VerifyAccessTokenSmsCode(const std::string& szSmsCode, const std::string& szUserId, const std::string& szPhoneNumber, const std::string& szAppKey)
{
	if (m_apis.pOpenSDK_VerifyAccessTokenSmsCode) {
		return m_apis.pOpenSDK_VerifyAccessTokenSmsCode(szSmsCode.c_str(), szUserId.c_str(), szPhoneNumber.c_str(), szAppKey.c_str());
	}
	return -1;
}

int CSdkMgrEzviz::CSdkMgrEzvizPrivate::GetHdSignSmsCode(const std::string& szAccessToken, const std::string& szSignString)
{
	if (m_apis.pOpenSDK_GetHdSignSmsCode) {
		return m_apis.pOpenSDK_GetHdSignSmsCode(szAccessToken.c_str(), szSignString.c_str());
	}
	return -1;
}

int CSdkMgrEzviz::CSdkMgrEzvizPrivate::VerifyHdSignSmsCode(const std::string& szAccessToken, const std::string& szSmsCode, const std::string& szUserId, const std::string& szAppKey)
{
	if (m_apis.pOpenSDK_VerifyHdSignSmsCode) {
		return m_apis.pOpenSDK_VerifyHdSignSmsCode(szAccessToken.c_str(), szSmsCode.c_str(), szUserId.c_str(), szAppKey.c_str());
	}
	return -1;
}

int CSdkMgrEzviz::CSdkMgrEzvizPrivate::UpdateCameraInfo(const std::string& szCamera, const std::string& szAccessToken, bool& isEncrypt)
{
	if (m_apis.pOpenSDK_UpdateCameraInfoToLocal) {
		return m_apis.pOpenSDK_UpdateCameraInfoToLocal(szCamera.c_str(), szAccessToken.c_str(), isEncrypt);
	}
	return -1;
}


int CSdkMgrEzviz::CSdkMgrEzvizPrivate::HttpSendWithWait(const char* szUri, const char* szHeaderParam, const char* szBody, char** pBuf, int* iLength)
{
	if (m_apis.pOpenSDK_HttpSendWithWait) {
		return m_apis.pOpenSDK_HttpSendWithWait(szUri, szHeaderParam, szBody, pBuf, iLength);
	}
	return -1;
}
#pragma endregion


void __stdcall CSdkMgrEzviz::messageHandler(const char *szSessionId,
											unsigned int iMsgType,
											unsigned int iErrorCode,
											const char *pMessageInfo,
											void *pUser)
{
	AUTO_LOG_FUNCTION;
	LOGA("(const char *szSessionId, %s\r\n\
unsigned int iMsgType, %d\r\n\
unsigned int iErrorCode, %d\r\n\
const char *pMessageInfo, %s\r\n\
void *pUser)\r\n", szSessionId, iMsgType, iErrorCode, pMessageInfo);


	switch (iMsgType) {
		case INS_PLAY_EXCEPTION: // 播放异常
			//pInstance->insPlayException(iErrorCode, pMessageInfo);
			break;
		case INS_PLAY_RECONNECT: 
			break;
		case INS_PLAY_RECONNECT_EXCEPTION: // 重连异常
			//pInstance->insPlayReconnectException(iErrorCode, pMessageInfo);
			break;
		case INS_PLAY_START:
			break;
		case INS_PLAY_STOP:
			break;
		case INS_PLAY_ARCHIVE_END:
			break;
		case INS_RECORD_FILE: // 查询回放成功，返回回放列表
			//pInstance->insRecordFile(pMessageInfo);
			break;
		case INS_RECORD_SEARCH_END:
			break;
		case INS_RECORD_SEARCH_FAILED: // 查询回放失败
			//pInstance->insRecordSearchFailed(iErrorCode, pMessageInfo);
			break;
	}
}


bool CSdkMgrEzviz::Init(const std::string& appKey) 
{
	do {
		int ret = 0;
		ret = m_dll.initLibrary("https://auth.ys7.com", "https://auth.ys7.com", appKey);
		if (ret != 0) {
			LOG(L"init failed: %d\n", ret);
			break;
		}
		m_curSessionId = m_dll.allocSession(messageHandler, this);
		LOG(L"cur session: %s\n", m_curSessionId.c_str());
		return true;
	} while (0);
	
	return false;
}


bool CSdkMgrEzviz::GetUsersDeviceList(CVideoUserInfoEzviz* user, 
									  CVideoDeviceInfoEzvizList& devList)
{
	AUTO_LOG_FUNCTION;
	USES_CONVERSION;
	assert(user);
	int ret = 0;
	void* buff = NULL;
	int l = 0;
	ret = m_dll.getDevList(user->get_user_accToken(), 0, 1500, &buff, &l);
	if (ret != 0) {
		assert(0); LOG(L"getDevList faild %d\n", ret); return false;
	}
	std::string json = static_cast<char*>(buff);
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

				CVideoDeviceInfoEzviz* device = new CVideoDeviceInfoEzviz();
				GetUsersDeviceList_GET_AS_STRING(cameraId);
				GetUsersDeviceList_GET_AS_STRING(cameraName);
				GetUsersDeviceList_GET_AS_INT(cameraNo);
				GetUsersDeviceList_GET_AS_INT(defence);
				GetUsersDeviceList_GET_AS_STRING(deviceId);
				GetUsersDeviceList_GET_AS_STRING(deviceName);
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

NAMESPACE_END
NAMESPACE_END
NAMESPACE_END
