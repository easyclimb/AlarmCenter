#pragma once
#include <map>
#include "video.h"


namespace video {
namespace ezviz {

class CSdkMgrEzviz
{
public:
#pragma region defs
	typedef struct _STREAM_TIME
	{
		unsigned int iYear;		
		unsigned int iMonth;	
		unsigned int iDay;		
		unsigned int iHour;		
		unsigned int iMinute;	
		unsigned int iSecond;	
	}STREAM_TIME, *LP_STREAM_TIME;

	typedef struct _NetStreamCallBackMessage
	{
	public:
		_NetStreamCallBackMessage(void) :
			iErrorCode(0), pMessageInfo(nullptr)
		{}
		unsigned int iErrorCode;
		char *pMessageInfo;
	} NSCBMsg, *LP_NSCBMsg;

	typedef enum MessageType
	{
		INS_PLAY_EXCEPTION				= 0,   
		INS_PLAY_RECONNECT				= 1,  
		INS_PLAY_RECONNECT_EXCEPTION	= 2,  
		INS_PLAY_START					= 3,
		INS_PLAY_STOP					= 4,  
		INS_PLAY_ARCHIVE_END			= 5,   

		INS_RECORD_FILE					= 20, 
		INS_RECORD_SEARCH_END			= 21,  
		INS_RECORD_SEARCH_FAILED		= 22,  
		INS_DEFENSE_SUCCESS				= 23, 
		INS_DEFENSE_FAILED				= 24, 
		INS_PTZCTRL_SUCCESS				= 46, 
		INS_PTZCTRL_FAILED				= 47,
	}MessageType;

	typedef enum DataType
	{
		NET_DVR_SYSHEAD = 1,     
		NET_DVR_STREAMDATA = 2,	
		NET_DVR_RECV_END = 3,   
	}DataType;

	typedef enum PTZCommand
	{
		UP,
		DOWN,
		LEFT,
		RIGHT,
		UPLEFT,
		DOWNLEFT,
		UPRIGHT,
		DOWNRIGHT,
		ZOOMIN,
		ZOOMOUT,
		FOCUSNEAR,
		FOCUSFAR,
		IRISSTARTUP,
		IRISSTOPDOWN,
		LIGHT,
		WIPER,
		AUTO
	}PTZCommand;

	typedef enum PTZAction
	{
		START,
		STOP
	}PTZAction;

#define PTZ_SPEED 7 

	typedef enum DefenceType
	{
		PIR,							
		ATHOME,                         
		OUTDOOR,					    
		BABYCRY,						
		MOTIONDETECT,					
		GLOBAL							
	}DefenceType;

	typedef enum DefenceStatus
	{
		UN_DEFENCE,					
		DEFENCE,					
		UN_SUPPORT,					
		FORCE_DEFENCE,				
	}DefenceStatus;

	typedef enum DefenceActor
	{
		D,							
		V,							
		I,								
	}DefenceActor;

	typedef void(__stdcall *OpenSDK_MessageHandler)(const char* szSessionId, unsigned int iMsgType, unsigned int iErrorCode, const char *pMessageInfo, void *pUser);

	typedef void(__stdcall *OpenSDK_Alarm_MessageHandler)(const char* szCameraId, const char* szContent, const char* szAlarmTime, void* pUser);

	typedef void(__stdcall *OpenSDK_Publish_MessageHandler)(const char* szContent, void* pUser);

	typedef void(__stdcall *OpenSDK_DataCallBack)(DataType enType, char* const pData, int iLen, void* pUser);


	typedef int OPENSDK_RESULT;

	typedef OPENSDK_RESULT(__stdcall *OpenSDK_InitLib)(const char* szAuthAddr, const char* szPlatform, const char* szAppId);

	typedef OPENSDK_RESULT(__stdcall *OpenSDK_FiniLib)();

	typedef OPENSDK_RESULT(__stdcall *OpenSDK_AllocSession)(OpenSDK_MessageHandler pHandle, void* pUser, char** pSession, int* iSessionLen, const bool bSync, const unsigned int iTimeOut);

	typedef OPENSDK_RESULT(__stdcall *OpenSDK_FreeSession)(const char* szSessionId);

