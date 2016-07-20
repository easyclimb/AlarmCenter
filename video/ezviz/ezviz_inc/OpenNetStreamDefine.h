/** \file      OpenNetStreamDefine.h
 *  \copyright HangZhou Hikvision System Technology Co.,Ltd. All Right Reserved.
 *  \brief     开放SDK数据结构定义
 *
 *  \author    panlong, peter
 *  \date      2014/12/15
 *
 *  \note      history:
 *  \note      V0.0.1 2014/12/15 peter 增加头文件注释 
 */

#ifndef _H_OPENNETSTREAMDEFINE_H_
#define _H_OPENNETSTREAMDEFINE_H_

/** \typedef OPENSDK_RESULT
 *  \brief   opensdk返回值，0表示成功，非0表示失败
 */
typedef int OPENSDK_RESULT; 

/** \enum  MessageType 
 *  \brief 消息类型定义
 */
enum MessageType
{
	INS_PLAY_EXCEPTION            = 0,   ///< 播放异常，通常是设备断线或网络异常造成
	INS_PLAY_RECONNECT            = 1,   ///< 重连，实时流播放时内部会自动重连
	INS_PLAY_RECONNECT_EXCEPTION  = 2,   ///< 重连异常
	INS_PLAY_START                = 3,   ///< 播放开始
	INS_PLAY_STOP                 = 4,   ///< 播放终止
	INS_PLAY_ARCHIVE_END          = 5,   ///< 播放结束，回放结束时会有此消息

	INS_VOICETALK_START           = 16,  ///< 语音对讲开始
	INS_VOICETALK_STOP            = 17,  ///< 语音对讲停止
	INS_VOICETALK_EXCEPTION       = 18,  ///< 语音对讲异常 
	INS_PTZ_EXCEPTION             = 19,  ///< 云台控制异常
	INS_RECORD_FILE               = 20,  ///< 查询的录像文件(录像搜索结果)
	INS_RECORD_SEARCH_END         = 21,  ///< 录像查询结束（暂不使用）
	INS_RECORD_SEARCH_FAILED      = 22,  ///< 录像查询失败
	INS_DEFENSE_SUCCESS			  = 23,  ///< 布防成功
	INS_DEFENSE_FAILED			  = 24,  ///< 布防失败
	INS_PLAY_ARCHIVE_EXCEPTION    = 28,	 ///< 回放异常结束，可能是接收数据超时
	INS_PTZCTRL_SUCCESS			  = 46,  ///< 云台控制命令发送成功
	INS_PTZCTRL_FAILED			  = 47,	 ///< 云台控制失败
};

/** \struct STREAM_TIME
 *  \brief  开放SDK获取OSD时间格式定义
 */
typedef struct _STREAM_TIME
{
	unsigned int iYear;					 ///< 年
	unsigned int iMonth;				 ///< 月
	unsigned int iDay;					 ///< 日
	unsigned int iHour;					 ///< 时
	unsigned int iMinute;				 ///< 分
	unsigned int iSecond;				 ///< 秒
}STREAM_TIME, *LP_STREAM_TIME;

/** \struct NetStreamCallbackMessage
 *  \brief 经过NetStream处理所反馈的信息                                                                    
 */
typedef struct _NetStreamCallBackMessage
{
public:
	_NetStreamCallBackMessage(void):
	  iErrorCode(0), pMessageInfo(0){}
	unsigned int iErrorCode;			///< 消息回调反馈的错误码
	char *pMessageInfo;					///< 消息回调反馈的信息
} NSCBMsg, *LP_NSCBMsg;

/** \enum  AlarmType 
 *  \brief 开放SDK告警类型定义
 */
