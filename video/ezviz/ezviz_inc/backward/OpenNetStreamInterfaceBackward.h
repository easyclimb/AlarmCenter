/** \file      OpenNetStreamInterfaceBackward.h
 *  \copyright HangZhou Hikvision System Technology Co.,Ltd. All Right Reserved.
 *  \brief     开放SDK向后兼容接口定义
 *
 *  \author    pikongxuan
 *  \date      2015/07/31
 *
 *  \note      history:
 *  \note      V2.4.1 2015/07/31 
 */

#ifndef _H_OPENNETSTREAMINTERFACE_BACKWARD_H_
#define _H_OPENNETSTREAMINTERFACE_BACKWARD_H_

#ifdef __GNUC__
#define DEPRECATED(func)  func __attribute__ ((deprecated))
#elif defined(_MSC_VER)
#define DEPRECATED(func) __declspec(deprecated) func
#endif


#include "OpenNetStreamBackwardWarning.h"

#ifdef __cplusplus
extern "C" {
#endif


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
 *  \return 0表示成功，-1表示失败
 *  \sa OpenSDK_FreeSession()
 */
DEPRECATED(OPENSDK_API OPENSDK_RESULT CALLBACK OpenSDK_AllocSession_Old(OpenSDK_MessageHandler pHandle, void* pUser, char** pSession, int* iSessionLen));

/** @} */ // end of alloc

/** \defgroup play 预览接口
 *  @{
 */

/** 
 *  \brief 预览
 *  \param[in] szSessionId   会话ID
 *  \param[in] hPlayWnd      播放窗口句柄 
 *  \param[in] szCameraId    监控点ID
 *  \param[in] szAccessToken 认证token
 *  \param[in] iVideoLevel   视频质量，输入范围在0-2之间
 *  \param[in] szSafeKey	 视频加密密钥 
 *  \param[in] szAppKey      从平台获取的AppKey
 *  \return 0表示成功，非0表示失败
 *  \sa OpenSDK_StopRealPlay()
 */
DEPRECATED(OPENSDK_API OPENSDK_RESULT CALLBACK OpenSDK_StartRealPlay_Old(const char* szSessionId, HWND hPlayWnd, const char* szCameraId, const char* szAccessToken, int iVideoLevel, const char* szSafeKey, const char* szAppKey));

/** 
 *  \brief 停止预览
 *  \param[in] szSessionId 会话ID
 *  \return 0表示成功，非0表示失败
 *  \sa OpenSDK_StartRealPlay()
 */
DEPRECATED(OPENSDK_API OPENSDK_RESULT CALLBACK OpenSDK_StopRealPlay_Old(const char* szSessionId));

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
 *  \return 0表示成功，非0表示失败
 *  \sa OpenSDK_StopPlayBack()
 *  \note 
 *  时间格式为：yyyy-MM-dd hh:mm:ss
 */
DEPRECATED(OPENSDK_API OPENSDK_RESULT CALLBACK OpenSDK_StartPlayBack_Old(const char* szSessionId, HWND hPlayWnd, const char* szCameraId, const char* szAccessToken, const char* szSafeKey, const char* szStartTime, const char* szStopTime, const char* szAppKey));

/** 
 *  \brief 停止回放
 *  \param[in] szSessionId 会话Id
 *  \return 0表示成功，非0表示失败
 *	\sa OpenSDK_StartPlayBack()
 */
DEPRECATED(OPENSDK_API OPENSDK_RESULT CALLBACK OpenSDK_StopPlayBack_Old(const char* szSessionId));

/** 
 *  \brief 录像搜索，包含云存储和设备SD卡录像
 *  \param[in] szSessionId   会话Id
 *  \param[in] szCameraId    监控点Id
 *  \param[in] szAccessToken 认证Token
 *  \param[in] szStartTime   开始时间
 *  \param[in] szStopTime    停止时间 
 *  \return 0表示成功，非0表示失败 
 *  \note
 *  时间格式为：yyyy-MM-dd hh:mm:ss
 */
DEPRECATED(OPENSDK_API OPENSDK_RESULT CALLBACK OpenSDK_StartSearch_Old(const char* szSessionId, const char* szCameraId, const char* szAccessToken, const char* szStartTime, const char* szStopTime));

/** @} */ // end of playback

/** \defgroup alaram 报警相关接口
 *  @{
 */

/** 
 *  \brief 设置告警推送
 *  \param[in] handle			告警推送函数
 *  \param[in] pUser			用户自定义数据，回调函数会原样抛出
 *  \return 0表示成功，-1表示失败
 *  \sa OpenSDK_Alarm_MessageHandler()
 */
DEPRECATED(OPENSDK_API OPENSDK_RESULT CALLBACK OpenSDK_Alarm_SetMsgCallBack_Old(OpenSDK_Alarm_MessageHandler alarmHandle, void* pUser));

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
 *  \return 0表示成功，-1表示失败
 */
DEPRECATED(OPENSDK_API OPENSDK_RESULT CALLBACK OpenSDK_PTZCtrl_Old(const char* szSessionId, const char* szAccessToken, const char* szCameraId, PTZCommand enCommand, PTZAction enAction, int iSpeed));

/** 
 *  \brief 按监控点ID进行布撤防
 *  \param[in] szSessionId   会话Id
 *  \param[in] szAccessToken 认证Token
 *  \param[in] szCameraId    监控点Id
 *  \param[in] enType        布撤防类型
 *  \param[in] enStatus      布撤防状态
 *  \param[in] enActor		 布撤防设备类型
 *  \return 非负表示成功，-1表示失败
 */
DEPRECATED(OPENSDK_API OPENSDK_RESULT CALLBACK OpenSDK_DevDefence_Old(const char* szSessionId, const char* szAccessToken, const char* szCameraId, DefenceType enType, DefenceStatus enStatus, DefenceActor enActor));

/** 
 *  \brief 按设备ID和通道号进行布撤防
 *  \param[in] szSessionId   会话Id
 *  \param[in] szAccessToken 认证Token
 *  \param[in] szDeviceId    设备Id
 *  \param[in] iCameraNo     通道号，操作设备本身通道号为0
 *  \param[in] enType        布撤防类型
 *  \param[in] enStatus      布撤防状态
 *  \param[in] enActor		 布撤防设备类型
 *  \return 非负表示成功，-1表示失败
 */
DEPRECATED(OPENSDK_API OPENSDK_RESULT CALLBACK OpenSDK_DevDefenceByDev_Old(const char* szSessionId, const char* szAccessToken, const char* szDeviceId, int iCameraNo, DefenceType enType, DefenceStatus enStatus, DefenceActor enActor));

/** @} */ // end of device

#ifdef __cplusplus
}
#endif

#endif