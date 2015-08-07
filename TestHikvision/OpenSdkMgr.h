#pragma once


typedef struct _STREAM_TIME
{
	unsigned int iYear;		//年
	unsigned int iMonth;	//月
	unsigned int iDay;		//日
	unsigned int iHour;		//时
	unsigned int iMinute;	//分
	unsigned int iSecond;	//秒
}STREAM_TIME, *LP_STREAM_TIME;

/** \struct NetStreamCallbackMessage
*  \brief 经过NetStream处理所反馈的信息
*/
typedef struct _NetStreamCallBackMessage
{
public:
	_NetStreamCallBackMessage(void) :
		iErrorCode(0), pMessageInfo(NULL)
	{}
	unsigned int iErrorCode;
	char *pMessageInfo;
} NSCBMsg, *LP_NSCBMsg;

enum MessageType
{
	INS_PLAY_EXCEPTION = 0,   // 播放异常，通常是设备断线或网络异常造成
	INS_PLAY_RECONNECT = 1,   // 重连，实时流播放时内部会自动重连
	INS_PLAY_RECONNECT_EXCEPTION = 2,   // 重连异常
	INS_PLAY_START = 3,   // 播放开始
	INS_PLAY_STOP = 4,   // 播放终止
	INS_PLAY_ARCHIVE_END = 5,   // 播放结束，回放结束时会有此消息

	INS_RECORD_FILE = 20,  // 查询的录像文件(录像搜索结果)
	INS_RECORD_SEARCH_END = 21,  // 录像查询结束（暂不使用）
	INS_RECORD_SEARCH_FAILED = 22,  // 录像查询失败
};

/** \enum  DataType
*  \brief 音视频流数据类型
*/
enum DataType
{
	NET_DVR_SYSHEAD = 1,      ///< 流头
	NET_DVR_STREAMDATA = 2,		///< 流数据
	NET_DVR_RECV_END = 3,      ///< 结束标记
};

/** \enum PTZCommand
*  \brief 云台控制命令
*/
enum PTZCommand
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
};

/** \enum PTZAction
*  \brief 云台操作命令
*/
enum PTZAction
{
	START,
	STOP
};

/** \enum  DefenceType
*  \brief 布撤防报警类型
*/
enum DefenceType
{
	PIR,							///< 红外
	ATHOME,                         ///< 在家，A1设备
	OUTDOOR,					    ///< 外出
	BABYCRY,						///< 婴儿啼哭
	MOTIONDETECT,					///< 移动侦测
	GLOBAL							///< 全部
};

/** \enum  DefenceStatus
*  \brief 布撤防状态
*/
enum DefenceStatus
{
	UN_DEFENCE,						///< 撤防
	DEFENCE,						///< 布防
	UN_SUPPORT,						///< 不支持
	FORCE_DEFENCE,					///< 强制布防，A1设备
};

enum DefenceActor
{
	D,								///< 设备
	V,								///< 视频通道
	I,								///< IO通道
};

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
typedef void(__stdcall *OpenSDK_MessageHandler)(const char* szSessionId, unsigned int iMsgType, unsigned int iErrorCode, const char *pMessageInfo, void *pUser);

/**
*  \brief 报警消息推送回调函数
*  \param szCameraId  监控点Id
*  \param szContent   报警推送内容
*  \param szAlarmTime 报警推送时间
*  \param pUser       用户自定义数据
*  \sa OpenSDK_Alarm_SetMsgCallBack()
*/
typedef void(__stdcall *OpenSDK_Alarm_MessageHandler)(const char* szCameraId, const char* szContent, const char* szAlarmTime, void* pUser);

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
typedef void(__stdcall *OpenSDK_Publish_MessageHandler)(const char* szContent, void* pUser);

/**
*  \brief 数据回调格式
*  \param enType 数据类型，参见 #DataType
*  \param pData  数据内容
*  \param iLen   数据长度
*  \param pUser  用户自定义数据
*/
typedef void(__stdcall *OpenSDK_DataCallBack)(DataType enType, char* const pData, int iLen, void* pUser);


typedef int OPENSDK_RESULT;

/**
*  \brief 初始化库
*  \param[in] szAuthAddr 认证域名
*  \param[in] szPlatform 平台域名
*  \param[in] szAppId    向平台申请的AppId
*  \return 0表示成功，-1表示失败
*  \sa OpenSDK_FiniLib()
*
*  \note
*  该接口必须在调用其他接口之前调用
*/
typedef OPENSDK_RESULT(__stdcall *OpenSDK_InitLib)(const char* szAuthAddr, const char* szPlatform, const char* szAppId);

