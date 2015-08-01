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

	INS_RECORD_FILE               = 20,  ///< 查询的录像文件(录像搜索结果)
	INS_RECORD_SEARCH_END         = 21,  ///< 录像查询结束（暂不使用）
	INS_RECORD_SEARCH_FAILED      = 22,  ///< 录像查询失败
	INS_DEFENSE_SUCCESS			  = 23,  ///<布防成功
	INS_DEFENSE_FAILED			  = 24,  ///<布防失败
	INS_PTZCTRL_SUCCESS			  = 46,  ///<云台控制命令发送成功
	INS_PTZCTRL_FAILED			  = 47,	 ///<云台控制失败
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
	ALARM_TYPE_ALL = -1,                 ///< 全部
	BODY_SENSOR_EVENT = 10000,		     ///< 人体感应事件
	EMERGENCY_BUTTON_EVENT,				 ///< 紧急遥控按钮事件
	MOTION_DETECT_ALARM,				 ///< 移动侦测报警
	BABY_CRY_ALARM,						 ///< 婴儿啼哭报警
	MAGNETIC_ALARM,						 ///< 门磁报警
	SMOKE_DETECTOR_ALARM,				 ///< 烟感报警
	COMBUSTIBLE_GAS_ALARM,				 ///< 可燃气体报警
	FLOOD_IN_ALARM,						 ///< 水浸报警
	EMERGENCY_BUTTON_ALARM,				 ///< 紧急按钮报警
	BODY_SENSOR_ALARM,					 ///< 人体感应报警
	IO_00_ALARM,						 ///< IO报警
	IO_01_ALARM,						 ///< IO-1报警
	IO_02_ALARM,						 ///< IO-2报警
	IO_03_ALARM,						 ///< IO-3报警
	IO_04_ALARM,						 ///< IO-4报警
	IO_05_ALARM,						 ///< IO-5报警
	IO_06_ALARM,						 ///< IO-6报警
	IO_07_ALARM,						 ///< IO-7报警
	IO_08_ALARM,						 ///< IO-8报警
	IO_09_ALARM,						 ///< IO-9报警
	IO_10_ALARM,						 ///< IO-10报警
	IO_11_ALARM,						 ///< IO-11报警
	IO_12_ALARM,						 ///< IO-12报警
	IO_13_ALARM,						 ///< IO-13报警
	IO_14_ALARM,						 ///< IO-14报警
	IO_15_ALARM,						 ///< IO-15报警
	IO_16_ALARM,						 ///< IO-16报警
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