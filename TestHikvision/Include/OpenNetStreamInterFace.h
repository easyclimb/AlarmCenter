/** \file      OpenNetStreamInterFace.h
 *  \copyright HangZhou Hikvision System Technology Co.,Ltd. All Right Reserved.
 *  \brief     开放SDK接口定义
 *
 *  \author    panlong, peter
 *  \date      2014/12/15
 *
 *  \note      history:
 *  \note      V0.0.1 2014/12/15 peter 修改文件注释 
 */

#ifndef _H_OPENNETSTREAMINTERFACE_H_
#define _H_OPENNETSTREAMINTERFACE_H_

#include "OpenNetStreamDefine.h"
#include "OpenNetStreamError.h"

#if defined (_WIN32) || defined(_WIN64)
#include <Windows.h>
#ifdef OPENNETSTREAM_EXPORTS
#  define OPENSDK_API __declspec(dllexport)
#else
#  define OPENSDK_API __declspec(dllimport)
#endif
#define _USE_HLOG_
#elif defined (OS_POSIX) || defined (__APPLE__) || defined(ANDROID) || defined (__linux__)
#include <cstdio>
#  define OPENSDK_API
#  define CALLBACK

#if defined(__linux__)
typedef unsigned int HWND;
#else
typedef void* HWND;
#endif

#else
#  error os not support!
#endif