	typedef OPENSDK_RESULT(__stdcall *OpenSDK_StartRealPlay)(const char* szSessionId, HWND hPlayWnd, const char* szCameraId, const char* szAccessToken, int iVideoLevel, const char* szSafeKey, const char* szAppKey, LP_NSCBMsg pNSCBMsg);

	typedef OPENSDK_RESULT(__stdcall *OpenSDK_StopRealPlay)(const char* szSessionId, LP_NSCBMsg pNSCBMsg);

	typedef OPENSDK_RESULT(__stdcall *OpenSDK_StartPlayBack)(const char* szSessionId, HWND hPlayWnd, const char* szCameraId, const char* szAccessToken, const char* szSafeKey, const char* szStartTime, const char* szStopTime, const char* szAppKey, LP_NSCBMsg pNSCBMsg);

	typedef OPENSDK_RESULT(__stdcall *OpenSDK_PlayBackResume)(const char* szSessionId);

	typedef OPENSDK_RESULT(__stdcall *OpenSDK_PlayBackPause)(const char* szSessionId);

	typedef OPENSDK_RESULT(__stdcall *OpenSDK_StopPlayBack)(const char* szSessionId, LP_NSCBMsg pNSCBMsg);

	typedef OPENSDK_RESULT(__stdcall *OpenSDK_SetDataCallBack)(const char* szSessionId, OpenSDK_DataCallBack pDataCallBack, void* pUser);

	typedef OPENSDK_RESULT(__stdcall *OpenSDK_StartSearch)(const char* szSessionId, const char* szCameraId, const char* szAccessId, const char* szStartTime, const char* szStopTime, LP_NSCBMsg pNSCBMsg);

	typedef OPENSDK_RESULT(__stdcall *OpenSDK_GetOSDTime)(const char* szSessionId, STREAM_TIME *pTime);

	typedef OPENSDK_RESULT(__stdcall *OpenSDK_OpenSound)(const char* szSessionId);

	typedef OPENSDK_RESULT(__stdcall *OpenSDK_CloseSound)(const char* szSessionId);

	typedef unsigned short(__stdcall *OpenSDK_GetVolume)(const char* szSessionId);

	typedef OPENSDK_RESULT(__stdcall *OpenSDK_SetVolume)(const char* szSessionId, unsigned short uVolume);

	typedef OPENSDK_RESULT(__stdcall *OpenSDK_StartVoiceTalk)(const char* szSessionId, const char* szAccessId, const char* szCameraId);

	typedef OPENSDK_RESULT(__stdcall *OpenSDK_StopVoiceTalk)(const char* szSessionId);

	typedef OPENSDK_RESULT(__stdcall *OpenSDK_CapturePicture)(const char* szSessionId, const char* szFileName);

	typedef OPENSDK_RESULT(__stdcall *OpenSDK_Mid_Login)(char** pAccessToken, int* iAccessTokenLen);

	typedef int(__stdcall *OpenSDK_Mid_Device_Add)(const char* szStrAccessToken);

	typedef int(__stdcall *OpenSDK_Mid_Device_Oper)(const char* szStrAccessToken, const char* szDeviceId);

	typedef OPENSDK_RESULT(__stdcall *OpenSDK_Data_GetDevList)(const char* szAccessId, int iPageStart, int iPageSize, void** pBuf, int* iLength);

	typedef OPENSDK_RESULT(__stdcall *OpenSDK_Data_GetDeviceInfo)(const char* szAccessToken, const char* szDeviceSerial, void** pBuf, int* iLength);


	typedef OPENSDK_RESULT(__stdcall *OpenSDK_Data_GetAlarmList)(const char* szAccessId, const char* szCameraId, const char* szStartTime, const char* szEndTime, int iAlarmType, int iStatus, int iPageStart, int iPageSize, void** pBuf, int* iLength);

	typedef OPENSDK_RESULT(__stdcall *OpenSDK_Data_SetAlarmRead)(const char* szAccessToken, const char* szAlarmId);

	typedef OPENSDK_RESULT(__stdcall *OpenSDK_Data_DeleteDevice)(const char* szAccessId, const char* szDeviceId);

	typedef OPENSDK_RESULT(__stdcall *OpenSDK_Data_Free)(void* pBuf);

