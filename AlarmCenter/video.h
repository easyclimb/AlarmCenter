#pragma once
#include <list>
#include <map>
#include <memory>

namespace video {

typedef enum msg_type
{
	TYPE_GET = 1,
	TYPE_HD = 2,
}msg_type;

typedef enum productor
{
	UNKNOWN,
	EZVIZ,
	JOVISION,
}productor;

inline productor Integer2Productor(int n) {
	switch (n) {
	case video::EZVIZ:
		return video::EZVIZ;
		break;
	case video::JOVISION:
		return video::JOVISION;
		break;
	case video::UNKNOWN:
	default:
		return video::UNKNOWN;
		break;
	}
}

class productor_info
{
private:
	productor _productor;
	std::wstring _name;
	std::wstring _description;
	std::string _appKey;

public:
	DECLARE_GETTER(productor, _productor);
	DECLARE_GETTER_SETTER(std::wstring, _name);
	DECLARE_GETTER_SETTER(std::wstring, _description);

	std::wstring get_formatted_name() const { return _name + L"[" + _description + L"]"; }

	productor_info(productor productor,
				   const std::wstring& name,
				   const std::wstring& desc)
		: _productor(productor)
		, _name(name)
		, _description(desc)
	{}

	productor_info()
		: _productor(UNKNOWN)
		, _name()
		, _description()
	{}

	productor_info(const productor_info& rhs)
		: _productor(rhs._productor)
		, _name(rhs._name)
		, _description(rhs._description)
	{}

	const productor_info& operator=(const productor_info& rhs)
	{
		_productor = rhs._productor;
		_name = rhs._name;
		_description = rhs._description;

		return *this;
	}
};

static const productor_info ProductorUnknown;

#define SET_DATA_MEMBER_STRING(obj, member) obj->set_##member(W2A(member));
#define SET_DATA_MEMBER_WCSTRING(obj, member) obj->set_##member(member.LockBuffer()); member.UnlockBuffer();
#define SET_DATA_MEMBER_INTEGER(obj, member) obj->set_##member(member);

#define SET_DEVICE_INFO_DATA_MEMBER_STRING(member) SET_DATA_MEMBER_STRING(deviceInfo, member);
#define SET_DEVICE_INFO_DATA_MEMBER_WCSTRING(member) SET_DATA_MEMBER_WCSTRING(deviceInfo, member);
#define SET_DEVICE_INFO_DATA_MEMBER_INTEGER(member) SET_DATA_MEMBER_INTEGER(deviceInfo, member);

#define SET_USER_INFO_DATA_MEMBER_STRING(member) SET_DATA_MEMBER_STRING(userInfo, member);
#define SET_USER_INFO_DATA_MEMBER_WSTRING(member) SET_DATA_MEMBER_WCSTRING(userInfo, member);
#define SET_USER_INFO_DATA_MEMBER_INTEGER(member) SET_DATA_MEMBER_INTEGER(userInfo, member);

class video_user_info;
typedef std::shared_ptr<video_user_info> video_user_info_ptr;
typedef std::weak_ptr<video_user_info> video_user_info_weak_ptr;
typedef std::list<video_user_info_ptr> video_user_info_list;

class video_device_info;
typedef std::shared_ptr<video_device_info> video_device_info_ptr;
typedef std::list<video_device_info_ptr> video_device_info_list;

namespace ezviz {
class video_user_info_ezviz;
typedef std::shared_ptr<video_user_info_ezviz> video_user_info_ezviz_ptr;
typedef std::list<video_user_info_ezviz_ptr> video_user_info_ezviz_list;

class video_device_info_ezviz;
typedef std::shared_ptr<video_device_info_ezviz> video_device_info_ezviz_ptr;
typedef std::list<video_device_info_ezviz_ptr> video_device_info_ezviz_list;
};

namespace jovision {
class video_user_info_jovision;
typedef std::shared_ptr<video_user_info_jovision> video_user_info_jovision_ptr;
typedef std::list<video_user_info_jovision_ptr> video_user_info_jovision_list;

class video_device_info_jovision;
typedef std::shared_ptr<video_device_info_jovision> video_device_info_jovision_ptr;
typedef std::list<video_device_info_jovision_ptr> video_device_info_jovision_list;

typedef int JCLink_t;
//事件类型
enum JCEventType
{
	//连接部分
	JCET_Connected, //未使用
	JCET_ConnectOK,	//连接成功
	JCET_UserAccessError, //用户验证失败
	JCET_NoChannel,		//主控通道未开启
	JCET_ConTypeError,	//连接类型错误
	JCET_ConCountLimit,	//超过主控连接最大数
	JCET_ConTimeout,	//连接超时
	JCET_DisconOK, //断开连接成功
	JCET_ConAbout, //连接异常断开
	JCET_ServiceStop,	//主控断开连接