/**
*  \brief  反初始化库
*  \return 0表示成功，-1表示失败
*  \sa OpenSDK_InitLib()
*/
typedef OPENSDK_RESULT(__stdcall *OpenSDK_FiniLib)();

/**
*  \brief 申请一个会话Id
*  \param[in]  pHandle     设置回调函数， #OpenSDK_MessageHandler
*  \param[in]  pUser       用户自定义数据，会在pHandle中原样抛出
*  \param[out] pSession    用于接收分配的会话ID
*  \param[out] iSessionLen pSession的长度
*  \return 0表示成功，-1表示失败
*  \sa OpenSDK_FreeSession()
*/
typedef OPENSDK_RESULT(__stdcall *OpenSDK_AllocSession)(OpenSDK_MessageHandler pHandle, void* pUser, char** pSession, int* iSessionLen, const bool bSync, const unsigned int iTimeOut);

/**
*  \brief 销毁SDK操作句柄
*  \param[in] szSessionId 会话Id，通过OpenSDK_AllocSession()创建
*  \return 0表示成功，-1表示失败
*  \sa OpenSDK_AllocSession()
*/
typedef OPENSDK_RESULT(__stdcall *OpenSDK_FreeSession)(const char* szSessionId);

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
typedef OPENSDK_RESULT(__stdcall *OpenSDK_StartRealPlay)(const char* szSessionId, HWND hPlayWnd, const char* szCameraId, const char* szAccessToken, int iVideoLevel, const char* szSafeKey, const char* szAppKey, LP_NSCBMsg pNSCBMsg);

/**
*  \brief 停止预览
*  \param[in] szSessionId 会话ID
*  \return 0表示成功，非0表示失败
*  \sa OpenSDK_StartRealPlay()
*/
typedef OPENSDK_RESULT(__stdcall *OpenSDK_StopRealPlay)(const char* szSessionId, LP_NSCBMsg pNSCBMsg);

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
typedef OPENSDK_RESULT(__stdcall *OpenSDK_StartPlayBack)(const char* szSessionId, HWND hPlayWnd, const char* szCameraId, const char* szAccessToken, const char* szSafeKey, const char* szStartTime, const char* szStopTime, const char* szAppKey, LP_NSCBMsg pNSCBMsg);

/**
*  \brief 恢复回放
*  \param[in] szSessionId 会话Id
*  \return 0表示成功，非0表示失败
*  \sa OpenSDK_PlayBackPause()
*  \note
*  该接口只能在OpenSDK_StartPlayBack()调用之后才能调用
*/
typedef OPENSDK_RESULT(__stdcall *OpenSDK_PlayBackResume)(const char* szSessionId);

/**
*  \brief 暂停回放
*  \param[in] szSessionId 会话Id
*  \return 0表示成功，非0表示失败
*	\sa OpenSDK_PlayBackResume()
*  \note
*  该接口只能在OpenSDK_StartPlayBack() 调用之后才能调用
*/
typedef OPENSDK_RESULT(__stdcall *OpenSDK_PlayBackPause)(const char* szSessionId);

/**
*  \brief 停止回放
*  \param[in] szSessionId 会话Id
*  \return 0表示成功，非0表示失败
*	\sa OpenSDK_StartPlayBack()
*/
typedef OPENSDK_RESULT(__stdcall *OpenSDK_StopPlayBack)(const char* szSessionId, LP_NSCBMsg pNSCBMsg);

/**
*  \brief 设置数据回调
*  \param[in] szSessionId   会话ID
*  \param[in] pDataCallBack 回调函数
*  \param[in] pUser         用户自定义数据，会通过pDataCallBack原样抛出
*  \return 0表示成功，-1表示失败
*/
typedef OPENSDK_RESULT(__stdcall *OpenSDK_SetDataCallBack)(const char* szSessionId, OpenSDK_DataCallBack pDataCallBack, void* pUser);

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
typedef OPENSDK_RESULT(__stdcall *OpenSDK_StartSearch)(const char* szSessionId, const char* szCameraId, const char* szAccessId, const char* szStartTime, const char* szStopTime, LP_NSCBMsg pNSCBMsg);

/**
*  \brief 获取OSD时间
*  \param[in]  szSessionId 会话Id
*  \param[out] pTime       OSD时间，格式见 #STREAM_TIME
*  \return 0表示成功，-1表示失败
*/
typedef OPENSDK_RESULT(__stdcall *OpenSDK_GetOSDTime)(const char* szSessionId, STREAM_TIME *pTime);

