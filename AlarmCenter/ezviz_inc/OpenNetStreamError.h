/** \file      OpenNetStreamError.h
 *  \copyright HangZhou Hikvision System Technology Co.,Ltd. All Right Reserved.
 *  \brief     开放SDK错误码定义
 *
 *  \author    panlong, peter
 *  \date      2014/12/15
 *
 *  \note      history: 
 *  \note      V2.4.6 2016/01/21 pikongxuan 新版本错误码定义
 */

#ifndef _H_OPENNETSTREAMERROR_H_ 
#define _H_OPENNETSTREAMERROR_H_

#define OPEN_SDK_NOERROR				0					///< 没有错误
#define OPEN_SDK_GENERAL_ERROR			-1					///< 常规（通用）错误

/******************************************************************************************************************/
/*下列为V2.4.6以前错误码，V2.4.6版本之后将不再使用*/
/******************************************************************************************************************/

#define OPEN_SDK_BASE					10000				///< 开放SDK基础错误  
#define OPEN_SDK_JSON_ERROR				OPEN_SDK_BASE + 1	///< Json解析出错     
#define OPEN_SDK_ERROR					OPEN_SDK_BASE + 2	///< 获取平台数据出错 
#define OPEN_SDK_DEV_NO_SUPPORT			OPEN_SDK_BASE + 3	///< 不支持的设备     
#define OPEN_SDK_ALLOC_ERROR			OPEN_SDK_BASE + 4	///< 申请内存失败     
#define OPEN_SDK_PARAM_ERROR			OPEN_SDK_BASE + 5	///< 传入参数非法     
#define OPEN_SDK_SAFE_KEY_ERROR			OPEN_SDK_BASE + 6	///< 安全密钥出错     
#define OPEN_SDK_SEARCHING_ERROR		OPEN_SDK_BASE + 7	///< 录像搜索出错
#define OPEN_SDK_SYNC_ERROR				OPEN_SDK_BASE + 8	///< 同步参数出错  
#define OPEN_SDK_INTERFACE_NO_IMPL		OPEN_SDK_BASE + 9	///< 接口未实现，主要针对平台
#define OPEN_SDK_ORDER_ERROR			OPEN_SDK_BASE + 10  ///< 接口调用顺序出错
#define OPEN_SDK_SESSION_EXPIRED		OPEN_SDK_BASE + 11  ///< SESSION已失效，Session对象被释放

#define DATA_ACCESS_ERROR			19999


/******************************************************************************************************************/
/* 下列为V2.4.6新版本错误码，通过OpenSDK_GetLastErrorCode拿到。接口基本默认只返回0和-1（除了部分特殊说明的接口）
错误码格式{xbbbbb}, x表示错误码表示，x的值如下表：
/// 0: correct condition
/// 1: network setup exception,(no DNS, connection lost)
/// 2: user operation error, input invalid params, rely on lib error(load, init, unload)
/// 3: platform server error
/// 4: memory exception, system resource(alloc memory, create thread failed)
/// 5: opensdk dependent lib netstream errorcode.
/// 6: method not supported
/// 9: undefined error
*/
/******************************************************************************************************************/

//网络级别错误码，CURL反馈, 都是调开放平台接口报错（测试时首先保证登陆成功）
#define OPEN_SDK_NETWORK_SETUP_BASE			100000
#define	OPEN_SDK_COULDNT_RESOLVE_HOST		OPEN_SDK_NETWORK_SETUP_BASE + 6		///< 给定的远程主机没有得到解析，这里是指platform域名无法正常解析,可能是DNS没有配置或者机器没有连网。
#define OPEN_SDK_COULDNT_CONNECT			OPEN_SDK_NETWORK_SETUP_BASE + 7		///< 远程主机不可达，这里是指无法访问platform,可能是platform地址配置错误。
#define OPEN_SDK_OPERATION_TIMEOUT			OPEN_SDK_NETWORK_SETUP_BASE + 28	///< 请求操作超时, 超时时间为20s, 请求平台超时，请检查platform地址配置错误。

//用户操作级别错误码
#define OPEN_SDK_USER_OPERATION_BASE		200000
#define OPEN_SDK_BAD_PARAMS					OPEN_SDK_USER_OPERATION_BASE + 1	///< 接口传入参数不符合要求
#define OPEN_SDK_SESSION_INVALID			OPEN_SDK_USER_OPERATION_BASE + 2	///< 当前Session不存在或者被释放,可能是SessionId传入值错误或者是Session已经被释放。
#define OPEN_SDK_VIDEO_RECORD_NOT_EXIST		OPEN_SDK_USER_OPERATION_BASE + 3	///< 指定时间段内录像记录不存在
#define OPEN_SDK_VIDEO_RECORD_SEARCHING		OPEN_SDK_USER_OPERATION_BASE + 4	///< 录像记录正在搜索
#define OPEN_SDK_STOP_ALARM_REC_FAILED		OPEN_SDK_USER_OPERATION_BASE + 5	///< 关闭告警失败, 可能是没有开启告警或者已经关闭告警