	//远程录像相关
	JCET_GetFileListOK,		//获取远程录像成功
	JCET_GetFileListError,	//获取远程录像失败

	JCET_RemotePlayBegin,	//远程回放开始
	JCET_RemotePlayEnd,		//远程回放结束
	JCET_RemoteError,		//远程回放失败
	JCET_RemotePlayTimeout,	//远程回访超时

	JCET_DownloadData,		//录像下载数据
	JCET_DownloadEnd,		//录像下载完成
	JCET_DownloadStop,		//录像下载停止
	JCET_DownloadError,		//远程下载失败
	JCET_DownloadTimeout,	//远程下载超时

	//其他
	JCET_StreamReset,	//码流重置信号(码流配置改变,须关闭并重新开启解码、录像等)
	JCET_Alarm,	//报警信号

	JCET_MAX,
};

//连接状态
enum JCLinkStatus
{
	JCLS_UnConnect,	//未连接
	JCLS_Connecting,//连接中
	JCLS_WaitData,	//连接成功等待数据
	JCLS_Connected,	//已连接
};

//数据帧类型
enum JCFrameType
{
	JCFT_I,			//视频I帧
	JCFT_P,			//视频P帧
	JCFT_B,			//视频B帧
	JCFT_Audio,		//音频帧
};

//录像类型
enum JCRecFileType
{
	JCRT_Unknown = 0,//未知
	JCRT_SV4,		//sv4文件
	JCRT_SV5,		//sv5文件
	JCRT_SV6,		//sv6文件
	JCRT_MP4,		//MP4文件
};

//设备类型
enum JCDeviceType
{
	JCDT_All = 0,		//未知设备
	JCDT_Card,			//视频采集卡
	JCDT_DVR,			//DVR
	JCDT_IPC,			//IPC
	JCDT_NVR,			//NVR
};

//编码类型
enum JCCodecID
{
	JCCID_Unknown,		//未知编码

	//视频编码
	JCCID_JENC04,		//中维04版编码
	JCCID_H264,			//H264编码