/**
*  \brief 打开声音
*  \param[in] szSessionId 会话Id
*  \return 0表示成功，-1表示失败
*  \sa OpenSDK_CloseSound()
*/
typedef OPENSDK_RESULT(__stdcall *OpenSDK_OpenSound)(const char* szSessionId);

/**
*  \brief 关闭声音
*  \param[in] szSessionId 会话Id
*  \return 0表示成功，-1表示失败
*  \sa OpenSDK_OpenSound()
*/
typedef OPENSDK_RESULT(__stdcall *OpenSDK_CloseSound)(const char* szSessionId);

/**
*  \brief 获取音量
*  \param[in] szSessionId 会话Id
*  \return 小于0表示失败，其他值表示音量大小(0-100之间)
*  \sa OpenSDK_SetVolume()
*/
typedef unsigned short(__stdcall *OpenSDK_GetVolume)(const char* szSessionId);

/**
*  \brief 设置音量
*  \param[in] szSessionId 会话Id
*  \param[in] uVolume     音量大小，0-100之间
*  \return 0表示成功，-1表示失败
*  \sa OpenSDK_GetVolume()
*/
typedef OPENSDK_RESULT(__stdcall *OpenSDK_SetVolume)(const char* szSessionId, unsigned short uVolume);

/**
*  \brief 开启语音对讲
*  \param[in] szSessionId   会话Id
*  \param[in] szAccessToken 认证Token
*  \param[in] szCameraId    摄像头Id
*  \return 0表示成功，-1表示失败
*  \sa OpenSDK_StopVoiceTalk()
*/
typedef OPENSDK_RESULT(__stdcall *OpenSDK_StartVoiceTalk)(const char* szSessionId, const char* szAccessId, const char* szCameraId);

/**
*  \brief 结束语音对讲
*  \param[in] szSessionId 会话Id
*  \return 0表示成功，-1表示失败
*  \sa OpenSDK_StartVoiceTalk()
*/
typedef OPENSDK_RESULT(__stdcall *OpenSDK_StopVoiceTalk)(const char* szSessionId);

/**
*  \ingroup info
*  \brief 截屏
*  \param[in] szSessionId 会话Id
*  \param[in] szFileName  图片保存路径，格式为JPG
*  \return 0表示成功，-1表示失败
*/
typedef OPENSDK_RESULT(__stdcall *OpenSDK_CapturePicture)(const char* szSessionId, const char* szFileName);

/**
*  \brief 登陆接口
*  \param[out] szAccessToken   认证Token
*  \param[out] iAccessTokenLen 认证Token长度
*  \return 0表示成功，-1表示失败
*  \sa OpenSDK_Data_Free()，用于释放szAccessToken申请的内存
*/
typedef OPENSDK_RESULT(__stdcall *OpenSDK_Mid_Login)(char** pAccessToken, int* iAccessTokenLen);

/**
*  \brief 设备添加
*  \param[in] szAccessToken 认证Token
*  \return 0表示成功，-1表示失败
*/
typedef int(__stdcall *OpenSDK_Mid_Device_Add)(const char* szStrAccessToken);

/**
*  \brief 设备操作
*  \param[in] szAccessToken 认证Token
*  \param[in] szDeviceId    设备Id
*  \return 0表示成功，-1表示失败
*/
typedef int(__stdcall *OpenSDK_Mid_Device_Oper)(const char* szStrAccessToken, const char* szDeviceId);

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
typedef OPENSDK_RESULT(__stdcall *OpenSDK_Data_GetDevList)(const char* szAccessId, int iPageStart, int iPageSize, void** pBuf, int* iLength);

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
typedef OPENSDK_RESULT(__stdcall *OpenSDK_Data_GetDeviceInfo)(const char* szAccessToken, const char* szDeviceSerial, void** pBuf, int* iLength);


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
typedef OPENSDK_RESULT(__stdcall *OpenSDK_Data_GetAlarmList)(const char* szAccessId, const char* szCameraId, const char* szStartTime, const char* szEndTime, int iAlarmType, int iStatus, int iPageStart, int iPageSize, void** pBuf, int* iLength);

/**
*  \brief 设置报警已读
*  \param[in] szAlarmId 报警ID
*  \return 0表示成功，非0表示失败
*/
typedef OPENSDK_RESULT(__stdcall *OpenSDK_Data_SetAlarmRead)(const char* szAccessToken, const char* szAlarmId);