//萤石云接口级别错误码, 详细参考https://open.ys7.com/doc.html#id=436
#define OPEN_SDK_OPENAPI_BASE				300000
#define OPEN_SDK_RESPINFO_BAD				OPEN_SDK_OPENAPI_BASE + 1			///< 请求返回的信息,json无法正常解析,可能是PlatformAddr配置有问题
#define OPEN_SDK_RESPINFO_INVALID			OPEN_SDK_OPENAPI_BASE + 2			///< 请求返回信息格式有误
#define OPEN_SDK_ACCESSTOKEN_INVALID		OPEN_SDK_OPENAPI_BASE + 10002		///< accesstoken异常或者过期,accessToken异常或请求方法不存在
#define OPEN_SDK_SIGNATURE_ERROR			OPEN_SDK_OPENAPI_BASE + 10008		///< 表示输入参数有问题。平台显示签名错误
#define OPEN_SDK_USERID_PHONE_UNBIND		OPEN_SDK_OPENAPI_BASE + 10014		///< APPKEY下对应的第三方userId和phone未绑定
#define OPEN_SDK_CHANNEL_NOT_EXIST			OPEN_SDK_OPENAPI_BASE + 20001		///< 通道不存在,通道对应某一监控点
#define OPEN_SDK_IDENTIFY_FAILED			OPEN_SDK_OPENAPI_BASE + 20005		///< 安全认证失败,需进行SDK安全认证, 可能原因是开通萤石云服务后，跨机器或者是AccessToken（通过getAccessToken接口拿到）和AppKey对应不同
#define OPEN_SDK_USER_NOTOWN_DEVICE			OPEN_SDK_OPENAPI_BASE + 20018		///< 该用户不拥有该设备

//系统资源级别错误
#define OPEN_SDK_SYSTEM_RESOURCE_BASE		400000
#define OPEN_SDK_COULDNT_CREATE_THREAD		OPEN_SDK_SYSTEM_RESOURCE_BASE + 1	///< 创建线程失败
#define OPEN_SDK_COULDNT_ALLOC_BUFFERS		OPEN_SDK_SYSTEM_RESOURCE_BASE + 2	///< 申请内存资源失败

//不支持的错误码
#define OPEN_SDK_NOT_SUPPORT_BASE			600000
#define OPEN_SDK_DEV_NOT_SUPPORT			OPEN_SDK_NOT_SUPPORT_BASE + 1		///< 不支持非1.7设备
#define OPEN_SDK_API_NO_IMPLEMENT			OPEN_SDK_NOT_SUPPORT_BASE + 2		///< 接口未实现

//未定义的错误码
#define OPEN_SDK_UNDEFINE_BASE				900000
#define OPEN_SDK_ALLOCSESSION_FAILED		OPEN_SDK_UNDEFINE_BASE + 1			///< AllocSession 失败
#define OPEN_SDK_SEARCH_RECORD_FAILED		OPEN_SDK_UNDEFINE_BASE + 2			///< 查询回放记录失败
#define OPEN_SDK_START_ALARM_REC_FAILED		OPEN_SDK_UNDEFINE_BASE + 3			///< 开启告警失败


//预览时回调接口反馈的错误码, 下列为常见错误码
#define INS_ERROR_OPERATIONCODE_FAILED								45  		///< 获取操作码失败， 一般是由于用户开启终端绑定了，会报这个错误。该用户没有关联硬件特征码,设备操作码and密钥获取失败,错误码为:3128
#define OPEN_SDK_STREAM_LIMIT										410			///< 取流路数限制， 即设备达到连接上线(直连3路，流媒体由服务器配置)
#define OPEN_SDK_DEV_NO_ONLINE										3121		///< 设备不在线
#define OPEN_SDK_PERMANENTKEY_EXCEPTION								2012		///< 重新输入密钥  1.明文密钥和输入MD5密钥不相等
#define OPEN_SDK_VTDU_TIMEOUT										2021		///< 流媒体向设备发送或接受信令超时,查看设备连接网络是否稳定。一般是网络问题。
#define OPEN_SDK_VTDU_CLIENT_TIMEOUT								2025		///< vtdu客户端接收回应超时,没有视频源,可能摄像头与后端设备接触有问题。
//vtdu rtsp
#define OPEN_SDK_VTDU_STOP											2034		///< vtdu取流停止失败
#define OPEN_SDK_VTDU_TOKEN_NOCONNECT_VTM							2047		///< VTDU 客户端连接不上VTM,预览时候断网,包重连异常
#define OPEN_SDK_VTDU_TOKEN_NOCONNECT_VTDU							2048		///< VTDU 客户端连接不上VTDU
#define OPEN_SDK_DEVICE_UNCONNECT_VTDU								2051		///< 设备连接不上流媒体,基本判断为网络问题引起。

#define OPEN_SDK_PLATFORM_CLIENT_NO_SIGN_RELEATED					3128		///< 没有关联特征码
#define OPEN_SDK_TTS_DEV_NO_ONLINE	 								5012		///< TTS设备不在线
#define INS_ERROR_V17_GET_OPERATIONCODE_PARAMETER_ERROR				2056		///< 获取操作码参数错误,开启终端绑定，且没有验证通过就进行预览。
//PRIVATE_VTDU
#define OPEN_SDK_PRIVATE_VTDU_REQUEST_TIMEOUT						2225		///< 网络带宽受限或者短时间对同一个请求过于频繁
#define OPEN_SDK_PRIVATE_VTDU_DISCONNECTED_LINK						2226		///< 网络无法联通
#define OPEN_SDK_PRIVATE_VTDU_STATUS_411							5411		///< token无权限,过期失效
#define OPEN_SDK_PRIVATE_VTDU_STATUS_452							5452		///< 设备连接预览流媒体服务器失败,设备同vtdu不在同一个局域网
#define OPEN_SDK_PRIVATE_VTDU_STATUS_546							5546		///< 并发路数限制


#endif	//_H_OPENNETSTREAMERROR_H_