	typedef OPENSDK_RESULT(__stdcall *OpenSDK_Alarm_SetMsgCallBack)(OpenSDK_Alarm_MessageHandler handle, OpenSDK_Publish_MessageHandler publishHandle, void* pUser);

	typedef OPENSDK_RESULT(__stdcall *OpenSDK_Alarm_StartRecv)(const char* szAccessId);

	typedef OPENSDK_RESULT(__stdcall *OpenSDK_Alarm_StopRecv)();

	typedef OPENSDK_RESULT(__stdcall *OpenSDK_PTZCtrl)(const char* szSessionId, const char* szAccessToken, const char* szCameraId, PTZCommand enCommand, PTZAction enAction, int iSpeed, LP_NSCBMsg pNSCBMsg);

	typedef OPENSDK_RESULT(__stdcall *OpenSDK_DevDefence)(const char* szSessionId, const char* szAccessToken, const char* szCameraId, DefenceType enType, DefenceStatus enStatus, DefenceActor enActor, LP_NSCBMsg pNSCBMsg);

	typedef OPENSDK_RESULT(__stdcall *OpenSDK_DevDefenceByDev)(const char* szSessionId, const char* szAccessToken, const char* szDeviceId, int iCameraNo, DefenceType enType, DefenceStatus enStatus, DefenceActor enActor, LP_NSCBMsg pNSCBMsg);

	typedef OPENSDK_RESULT(__stdcall *OpenSDK_GetAccessTokenSmsCode)(const char* szSignString);

	typedef OPENSDK_RESULT(__stdcall *OpenSDK_VerifyAccessTokenSmsCode)(const char* szSmsCode, const char* szUserId, const char* szPhoneNumber, const char* szAppKey);

	typedef OPENSDK_RESULT(__stdcall *OpenSDK_GetHdSignSmsCode)(const char* szAccessToken, const char* szSignString);

	typedef OPENSDK_RESULT(__stdcall *OpenSDK_VerifyHdSignSmsCode)(const char* szAccessToken, const char* szSmsCode, const char* szUserId, const char* szAppKey);

	typedef OPENSDK_RESULT(__stdcall *OpenSDK_UpdateCameraInfoToLocal)(const char* szCameraId, const char* szAccessToken, bool& isEncrypt);

	typedef OPENSDK_RESULT(__stdcall *OpenSDK_HttpSendWithWait)(const char* szUri, const char* szHeaderParam, const char* szBody, char** pBuf, int* iLength);

	struct tagNETSTREAMAPI
	{
		OpenSDK_InitLib              pOpenSDK_InitLib;
		OpenSDK_FiniLib              pOpenSDK_FiniLib;
		OpenSDK_AllocSession         pOpenSDK_AllocSession;
		OpenSDK_FreeSession          pOpenSDK_FreeSession;
		OpenSDK_StartRealPlay        pOpenSDK_StartRealPlay;
		OpenSDK_StopRealPlay         pOpenSDK_StopRealPlay;
		OpenSDK_StartPlayBack        pOpenSDK_StartPlayBack;
		OpenSDK_PlayBackResume       pOpenSDK_PlayBackResume;
		OpenSDK_PlayBackPause        pOpenSDK_PlayBackPause;
		OpenSDK_StopPlayBack         pOpenSDK_StopPlayBack;
		OpenSDK_SetDataCallBack      pOpenSDK_SetDataCallBack;
		OpenSDK_StartSearch          pOpenSDK_StartSearch;
		OpenSDK_GetOSDTime           pOpenSDK_GetOSDTime;

		OpenSDK_OpenSound            pOpenSDK_OpenSound;
		OpenSDK_CloseSound           pOpenSDK_CloseSound;
		OpenSDK_GetVolume            pOpenSDK_GetVolume;
		OpenSDK_SetVolume            pOpenSDK_SetVolume;
		OpenSDK_StartVoiceTalk       pOpenSDK_StartVoiceTalk;
		OpenSDK_StopVoiceTalk        pOpenSDK_StopVoiceTalk;
		OpenSDK_CapturePicture       pOpenSDK_CapturePicture;

