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
//�¼�����
enum JCEventType
{
	//���Ӳ���
	JCET_Connected, //δʹ��
	JCET_ConnectOK,	//���ӳɹ�
	JCET_UserAccessError, //�û���֤ʧ��
	JCET_NoChannel,		//����ͨ��δ����
	JCET_ConTypeError,	//�������ʹ���
	JCET_ConCountLimit,	//�����������������
	JCET_ConTimeout,	//���ӳ�ʱ
	JCET_DisconOK, //�Ͽ����ӳɹ�
	JCET_ConAbout, //�����쳣�Ͽ�
	JCET_ServiceStop,	//���ضϿ�����

	//Զ��¼�����
	JCET_GetFileListOK,		//��ȡԶ��¼��ɹ�
	JCET_GetFileListError,	//��ȡԶ��¼��ʧ��

	JCET_RemotePlayBegin,	//Զ�̻طſ�ʼ
	JCET_RemotePlayEnd,		//Զ�̻طŽ���
	JCET_RemoteError,		//Զ�̻ط�ʧ��
	JCET_RemotePlayTimeout,	//Զ�̻طó�ʱ

	JCET_DownloadData,		//¼����������
	JCET_DownloadEnd,		//¼���������
	JCET_DownloadStop,		//¼������ֹͣ
	JCET_DownloadError,		//Զ������ʧ��
	JCET_DownloadTimeout,	//Զ�����س�ʱ

	//����
	JCET_StreamReset,	//���������ź�(�������øı�,��رղ����¿������롢¼���)
	JCET_Alarm,	//�����ź�

	JCET_MAX,
};

//����״̬
enum JCLinkStatus
{
	JCLS_UnConnect,	//δ����
	JCLS_Connecting,//������
	JCLS_WaitData,	//���ӳɹ��ȴ�����
	JCLS_Connected,	//������
};

//����֡����
enum JCFrameType
{
	JCFT_I,			//��ƵI֡
	JCFT_P,			//��ƵP֡
	JCFT_B,			//��ƵB֡
	JCFT_Audio,		//��Ƶ֡
};

//¼������
enum JCRecFileType
{
	JCRT_Unknown = 0,//δ֪
	JCRT_SV4,		//sv4�ļ�
	JCRT_SV5,		//sv5�ļ�
	JCRT_SV6,		//sv6�ļ�
	JCRT_MP4,		//MP4�ļ�
};

//�豸����
enum JCDeviceType
{
	JCDT_All = 0,		//δ֪�豸
	JCDT_Card,			//��Ƶ�ɼ���
	JCDT_DVR,			//DVR
	JCDT_IPC,			//IPC
	JCDT_NVR,			//NVR
};

//��������
enum JCCodecID
{
	JCCID_Unknown,		//δ֪����

	//��Ƶ����
	JCCID_JENC04,		//��ά04�����
	JCCID_H264,			//H264����

	//��Ƶ����
	JCCID_PCM,			//PCM����
	JCCID_AMR,			//AMR����
	JCCID_G711A,		//G711����A
	JCCID_G711U,		//G711����U
};

//����֡
typedef struct JCStreamFrame
{
	JCFrameType sType;
	PUCHAR pFrameData;
	int nFrameSize;
}JCStreamFrame, *PJCStreamFrame;

//�����豸��Ϣ
typedef struct JCLanDeviceInfo
{
	char szCloudSEE[16];
	char szIP[16];
	int nPort;
	char szDeviceName[128];
	int nDeviceType;
	int nChannelNum;
}JCLanDeviceInfo, *PJCLanDeviceInfo;

//�豸��Ϣ
typedef struct JCDeviceInfo
{
	JCDeviceType eDevType;
}JCDeviceInfo, *PJCDeviceInfo;

//������Ϣ
typedef struct JCStreamInfo
{
	JCCodecID eVideoCodec;		//��Ƶ����
	int nFrameWidth;			//��Ƶ֡���
	int nFrameHeight;			//��Ƶ֡�߶�
	int nVideoFrameRateNum;		//��Ƶ֡�ʷ���
	int nVideoFrameRateDen;		//��Ƶ֡�ʷ�ĸ

	JCCodecID eAudioCodec;		//��Ƶ����
	int nAudioSamples;			//��Ƶ������
	int nAudioSampleBits;		//��Ƶ����λ��
	int nAudioChannels;			//��Ƶͨ����

	JCRecFileType eRecFileType;	//¼���ļ�����
}JCStreamInfo, *PJCStreamInfo;

//���ڶ�
typedef struct JCDateBlock
{
	int nBeginYear;		//��ʼ���
	int nBeginMonth;	//��ʼ�·�
	int nBeginDay;		//��ʼ����
	int nEndYear;		//�������
	int nEndMonth;		//�����·�
	int nEndDay;		//��������
}JCDateBlock, *PJCDateBlock;

//¼����Ϣ
typedef struct JCRecFileInfo
{
	int nRecFileID;
	JCRecFileType eType;
	char szBeginTime[8];
	char szFilename[16];
	char szPathName[64];
}JCRecFileInfo, *PJCRecFileInfo;

//yv12����֡
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
};

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


};