#ifdef __cplusplus
extern "C" {
#endif

/** 
 *  \ingroup alloc
 *  \brief 信息回调
 *  \param szSessionId  申请的会话ID
 *  \param iMsgType     消息号 #MessageType
 *	\param iErrorCode   错误码 OpenNetStreamError.h
 *  \param pMessageInfo 信息
 *  \param pUser		用户自定义数据 
 *
 *  \note  
 *  当iMsgType == INS_RECORD_FILE时，pMessageInfo的格式如下
 *  \code {.json}
 *  {
 *		"FileSize":10, 
 *		"FileList": [
 *			{
 *				"StartTime":"",
 *				"EndTime":""
 *			}
 *		]
 *	}
 *  \endcode
 *  
 *  \sa OpenSDK_AllocSession()
 */
typedef void (CALLBACK *OpenSDK_MessageHandler)(const char* szSessionId, unsigned int iMsgType, unsigned int iErrorCode,const char *pMessageInfo, void *pUser);

/** \defgroup init 开放SDK初始化
 *  @{
 */

/** 
 *  \brief 初始化库
 *  \param[in] szAuthAddr 认证域名:开放平台认证中心地址，默认地址为：https://auth.ys7.com .对于开发者而言，请填写此默认地址即可. 
 *  \param[in] szPlatform 平台域名:开放平台后台地址，默认地址为：https://open.ys7.com .对于开发者而言，请填写此默认地址即可.
 *  \param[in] szAppId    向平台申请的AppKey
 *  \return 0表示成功，-1表示失败
 *  \sa OpenSDK_FiniLib()
 *
 *  \note
 *  该接口必须在调用其他接口之前调用
 */
OPENSDK_API OPENSDK_RESULT CALLBACK OpenSDK_InitLib(const char* szAuthAddr, const char* szPlatform, const char* szAppId);

/** 
 *  \brief  反初始化库
 *  \return 0表示成功，-1表示失败
 *  \sa OpenSDK_InitLib()
 */
OPENSDK_API OPENSDK_RESULT CALLBACK OpenSDK_FiniLib();

/** @} */ // end of init

/** \defgroup alloc 分配会话
 *  @{
 */

/** 
 *  \brief 申请一个会话Id
 *  \param[in]  pHandle     设置回调函数， #OpenSDK_MessageHandler
 *  \param[in]  pUser       用户自定义数据，会在pHandle中原样抛出
 *  \param[out] pSession    用于接收分配的会话ID
 *  \param[out] iSessionLen pSession的长度
 *	\param[in]  bSync		设置同步，默认为异步
 *	\param[in]	iTimeOut	同步等待时长， 默认0xEA60=1min
 *  \return 0表示成功，-1表示失败
 *  \sa OpenSDK_FreeSession()
 */
OPENSDK_API OPENSDK_RESULT CALLBACK OpenSDK_AllocSession(OpenSDK_MessageHandler pHandle, void* pUser, char** pSession, int* iSessionLen, const bool bSync = false,const unsigned int iTimeOut = 0xEA60);

/** 
 *  \brief 销毁SDK操作句柄
 *  \param[in] szSessionId 会话Id，通过OpenSDK_AllocSession()创建
 *  \return 0表示成功，-1表示失败
 *  \sa OpenSDK_AllocSession()
 */
OPENSDK_API OPENSDK_RESULT CALLBACK OpenSDK_FreeSession(const char* szSessionId);

/** @} */ // end of alloc

/** \defgroup play 预览接口
 *  @{
 */

/** 
 *  \brief 数据回调格式
 *  \param enType 数据类型，参见 #DataType
 *  \param pData  数据内容
 *  \param iLen   数据长度
 *  \param pUser  用户自定义数据
 */
typedef void (CALLBACK *OpenSDK_DataCallBack)(DataType enType, char* const pData, int iLen, void* pUser);

/** 
 *  \brief 设置数据回调
 *  \param[in] szSessionId   会话ID
 *  \param[in] pDataCallBack 回调函数
 *  \param[in] pUser         用户自定义数据，会通过pDataCallBack原样抛出
 *  \return 0表示成功，-1表示失败
 */
OPENSDK_API OPENSDK_RESULT CALLBACK OpenSDK_SetDataCallBack(const char* szSessionId, OpenSDK_DataCallBack pDataCallBack, void* pUser);

/** 
 *  \brief 预览
 *  \param[in] szSessionId   会话ID
 *  \param[in] hPlayWnd      播放窗口句柄 
 *  \param[in] szCameraId    监控点ID
 *  \param[in] szAccessToken 认证token
 *  \param[in] iVideoLevel   视频质量，输入范围在0-2之间
 *  \param[in] szSafeKey	 视频加密密钥 
 *  \param[in] szAppKey      从平台获取的AppKey
 *	\param[out] pNSCBMsg	 同步回调传出参数,如果为同步，不允许为NULL
 *  \return 0表示成功，非0表示失败
 *  \sa OpenSDK_StopRealPlay()
 */
OPENSDK_API OPENSDK_RESULT CALLBACK OpenSDK_StartRealPlay(const char* szSessionId, HWND hPlayWnd, const char* szCameraId, const char* szAccessToken, int iVideoLevel, const char* szSafeKey, const char* szAppKey, LP_NSCBMsg pNSCBMsg = NULL);

/** 
 *  \brief 停止预览
 *  \param[in] szSessionId 会话ID
 *	\param[out] pNSCBMsg   同步回调传出参数,如果为同步，不允许为NULL
 *  \return 0表示成功，非0表示失败
 *  \sa OpenSDK_StartRealPlay()
 */
OPENSDK_API OPENSDK_RESULT CALLBACK OpenSDK_StopRealPlay(const char* szSessionId, LP_NSCBMsg pNSCBMsg = NULL);

/** @} */ // end of play

/** \defgroup playback 回放接口
 *  @{
 */

/** 
 *  \brief 回放
 *  \param[in] szSessionId   会话Id
 *  \param[in] hPlayWnd      播放窗口句柄
 *  \param[in] szCameraId    监控点ID
 *  \param[in] szAccessToken 认证Token
 *  \param[in] szSafeKey     视频加密密钥
 *  \param[in] szStartTime   开始时间
 *  \param[in] szStopTime    停止时间
 *  \param[in] szAppKey      从平台获取的AppKey
 *	\param[out] pNSCBMsg	 同步回调传出参数,如果为同步，不允许为NULL
 *  \return 0表示成功，非0表示失败
 *  \sa OpenSDK_StopPlayBack()
 *  \note 
 *  时间格式为：yyyy-MM-dd hh:mm:ss
 */
OPENSDK_API OPENSDK_RESULT CALLBACK OpenSDK_StartPlayBack(const char* szSessionId, HWND hPlayWnd, const char* szCameraId, const char* szAccessToken, const char* szSafeKey, const char* szStartTime, const char* szStopTime, const char* szAppKey, LP_NSCBMsg pNSCBMsg = NULL);

/** 
 *  \brief 恢复回放
 *  \param[in] szSessionId 会话Id
 *  \return 0表示成功，非0表示失败
 *  \sa OpenSDK_PlayBackPause()
 *  \note
 *  该接口只能在OpenSDK_StartPlayBack()调用之后才能调用
 */
OPENSDK_API OPENSDK_RESULT CALLBACK OpenSDK_PlayBackResume(const char* szSessionId);

/** 
 *  \brief 暂停回放
 *  \param[in] szSessionId 会话Id
 *  \return 0表示成功，非0表示失败
 *	\sa OpenSDK_PlayBackResume()
 *  \note 
 *  该接口只能在OpenSDK_StartPlayBack() 调用之后才能调用
 */
OPENSDK_API OPENSDK_RESULT CALLBACK OpenSDK_PlayBackPause(const char* szSessionId);

/** 
 *  \brief 停止回放
 *  \param[in] szSessionId 会话Id
 *	\param[out] pNSCBMsg   同步回调传出参数,如果为同步，不允许为NULL
 *  \return 0表示成功，非0表示失败
 *	\sa OpenSDK_StartPlayBack()
 */
OPENSDK_API OPENSDK_RESULT CALLBACK OpenSDK_StopPlayBack(const char* szSessionId, LP_NSCBMsg pNSCBMsg = NULL);

/** 
 *  \brief 录像搜索，包含云存储和设备SD卡录像
 *  \param[in] szSessionId   会话Id
 *  \param[in] szCameraId    监控点Id
 *  \param[in] szAccessToken 认证Token
 *  \param[in] szStartTime   开始时间
 *  \param[in] szStopTime    停止时间 
 *	\param[out] pNSCBMsg	 同步回调传出参数,如果为同步，不允许为NULL
 *  \return 0表示成功，非0表示失败 
 *  \note
 *  时间格式为：yyyy-MM-dd hh:mm:ss
 */
OPENSDK_API OPENSDK_RESULT CALLBACK OpenSDK_StartSearch(const char* szSessionId, const char* szCameraId, const char* szAccessToken, const char* szStartTime, const char* szStopTime, LP_NSCBMsg pNSCBMsg = NULL);

/** @} */ // end of playback

/** \defgroup info 视频信息获取
 *  @{
 */

/** 
 *  \brief 获取OSD时间
 *  \param[in]  szSessionId 会话Id
 *  \param[out] pTime       OSD时间，格式见 #STREAM_TIME 
 *  \return 0表示成功，-1表示失败
 */
OPENSDK_API OPENSDK_RESULT CALLBACK OpenSDK_GetOSDTime(const char* szSessionId, STREAM_TIME *pTime);

/** @} */ // end of info

/** \defgroup sound 音频相关接口
 *  @{
 */

/** 
 *  \brief 打开声音
 *  \param[in] szSessionId 会话Id
 *  \return 0表示成功，-1表示失败
 *  \sa OpenSDK_CloseSound()
 */
OPENSDK_API OPENSDK_RESULT CALLBACK OpenSDK_OpenSound(const char* szSessionId);

/** 
 *  \brief 关闭声音
 *  \param[in] szSessionId 会话Id
 *  \return 0表示成功，-1表示失败
 *  \sa OpenSDK_OpenSound()
 */
OPENSDK_API OPENSDK_RESULT CALLBACK OpenSDK_CloseSound(const char* szSessionId);

/**
 *  \brief 获取音量
 *  \param[in] szSessionId 会话Id
 *  \return 小于0表示失败，其他值表示音量大小(0-100之间)
 *  \sa OpenSDK_SetVolume()
 */
OPENSDK_API unsigned short CALLBACK OpenSDK_GetVolume(const char* szSessionId);

/** 
 *  \brief 设置音量
 *  \param[in] szSessionId 会话Id
 *  \param[in] uVolume     音量大小，0-100之间
 *  \return 0表示成功，-1表示失败
 *  \sa OpenSDK_GetVolume()
 */
OPENSDK_API OPENSDK_RESULT CALLBACK OpenSDK_SetVolume(const char* szSessionId, unsigned short uVolume);

/** @} */ // end of sound

/** \defgroup voice_talk 语音对讲
 *  @{
 */

/** 
 *  \brief 开启语音对讲
 *  \param[in] szSessionId   会话Id
 *  \param[in] szAccessToken 认证Token
 *  \param[in] szCameraId    摄像头Id
 *  \return 0表示成功，-1表示失败
 *  \sa OpenSDK_StopVoiceTalk()
 */
OPENSDK_API OPENSDK_RESULT CALLBACK OpenSDK_StartVoiceTalk(const char* szSessionId, const char* szAccessToken, const char* szCameraId);

/** 
 *  \brief 结束语音对讲
 *  \param[in] szSessionId 会话Id
 *  \return 0表示成功，-1表示失败
 *  \sa OpenSDK_StartVoiceTalk()
 */
OPENSDK_API OPENSDK_RESULT CALLBACK OpenSDK_StopVoiceTalk(const char* szSessionId);

/** @} */ // end of voice_talk

/** 
 *  \ingroup info
 *  \brief 截屏
 *  \param[in] szSessionId 会话Id
 *  \param[in] szFileName  图片保存路径，格式为JPG
 *  \return 0表示成功，-1表示失败
 */
OPENSDK_API OPENSDK_RESULT CALLBACK OpenSDK_CapturePicture(const char* szSessionId, const char* szFileName);

/** \defgroup midpage 中间页
 *  @{
 */


/** 
 *  \brief 登陆接口
 *  \param[out] szAccessToken   认证Token
 *  \param[out] iAccessTokenLen 认证Token长度
 *  \return 0表示成功，-1表示失败
 *  \sa OpenSDK_Data_Free()，用于释放szAccessToken申请的内存
 */
OPENSDK_API int CALLBACK OpenSDK_Mid_Login( char** szAccessToken, int* iAccessTokenLen );

/** 
 *  \brief 设备添加
 *  \param[in] szAccessToken 认证Token
 *  \return 0表示成功，非0表示失败
 */
OPENSDK_API int CALLBACK OpenSDK_Mid_Device_Add(const char* szAccessToken);

/** 
 *  \brief 设备操作
 *  \param[in] szAccessToken 认证Token
 *  \param[in] szDeviceId    设备Id
 *  \return 0表示成功，-1表示失败
 */
OPENSDK_API int CALLBACK OpenSDK_Mid_Device_Oper(const char* szAccessToken, const char* szDeviceId);

/** @} */ // end of midpage

/** \defgroup data 数据接口
 *  @{
 */

/** 
 *  \brief 获取摄像头列表
 *  \param[in]  szAccessToken 认证Token
 *  \param[in]  iPageStart    分页起始页，从0开始
 *  \param[in]  iPageSize     分页大小
 *  \param[out] pBuf          摄像头列表的JSON字符串
 *  \param[out] iLength       获取到的数据大小
 *  \return 0表示成功，-1表示失败 
 *
 *  \note 
 *  摄像头列表的JSON格式如下
 *  \code {.json}
 *	{
 *		"resultCode":"200", 
 *		"count":num, 
 *		"cameraList":[
 *			{
 *				"deviceId":"", "cameraId":"", "cameraNo":"", "cameraName":"", "display":"", "status":"", "isShared":"", "picUrl":"", "isEncrypt":0; 
 *			}
 *		]
 *	}
 *  \endcode
 */
OPENSDK_API OPENSDK_RESULT CALLBACK OpenSDK_Data_GetDevList( const char* szAccessToken, int iPageStart, int iPageSize, void** pBuf, int* iLength);

/** 
 *  \brief 获取单个设备信息
 *  \param[in] szDeviceSerial 设备序列号
 *  \param[out] pBuf          设备信息的JSON字符串
 *  \param[out] iLength       获取到的数据大小
 *  \return 0表示成功，非0表示失败
 *
 *  \note 
 *  设备信息的JSON格式如下
 *  \code {.json}
 *	{
 *		"result": {
 *			"data": [
 *				{
 *					"deviceId": "", "deviceSerial": "", "cameraId": "", "cameraNo": 1, "cameraName": "", "status": 1, "isShared": "0", "picUrl": "", "isEncrypt": 0
 *				}
 *			],
 *			"code": "200",
 *			"msg": "操作成功!"
 *		}
 *	}
 *  \endcode
 */
OPENSDK_API OPENSDK_RESULT CALLBACK OpenSDK_Data_GetDeviceInfo(const char* szAccessToken, const char* szDeviceSerial, void** pBuf, int* iLength);

/** 
 *  \brief 获取报警列表
 *  \param[in]  szAccessToken 认证Token
 *  \param[in]  szCameraId    监控点Id
 *  \param[in]  szStartTime   开始时间
 *  \param[in]  szEndTime     结束时间
 *  \param[in]  iAlarmType    报警类型，参见 #AlarmType
 *  \param[in]  iStatus       报警状态，0表示未读，1表示已读，2表示所有
 *  \param[in]  iPageStart    分页起始页，从0开始
 *  \param[in]  iPageSize     分页大小
 *  \param[out] pBuf		  报警信息列表
 *  \param[out] iLength       报警信息列表长度
 *  \return 0表示成功，-1表示失败
 *  
 *  \note 
 *  时间格式为：yyyy-MM-dd hh:mm:ss
 *
 *  \note 
 *  报警信息列表JSON格式如下：
 *  \code {.json}
 *  {
 *		"resultCode":"200", 
 *		"count":2, 
 *		"alarmList":[
 *			{	
 *				"alarmId":"", "objectName":"", "alarmType":"", "alarmStart":"" 
 *			}
 *		]
 *	}
 *  \endcode
 */
OPENSDK_API OPENSDK_RESULT CALLBACK OpenSDK_Data_GetAlarmList( const char* szAccessToken, const char* szCameraId, const char* szStartTime, const char* szEndTime, AlarmType iAlarmType, int iStatus, int iPageStart, int iPageSize, void** pBuf, int* iLength);

/** 
 *  \brief 更新监控点详细信息到缓存
 *  \param[in] szCameraId		监控点ID
 *  \param[in] szAccessToken	认证Token
 *	\param[out] isEncrypt		监控点设备是否加密
 *  \return 0表示成功，其他值表示失败
 */
OPENSDK_API OPENSDK_RESULT CALLBACK OpenSDK_UpdateCameraInfoToLocal(const char* szCameraId, const char* szAccessToken, bool& isEncrypt);


/** 
 *  \brief 设置报警已读
 *  \param[in] szAlarmId 报警ID
 *  \return 0表示成功，非0表示失败
 */
OPENSDK_API OPENSDK_RESULT CALLBACK OpenSDK_Data_SetAlarmRead(const char* szAccessToken, const char* szAlarmId);

/** 
 *  \brief 删除设备
 *  \param[in] szAccessToken 认证Token
 *  \param[in] szDeviceId    设备Id
 *  \return 0表示成功，非0表示失败
 */
OPENSDK_API OPENSDK_RESULT CALLBACK OpenSDK_Data_DeleteDevice( const char* szAccessToken, const char* szDeviceId);

/** 
 *  \brief 销毁SDK分配的内存
 *  \param[in] pBuf SDK分配的内存
 *  \return 0表示成功，-1表示失败
 */
OPENSDK_API OPENSDK_RESULT CALLBACK OpenSDK_Data_Free(void* pBuf);

/** @} */ // end of data

/** \defgroup alaram 报警相关接口
 *  @{
 */

/** 
 *  \brief 报警消息推送回调函数
 *  \param szCameraId  监控点Id
 *  \param szContent   报警推送内容
 *  \param szAlarmTime 报警推送时间
 *  \param pUser       用户自定义数据
 *  \sa OpenSDK_Alarm_SetMsgCallBack()
 *
 *  \note 
 *  报警推送内容JSON格式如下：
 *  \code {.json}
 *	{
 *		"alarmID": "11",						// 报警ID
 *		"alarmType": "11",						// 报警类型
 *		"deviceSeril": "231213",				// 设备序列号
 *		"alarmTime": "2012-12-25 15:25:42",		// 报警时间
 *		"channelID": "1",						// 通道ID
 *		"channelName": "通道1",					// 通道名称
 *		"picUrl": "http://www.hikvision.com/",	// 图片URL
 *		"content":"string"						// 消息发送的内容
 *	}
 *  \endcode
 */
typedef void (CALLBACK *OpenSDK_Alarm_MessageHandler)(const char* szCameraId, const char* szContent, const char* szAlarmTime, void* pUser);

/** 
 *  \brief 推送消息回调函数, 包括报警、布撤防、用户信息变更、设备删除、
 *							设备权限变更、设备上下线、用户（取消）授权服务消息
 *  \param szContent   消息推送内容
 *  \param pUser       用户自定义数据
 *  \sa OpenSDK_Publish_SetMsgCallBack()
 *
 *  \note 
 *  消息推送内容JSON格式如下：
 *  \code {.json}
 *	{
 *		"type": "message",						// 推送类型(不变)
 *		"messageType": "A100",						// 推送类型号(不变)
 *		"destination": "dest",					// 推送对象
 *		"MsgType": "1",							// 推送的消息类型
 *		"data":{								// 推送的消息内容
 *			... ,
 *			...
 *		}
 *	}
 *  \endcode
 */
typedef void (CALLBACK *OpenSDK_Publish_MessageHandler)(const char* szContent, void* pUser);

/** 
 *  \brief 设置告警推送
 *  \param[in] handle			告警推送函数
 *	\param[in] publishHandle	推送消息函数
 *  \param[in] pUser			用户自定义数据，回调函数会原样抛出
 *  \return 0表示成功，-1表示失败
 *  \sa OpenSDK_Alarm_MessageHandler()
 */
OPENSDK_API OPENSDK_RESULT CALLBACK OpenSDK_Alarm_SetMsgCallBack(OpenSDK_Alarm_MessageHandler alarmHandle, OpenSDK_Publish_MessageHandler publishHandle, void* pUser);

/** 
 *  \brief 开始接受告警
 *  \param[in] szAccessToken 认证token
 *  \return 0表示成功，-1表示失败
 *  \sa OpenSDK_Alarm_StopRecv()
 */
OPENSDK_API OPENSDK_RESULT CALLBACK OpenSDK_Alarm_StartRecv(const char* szAccessToken);

/** 
 *  \brief 停止接收告警
 *  \return 0表示成功，-1表示失败
 *  \sa OpenSDK_Alarm_StartRecv()
 */
OPENSDK_API OPENSDK_RESULT CALLBACK OpenSDK_Alarm_StopRecv();

/** @} */ // end of alaram

/** \defgroup device 设备操作相关接口
 *  @{
 */

/**
 *  \brief 云台控制
 *  \param[in] szSessionId   会话ID
 *  \param[in] szAccessToken 认证Token
 *  \param[in] szCameraId	 监控点Id
 *  \param[in] enCommand	 云台控制命令，参见 #PTZCommand
 *  \param[in] enAction		 云台操作命令，参见 #PTZAction
 *  \param[in] iSpeed		 速度0-7之间
 *	\param[out] pNSCBMsg	 同步回调传出参数,如果为同步，不允许为NULL
 *  \return 0表示成功，-1表示失败
 */
OPENSDK_API OPENSDK_RESULT CALLBACK OpenSDK_PTZCtrl(const char* szSessionId, const char* szAccessToken, const char* szCameraId, PTZCommand enCommand, PTZAction enAction, int iSpeed, LP_NSCBMsg pNSCBMsg = NULL);

/** 
 *  \brief 按监控点ID进行布撤防
 *  \param[in] szSessionId   会话Id
 *  \param[in] szAccessToken 认证Token
 *  \param[in] szCameraId    监控点Id
 *  \param[in] enType        布撤防类型
 *  \param[in] enStatus      布撤防状态
 *  \param[in] enActor		 布撤防设备类型
 *	\param[out] pNSCBMsg	 同步回调传出参数,如果为同步，不允许为NULL
 *  \return 非负表示成功，-1表示失败
 */
OPENSDK_API OPENSDK_RESULT CALLBACK OpenSDK_DevDefence(const char* szSessionId, const char* szAccessToken, const char* szCameraId, DefenceType enType, DefenceStatus enStatus, DefenceActor enActor, LP_NSCBMsg pNSCBMsg = NULL);

/** 
 *  \brief 按设备ID和通道号进行布撤防
 *  \param[in] szSessionId   会话Id
 *  \param[in] szAccessToken 认证Token
 *  \param[in] szDeviceId    设备Id
 *  \param[in] iCameraNo     通道号，操作设备本身通道号为0
 *  \param[in] enType        布撤防类型
 *  \param[in] enStatus      布撤防状态
 *  \param[in] enActor		 布撤防设备类型
 *	\param[out] pNSCBMsg	 同步回调传出参数,如果为同步，不允许为NULL
 *  \return 非负表示成功，-1表示失败
 */
OPENSDK_API OPENSDK_RESULT CALLBACK OpenSDK_DevDefenceByDev(const char* szSessionId, const char* szAccessToken, const char* szDeviceId, int iCameraNo, DefenceType enType, DefenceStatus enStatus, DefenceActor enActor, LP_NSCBMsg pNSCBMsg = NULL);

/** @} */ // end of device

/** \defgroup sms_code 第三方短信验证码相关接口
 *  @{
 */

/** 
 *  \brief 获取AccessToken短信验证码
 *  \param[in]  szSignString 签名字符串 参照萤石平台接口使用说明文档
 *  \return 0表示成功，其他值表示失败
 */
OPENSDK_API OPENSDK_RESULT CALLBACK OpenSDK_GetAccessTokenSmsCode(const char* szSignString);

/** 
 *  \brief 验证AccessToken短信验证码
 *  \param[in] szSmsCode     短信验证码
 *  \param[in] szUserId      用户Id
 *  \param[in] szPhoneNumber 用户手机号码
 *  \param[in] szAppKey      从平台获取的AppKey
 *  \return 0表示成功，其他值表示失败
 */
OPENSDK_API OPENSDK_RESULT CALLBACK OpenSDK_VerifyAccessTokenSmsCode(const char* szSmsCode, const char* szUserId, const char* szPhoneNumber, const char* szAppKey);

/** 
 *  \brief 获取硬件特征码短信验证码
 *  \param[in]  szAccessToken 认证Token
 *  \param[in]  szSignString  签名字符串 参照萤石平台接口使用说明文档
 *  \return 0表示成功，其他值表示失败
 */
OPENSDK_API OPENSDK_RESULT CALLBACK OpenSDK_GetHdSignSmsCode(const char* szAccessToken, const char* szSignString);

/** 
 *  \brief 验证硬件特征码短信验证码
 *  \param[in] szAccessToken 认证Token
 *  \param[in] szSmsCode     短信验证码
 *  \param[in] szUserId      用户Id
 *  \param[in] szAppKey      从平台获取的AppKey
 *  \return 0表示成功，其他值表示失败
 */
OPENSDK_API OPENSDK_RESULT CALLBACK OpenSDK_VerifyHdSignSmsCode(const char* szAccessToken, const char* szSmsCode, const char* szUserId, const char* szAppKey);

/** @} */ // end of sms_code

/** \defgroup util_tool 通用工具接口
*  @{
*/

/** 
 *  \brief Http请求接口
 *  \param[in] szUri			请求地址
 *  \param[in] szHeaderParam	头部参数
 *  \param[in] szBody			Body数据
 *  \param[in] pBuf				返回报文的内容
 *  \param[in] iLength			返回报文的长度
 *  \return 0表示成功，其他值表示失败
 */
OPENSDK_API OPENSDK_RESULT CALLBACK OpenSDK_HttpSendWithWait(const char* szUri, const char* szHeaderParam, const char* szBody, char** pBuf, int* iLength);

/** 
 *  \brief 方法透传,直接向平台发出请求
 *  \param[in]  reqStr		发给开发平台的请求
 *  \param[out] pBuf		平方平台响应消息
 *	\param[out] iLength		pBuf的长度
 *  \return 0表示成功，其他值表示失败
 */
OPENSDK_API OPENSDK_RESULT CALLBACK OpenSDK_RequestPassThrough(const char* reqStr, char** pBuf, int* iLength);
/** @} */ // end of util_tool

#ifdef __cplusplus
}
#endif

#endif