		OpenSDK_Mid_Login            pOpenSDK_Mid_Login;
		OpenSDK_Mid_Device_Add       pOpenSDK_Mid_Device_Add;
		OpenSDK_Mid_Device_Oper      pOpenSDK_Mid_Device_Oper;

		OpenSDK_Data_GetDevList      pOpenSDK_Data_GetDevList;
		OpenSDK_Data_GetDeviceInfo   pOpenSDK_Data_GetDeviceInfo;
		OpenSDK_Data_GetAlarmList    pOpenSDK_Data_GetAlarmList;
		OpenSDK_Data_SetAlarmRead    pOpenSDK_Data_SetAlarmRead;
		OpenSDK_Data_DeleteDevice    pOpenSDK_Data_DeleteDevice;
		OpenSDK_Data_Free            pOpenSDK_Data_Free;

		OpenSDK_Alarm_SetMsgCallBack pOpenSDK_Alarm_SetMsgCallBack;
		OpenSDK_Alarm_StartRecv      pOpenSDK_Alarm_StartRecv;
		OpenSDK_Alarm_StopRecv       pOpenSDK_Alarm_StopRecv;

		OpenSDK_PTZCtrl                     pOpenSDK_PTZCtrl;
		OpenSDK_DevDefence                  pOpenSDK_DevDefence;
		OpenSDK_DevDefenceByDev             pOpenSDK_DevDefenceByDev;

		OpenSDK_GetAccessTokenSmsCode       pOpenSDK_GetAccessTokenSmsCode;
		OpenSDK_VerifyAccessTokenSmsCode    pOpenSDK_VerifyAccessTokenSmsCode;
		OpenSDK_GetHdSignSmsCode            pOpenSDK_GetHdSignSmsCode;
		OpenSDK_VerifyHdSignSmsCode         pOpenSDK_VerifyHdSignSmsCode;
		OpenSDK_HttpSendWithWait            pOpenSDK_HttpSendWithWait;
		OpenSDK_UpdateCameraInfoToLocal		pOpenSDK_UpdateCameraInfoToLocal;
	};
#pragma endregion

protected:
#pragma region CSdkMgrEzvizPrivate
	class CSdkMgrEzvizPrivate
	{
	private:
		HMODULE m_library;
		tagNETSTREAMAPI m_apis;
	public:
		int initLibrary(const std::string& authAddr, const std::string& platform, const std::string& appKey);
		int releaseLibrary();

		std::string oauth_login();
		int deviceAdd(const std::string& accessId);
		int deviceOper(const std::string& accessId, const std::string& deviceId);

		std::string allocSession(OpenSDK_MessageHandler handle, void* pUser, const bool bSync = false, const unsigned int iTimeOut = 0xffffffff);
		int freeSession(const std::string& session);

		int startRealPlay(const std::string& session, HWND hPlayWnd, const std::string& cameraId, const std::string& accessId, const std::string& safekey, const std::string& appKey, int videoLevel = 0, LP_NSCBMsg pNSCBMsg = nullptr);
		int stopRealPlay(const std::string& session, LP_NSCBMsg pNSCBMsg = nullptr);

		int startPlayBack(const std::string& session, HWND hPlayWnd, const std::string& cameraId, const std::string& accessId, const std::string& safekey, const std::string& startTime, const std::string& stopTime, const std::string& appKey, LP_NSCBMsg pNSCBMsg = nullptr);
		int playBackResume(const std::string& session);
		int playBackPause(const std::string& session);
		int stopPlayBack(const std::string& session, LP_NSCBMsg pNSCBMsg = nullptr);
		int setDataCallBack(const std::string& szSessionId, OpenSDK_DataCallBack pDataCallBack, void* pUser);

		int startSearch(const std::string& session, const std::string& cameraId, const std::string& accessId, const std::string& startTime, const std::string& stopTime, LP_NSCBMsg pNSCBMsg = nullptr);
		int getOSDTime(const std::string& session, STREAM_TIME* pTime);

		int getDevList(const std::string& accessId, int pageStart, int pageSize, void** pBuf, int* length);
		int getDevInfo(const std::string& accessId, const std::string& devSerial, void** pBuf, int* length);
		int getAlarmList(const std::string& accessId, const std::string& cameraId, const std::string& startTime, const std::string& endTime, int alarmType, int status, int pageStart, int pageSize, void** pBuf, int* length);
		int setAlarmRead(const std::string& accessId, const std::string& alarmId);
		int deleteDevice(const std::string& accessId, const std::string& deviceId);
		int freeData(void* pBuf);