/**
*  \brief 删除设备
*  \param[in] szAccessToken 认证Token
*  \param[in] szDeviceId    设备Id
*  \return 0表示成功，-1表示失败
*/
typedef OPENSDK_RESULT(__stdcall *OpenSDK_Data_DeleteDevice)(const char* szAccessId, const char* szDeviceId);

/**
*  \brief 销毁SDK分配的内存
*  \param[in] pBuf SDK分配的内存
*  \return 0表示成功，-1表示失败
*/
typedef OPENSDK_RESULT(__stdcall *OpenSDK_Data_Free)(void* pBuf);

/**
*  \brief 设置告警推送
*  \param[in] handle 告警推送函数
*  \param[in] pUser  用户自定义数据，回调函数会原样抛出
*  \return 0表示成功，-1表示失败
*  \sa OpenSDK_Alarm_MessageHandler()
*/
typedef OPENSDK_RESULT(__stdcall *OpenSDK_Alarm_SetMsgCallBack)(OpenSDK_Alarm_MessageHandler handle, OpenSDK_Publish_MessageHandler publishHandle, void* pUser);

/**
*  \brief 开始接受告警
*  \param[in] szAccessToken 认证token
*  \return 0表示成功，-1表示失败
*  \sa OpenSDK_Alarm_StopRecv()
*/
typedef OPENSDK_RESULT(__stdcall *OpenSDK_Alarm_StartRecv)(const char* szAccessId);

/**
*  \brief 停止接收告警
*  \return 0表示成功，-1表示失败
*  \sa OpenSDK_Alarm_StartRecv()
*/
typedef OPENSDK_RESULT(__stdcall *OpenSDK_Alarm_StopRecv)();

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
typedef OPENSDK_RESULT(__stdcall *OpenSDK_PTZCtrl)(const char* szSessionId, const char* szAccessToken, const char* szCameraId, PTZCommand enCommand, PTZAction enAction, int iSpeed, LP_NSCBMsg pNSCBMsg);

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
typedef OPENSDK_RESULT(__stdcall *OpenSDK_DevDefence)(const char* szSessionId, const char* szAccessToken, const char* szCameraId, DefenceType enType, DefenceStatus enStatus, DefenceActor enActor, LP_NSCBMsg pNSCBMsg);

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
typedef OPENSDK_RESULT(__stdcall *OpenSDK_DevDefenceByDev)(const char* szSessionId, const char* szAccessToken, const char* szDeviceId, int iCameraNo, DefenceType enType, DefenceStatus enStatus, DefenceActor enActor, LP_NSCBMsg pNSCBMsg);

/**
*  \brief 获取AccessToken短信验证码
*  \param[in]  szSignString 签名字符串
*  \return 0表示成功，其他值表示失败
*/
typedef OPENSDK_RESULT(__stdcall *OpenSDK_GetAccessTokenSmsCode)(const char* szSignString);

/**
*  \brief 验证AccessToken短信验证码
*  \param[in] szSmsCode     短信验证码
*  \param[in] szUserId      用户Id
*  \param[in] szPhoneNumber 用户手机号码
*  \param[in] szAppKey      从平台获取的AppKey
*  \return 0表示成功，其他值表示失败
*/
typedef OPENSDK_RESULT(__stdcall *OpenSDK_VerifyAccessTokenSmsCode)(const char* szSmsCode, const char* szUserId, const char* szPhoneNumber, const char* szAppKey);

/**
*  \brief 获取硬件特征码短信验证码
*  \param[in]  szAccessToken 认证Token
*  \param[in]  szSignString  签名字符串
*  \return 0表示成功，其他值表示失败
*/
typedef OPENSDK_RESULT(__stdcall *OpenSDK_GetHdSignSmsCode)(const char* szAccessToken, const char* szSignString);

/**
*  \brief 验证硬件特征码短信验证码
*  \param[in] szAccessToken 认证Token
*  \param[in] szSmsCode     短信验证码
*  \param[in] szUserId      用户Id
*  \param[in] szAppKey      从平台获取的AppKey
*  \return 0表示成功，其他值表示失败
*/
typedef OPENSDK_RESULT(__stdcall *OpenSDK_VerifyHdSignSmsCode)(const char* szAccessToken, const char* szSmsCode, const char* szUserId, const char* szAppKey);

