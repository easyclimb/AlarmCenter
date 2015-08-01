/** \file      OpenNetStreamError.h
 *  \copyright HangZhou Hikvision System Technology Co.,Ltd. All Right Reserved.
 *  \brief     开放SDK错误码定义
 *
 *  \author    panlong, peter
 *  \date      2014/12/15
 *
 *  \note      history: 
 *  \note      V0.0.1 2014/12/15 peter 修改文件注释 
 */

#ifndef _H_OPENNETSTREAMERROR_H_ 
#define _H_OPENNETSTREAMERROR_H_

#define OPEN_SDK_NOERROR				0					///< 没有错误
#define OPEN_SDK_GENERAL_ERROR			-1					///< 常规（通用）错误

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

#define DATA_ACCESS_ERROR			19999

#endif