		int openSound(const std::string& session);
		int closeSound(const std::string& session);
		int getVolume(const std::string& session);
		int setVolume(const std::string& session, unsigned short uVolume);
		int startVoiceTalk(const std::string& session, const std::string& accessId, const std::string& cameraId);
		int stopVoiceTalk(const std::string& session);
		int capturePicture(const std::string& session, const std::string& szFileName);
		int setAlarmMsgCallBack(OpenSDK_Alarm_MessageHandler handler, OpenSDK_Publish_MessageHandler publishHandler, void* pUser);
		int startAlarmRecv(const std::string& accessId);
		int stopAlarmRecv();
		int PTZCtrl(const std::string& szSessionId, const std::string& szAccessToken, const std::string& szCameraId, PTZCommand enCommand, PTZAction enAction, int iSpeed, LP_NSCBMsg pNSCBMsg = nullptr);
		int DevDefence(const std::string& szSessionId, const std::string& szAccessToken, const std::string& szCameraId, DefenceType enType, DefenceStatus enStatus, DefenceActor enActor, LP_NSCBMsg pNSCBMsg = nullptr);
		int DevDefenceByDev(const std::string& szSessionId, const std::string& szAccessToken, const std::string& szDeviceId, int iCameraNo, DefenceType enType, DefenceStatus enStatus, DefenceActor enActor, LP_NSCBMsg pNSCBMsg = nullptr);
		int DevDefence(const std::string& szSessionId, const std::string& szAccessToken, const std::string& szCameraId, DefenceType enType, DefenceStatus enStatus, DefenceActor enActor);
		int DevDefenceByDev(const std::string& szSessionId, const std::string& szAccessToken, const std::string& szDeviceId, int iCameraNo, DefenceType enType, DefenceStatus enStatus, DefenceActor enActor);
		int GetAccessTokenSmsCode(const std::string& szSignString);
		int VerifyAccessTokenSmsCode(const std::string& szSmsCode, const std::string& szUserId, const std::string& szPhoneNumber, const std::string& szAppKey);
		int GetHdSignSmsCode(const std::string& szAccessToken, const std::string& szSignString);
		int VerifyHdSignSmsCode(const std::string& szAccessToken, const std::string& szSmsCode, const std::string& szUserId, const std::string& szAppKey);
		int UpdateCameraInfo(const std::string& szCamera, const std::string& szAccessToken, bool& isEncrypt);
		int HttpSendWithWait(const char* szUri, const char* szHeaderParam, const char* szBody, char** pBuf, int* iLength);
	public:
		CSdkMgrEzvizPrivate();
		~CSdkMgrEzvizPrivate();
	};
#pragma endregion
	//std::string m_curSessionId;
	std::map<std::string, std::map<std::string, std::string>> _sessionMap;
public:
	CSdkMgrEzvizPrivate m_dll;
	bool Init(const std::string& appKey);
	bool GetUsersDeviceList(CVideoUserInfoEzviz* user, CVideoDeviceInfoEzvizList& devList);
	bool VerifyDeviceInfo(CVideoUserInfoEzviz* user, CVideoDeviceInfoEzviz* device);
	typedef enum SdkEzvizResult
	{
		RESULT_OK,
		RESULT_PRIVATE_CLOUD_CONNECT_FAILED_OR_USER_NOT_EXSIST,
	}SdkEzvizResult;
	SdkEzvizResult VerifyUserAccessToken(CVideoUserInfoEzviz* user);
	void FreeUserSession(const std::string& user_phone);
	std::string GetSessionId(const std::string& user_phone, const std::string& cameraId,
							 CSdkMgrEzviz::OpenSDK_MessageHandler messageHandler, void* data);
protected:
	

	
public:
	//COpenSdkMgr();
	~CSdkMgrEzviz();
	DECLARE_SINGLETON(CSdkMgrEzviz);
	DECLARE_UNCOPYABLE(CSdkMgrEzviz);
};


NAMESPACE_END
NAMESPACE_END