/**
*  \brief 更新摄像头信息到缓存
*  \param[in] szCameraId		监控点ID
*  \param[in] szAccessToken	认证Token
*  \return 0表示成功，其他值表示失败
*/
typedef OPENSDK_RESULT(__stdcall *OpenSDK_UpdateCameraInfoToLocal)(const char* szCameraId, const char* szAccessToken, bool& isEncrypt);



/**
*  \brief Http请求接口
*  \param[in] szUri			请求地址
*  \param[in] szHeaderParam	头部参数
*  \param[in] szBody			Body数据
*  \param[in] pBuf				返回报文的内容
*  \param[in] iLength			返回报文的长度
*  \return 0表示成功，其他值表示失败
*/
typedef OPENSDK_RESULT (__stdcall *OpenSDK_HttpSendWithWait)(const char* szUri, const char* szHeaderParam, const char* szBody, char** pBuf, int* iLength);

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


class COpenSdkMgr
{
private:
	HMODULE m_library;
	tagNETSTREAMAPI m_apis;
public:
	int initLibrary(const std::string& authAddr, const std::string& platform, const std::string appId);
	int releaseLibrary();

	std::string login();
	int deviceAdd(const std::string& accessId);
	int deviceOper(const std::string& accessId, const std::string& deviceId);

	std::string allocSession(OpenSDK_MessageHandler handle, void* pUser, const bool bSync = false, const unsigned int iTimeOut = 0xffffffff);
	int freeSession(const std::string& session);

	int startRealPlay(const std::string& session, HWND hPlayWnd, const std::string& cameraId, const std::string& accessId, const std::string& safekey, const std::string& appKey, int videoLevel = 0, LP_NSCBMsg pNSCBMsg = NULL);
	int stopRealPlay(const std::string& session, LP_NSCBMsg pNSCBMsg = NULL);

	int startPlayBack(const std::string& session, HWND hPlayWnd, const std::string& cameraId, const std::string& accessId, const std::string& safekey, const std::string& startTime, const std::string& stopTime, const std::string& appKey, LP_NSCBMsg pNSCBMsg = NULL);
	int playBackResume(const std::string& session);
	int playBackPause(const std::string& session);
	int stopPlayBack(const std::string& session, LP_NSCBMsg pNSCBMsg = NULL);
	int setDataCallBack(const std::string& szSessionId, OpenSDK_DataCallBack pDataCallBack, void* pUser);

	int startSearch(const std::string& session, const std::string& cameraId, const std::string& accessId, const std::string& startTime, const std::string& stopTime, LP_NSCBMsg pNSCBMsg = NULL);
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
	int PTZCtrl(const std::string& szSessionId, const std::string& szAccessToken, const std::string& szCameraId, PTZCommand enCommand, PTZAction enAction, int iSpeed, LP_NSCBMsg pNSCBMsg = NULL);
	int DevDefence(const std::string& szSessionId, const std::string& szAccessToken, const std::string& szCameraId, DefenceType enType, DefenceStatus enStatus, DefenceActor enActor, LP_NSCBMsg pNSCBMsg = NULL);
	int DevDefenceByDev(const std::string& szSessionId, const std::string& szAccessToken, const std::string& szDeviceId, int iCameraNo, DefenceType enType, DefenceStatus enStatus, DefenceActor enActor, LP_NSCBMsg pNSCBMsg = NULL);
	int DevDefence(const std::string& szSessionId, const std::string& szAccessToken, const std::string& szCameraId, DefenceType enType, DefenceStatus enStatus, DefenceActor enActor);
	int DevDefenceByDev(const std::string& szSessionId, const std::string& szAccessToken, const std::string& szDeviceId, int iCameraNo, DefenceType enType, DefenceStatus enStatus, DefenceActor enActor);
	int GetAccessTokenSmsCode(const std::string& szSignString);
	int VerifyAccessTokenSmsCode(const std::string& szSmsCode, const std::string& szUserId, const std::string& szPhoneNumber, const std::string& szAppKey);
	int GetHdSignSmsCode(const std::string& szAccessToken, const std::string& szSignString);
	int VerifyHdSignSmsCode(const std::string& szAccessToken, const std::string& szSmsCode, const std::string& szUserId, const std::string& szAppKey);
	int UpdateCameraInfo(const std::string& szCamera, const std::string& szAccessToken, bool& isEncrypt);
	int HttpSendWithWait(const char* szUri, const char* szHeaderParam, const char* szBody, char** pBuf, int* iLength);
	//COpenSdkMgr();
	~COpenSdkMgr();
	DECLARE_SINGLETON(COpenSdkMgr);
	DECLARE_UNCOPYABLE(COpenSdkMgr);
};