enum AlarmType
{
	ALARM_TYPE_ALL 					= -1,            ///< 全部
	BODY_SENSOR_EVENT				= 10000,		 ///< 人体感应事件
	EMERGENCY_BUTTON_EVENT 			= 10001,		///< 紧急遥控按钮事件
	MOTION_DETECT_ALARM 			= 10002,		///< 移动侦测报警
	BABY_CRY_ALARM 					= 10003,		///< 婴儿啼哭报警
	MAGNETIC_ALARM 					= 10004,		///< 门磁报警
	SMOKE_DETECTOR_ALARM 			= 10005,		///< 烟感报警
	COMBUSTIBLE_GAS_ALARM 			= 10006,		///< 可燃气体报警
	FLOOD_IN_ALARM 					= 10008,		///< 水浸报警
	EMERGENCY_BUTTON_ALARM 			= 10009,		///< 紧急按钮报警
	BODY_SENSOR_ALARM 				= 10010,		///< 人体感应报警
	SHELTER_ALARM					= 10011, 		///< 遮挡报警
	VIDEO_LOSS_ALARM				= 10012,		///< 视频丢失
	LINE_DETECTION_ALARM			= 10013,		///< 越界侦测
	FIELD_DETECTION_ALARM			= 10014,		///< 区域入侵
	FACE_DETECTION_ALARM			= 10015,		///< 人脸检测事件
	DOOR_BELL_ALARM					= 10016,		///< 智能门铃报警
	DEVOFFLINE_ALARM				= 10017,		///< 摄像机失去关联报警
	CURTAIN_ALARM					= 10018,		///< 幕帘报警
	MOVE_MAGNETOMETER_ALARM			= 10019,		///< 单体门磁报警
	SCENE_CHANGE_DETECTION_ALARM	= 10020,		///< 场景变更侦测
	DEFOCUS_ALARM					= 10021,		///< 虚焦侦测
	AUDIO_EXCEPTION_ALARM			= 10022,		///< 音频异常侦测
	LEFT_DETECTION_ALARM			= 10023,		///< 物品遗留侦测
	TAKE_DETECTION_ALARM			= 10024,		///< 物品拿取侦测
	PARKING_DETECTION_ALARM			= 10025,		///< 非法停车侦测
	HIGH_DENSITY_DETECTION_ALARM	= 10026,		///< 人员聚集侦测
	LOITER_DETECTION_ALARM			= 10027,		///< 徘徊检测侦测
	RUN_DETECTION_ALARM				= 10028,		///< 快速移动侦测
	ENTER_AREA_DETECTION_ALARM		= 10029,		///< 进入区域侦测
	EXIT_AREA_DETECTION_ALARM		= 10030,		///< 离开区域侦测
	MAG_GIM_ALARM					= 10031,		///< 磁干扰报警
	UNDER_VOLTAGE_ALARM				= 10032,		///< 电池欠压报警
	INTRUSION_ALARM					= 10033,		///< 闯入报警
	IO_00_ALARM						= 10100,		///< IO报警
	IO_01_ALARM						= 10101,		///< IO-1报警
	IO_02_ALARM						= 10102,		///< IO-2报警
	IO_03_ALARM						= 10103,		///< IO-3报警
	IO_04_ALARM						= 10104,		///< IO-4报警
	IO_05_ALARM						= 10105,		///< IO-5报警
	IO_06_ALARM						= 10106,		///< IO-6报警
	IO_07_ALARM						= 10107,		///< IO-7报警
	IO_08_ALARM						= 10108,		///< IO-8报警
	IO_09_ALARM						= 10109,		///< IO-9报警
	IO_10_ALARM						= 10110,		///< IO-10报警
	IO_11_ALARM						= 10111,		///< IO-11报警
	IO_12_ALARM						= 10112,		///< IO-12报警
	IO_13_ALARM						= 10113,		///< IO-13报警
	IO_14_ALARM						= 10114,		///< IO-14报警
	IO_15_ALARM						= 10115,		///< IO-15报警
	IO_16_ALARM						= 10116,		///< IO-16报警
};

/** \enum  AlarmNotifyType
 *  \brief 开放SDK告警通知类型定义
 */
typedef enum _AlarmNotifyType
{
	ALARM_NOTIFY_CONNECT_EXCEPTION = 100,	///< SDK同萤石平台连接异常
	ALARM_NOTIFY_RECONNECT_SUCCESS,			///< 重连成功
	ALARM_NOTIFY_RECONNECT_FAILED			///< 重连失败
} AlarmNotifyType;

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
 *  一般情况下，鼠标按下代表开始，鼠标松开代表停止
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

/** \enum  DataType 
 *  \brief 音视频流数据类型
 */
enum DataType {
	NET_DVR_SYSHEAD       = 1,      ///< 流头
	NET_DVR_STREAMDATA    = 2,		///< 流数据
	NET_DVR_RECV_END      = 3,      ///< 结束标记
};

#endif