	//音频编码
	JCCID_PCM,			//PCM数据
	JCCID_AMR,			//AMR编码
	JCCID_G711A,		//G711编码A
	JCCID_G711U,		//G711编码U
};

//码流帧
typedef struct JCStreamFrame
{
	JCFrameType sType;
	PUCHAR pFrameData;
	int nFrameSize;
}JCStreamFrame, *PJCStreamFrame;

//检索设备信息
typedef struct JCLanDeviceInfo
{
	char szCloudSEE[16];
	char szIP[16];
	int nPort;
	char szDeviceName[128];
	int nDeviceType;
	int nChannelNum;
}JCLanDeviceInfo, *PJCLanDeviceInfo;

//设备信息
typedef struct JCDeviceInfo
{
	JCDeviceType eDevType;
}JCDeviceInfo, *PJCDeviceInfo;

//码流信息
typedef struct JCStreamInfo
{
	JCCodecID eVideoCodec;		//视频编码
	int nFrameWidth;			//视频帧宽度
	int nFrameHeight;			//视频帧高度
	int nVideoFrameRateNum;		//视频帧率分子
	int nVideoFrameRateDen;		//视频帧率分母

	JCCodecID eAudioCodec;		//音频编码
	int nAudioSamples;			//音频采样率
	int nAudioSampleBits;		//音频采样位数
	int nAudioChannels;			//音频通道数

	JCRecFileType eRecFileType;	//录像文件类型
}JCStreamInfo, *PJCStreamInfo;

//日期段
typedef struct JCDateBlock
{
	int nBeginYear;		//开始年份
	int nBeginMonth;	//开始月份
	int nBeginDay;		//开始日期
	int nEndYear;		//结束年份
	int nEndMonth;		//结束月份
	int nEndDay;		//结束日期
}JCDateBlock, *PJCDateBlock;

//录像信息
typedef struct JCRecFileInfo
{
	int nRecFileID;
	JCRecFileType eType;
	char szBeginTime[8];
	char szFilename[16];
	char szPathName[64];
}JCRecFileInfo, *PJCRecFileInfo;

//yv12数据帧
typedef struct JCRawFrame
{
	DWORD dwWidth;
	DWORD dwHeight;
	PUCHAR pY;
	PUCHAR pU;
	PUCHAR pV;
	DWORD dwYPitch;
	DWORD dwUPitch;
	DWORD dwVPitch;
}JCRawFrame, *PJCRawFrame;
}; // namespace jovision end


typedef struct zone_uuid
{
	int _ademco_id;
	int _zone_value;
	int _gg;

public:
	zone_uuid(int ademco_id, int zone_value, int gg)
		:_ademco_id(ademco_id), _zone_value(zone_value), _gg(gg)
	{}

	zone_uuid() = default;
	zone_uuid& operator=(const zone_uuid& rhs) = default;
	// use default

	bool operator == (const zone_uuid& rhs)
	{
		return (_ademco_id == rhs._ademco_id)
			&& (_zone_value == rhs._zone_value)
			&& (_gg == rhs._gg);
	}
#ifdef _DEBUG
	std::string toString() const
	{
		char buff[1024] = { 0 };
		sprintf_s(buff, "ademco_id:%d, zone_value:%d, gg:%d", _ademco_id, _zone_value, _gg);
		return std::string(buff);
	}
#endif

}zone_uuid;
typedef std::shared_ptr<zone_uuid> zone_uuid_ptr;
typedef std::list<zone_uuid_ptr> zone_uuid_list;

struct cmp_zone_uuid
{
	bool operator() (const zone_uuid& l, const zone_uuid& r) const
	{
		if (l._ademco_id < r._ademco_id) return true;
		if (l._ademco_id > r._ademco_id) return false;

		if (l._zone_value < r._zone_value) return true;
		if (l._zone_value > r._zone_value) return false;

		if (l._gg < r._gg) return true;
		if (l._gg > r._gg) return false;

		return false;
	}
};

typedef struct bind_info
{
	int _id;
	video_device_info_ptr _device;
	int auto_play_when_alarm_;

	bind_info() = default;

	bind_info(int id, video_device_info_ptr device, int auto_play_when_alarm)
		:_id(id), _device(device), auto_play_when_alarm_(auto_play_when_alarm)
	{}

	bind_info& operator=(const bind_info& rhs) = default;

#ifdef _DEBUG
	std::string toString() const
	{
		char buff[1024] = { 0 };
		sprintf_s(buff, "_device:%p, auto_play_when_alarm_:%d", _device.get(), auto_play_when_alarm_);
		return std::string(buff);
	}
#endif
}bind_info;

typedef std::map<zone_uuid, bind_info, cmp_zone_uuid> bind_map;

typedef struct video_device_identifier {
	video::productor productor = video::UNKNOWN;
	int dev_id = -1;
}video_device_identifier;

};


