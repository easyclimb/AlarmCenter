#pragma once


namespace video {

namespace jovision {

namespace detail {

#define JCSDK_CALLTYPE __stdcall

#define JCSDK_INVALID_LINKVALUE -1
typedef int JCLink_t;

#define JCMAX_LINKNUM 512

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

}

using namespace video::jovision::detail;

class sdk_mgr_jovision
{

	//回调函数
	typedef void(__cdecl *JCEventCallback_t)(JCLink_t nLinkID, JCEventType etType, DWORD_PTR pData1, DWORD_PTR pData2, LPVOID pUserData);
	typedef void(__cdecl *JCDataCallback_t)(JCLink_t nLinkID, PJCStreamFrame pFrame, LPVOID pUserData);
	typedef void(__cdecl *JCRawDataCallback_t)(JCLink_t nLinkID, PJCRawFrame pFrame, LPVOID pUserData);
	typedef void(__cdecl *JCLanSearchDeviceCallback_t)(PJCLanDeviceInfo pDevice);

	//全局函数------------------------------------------------------------------------------
	//初始化SDK
	typedef  BOOL(JCSDK_CALLTYPE *JCSDK_InitSDK)(int nLocStartPort);

	//释放SDK
	typedef void (JCSDK_CALLTYPE *JCSDK_ReleaseSDK)();

	//设置回调函数
	typedef void (JCSDK_CALLTYPE *JCSDK_RegisterCallback)(
		JCEventCallback_t funEventCallback,
		JCDataCallback_t funDataCallback,
		JCRawDataCallback_t funRawDataCallback,
		JCLanSearchDeviceCallback_t funLanSearchDeviceCallback);

	//连接-----------------------------------------------------------------------------------
	//连接
	typedef JCLink_t(JCSDK_CALLTYPE *JCSDK_Connect)(char *szServer, int nPort, int nChannel,
													char *szNetUser, char *szPwd, BOOL bNumOrIP, LPVOID pUserData);

	//断开连接
	typedef BOOL(JCSDK_CALLTYPE *JCSDK_Disconnect)(JCLink_t nLinkID);

	//获取设备信息
	typedef BOOL(JCSDK_CALLTYPE *JCSDK_GetDeviceInfo)(JCLink_t nLinkID, PJCDeviceInfo pDevInfo);

	//解码预览-------------------------------------------------------------------------------
	//开启解码器
	typedef BOOL(JCSDK_CALLTYPE *JCSDK_EnableDecoder)(JCLink_t nLinkID, BOOL bEnable);

	//设置视频预览
	typedef BOOL(JCSDK_CALLTYPE *JCSDK_SetVideoPreview)(JCLink_t nLinkID, HWND hWnd, LPRECT pRect);

	//设置音频预览
	typedef BOOL(JCSDK_CALLTYPE *JCSDK_SetAudioPreview)(JCLink_t nLinkID, HWND hWnd);

	//获取码流信息
	typedef BOOL(JCSDK_CALLTYPE *JCSDK_GetStreamInfo)(JCLink_t nLinkID, PJCStreamInfo pInfo);

	//录像功能-------------------------------------------------------------------------------
	//开启录像
	typedef BOOL(JCSDK_CALLTYPE *JCSDK_StartRec)(JCLink_t nLinkID, char *szFilename);

	//关闭录像
	typedef BOOL(JCSDK_CALLTYPE *JCSDK_StopRec)(JCLink_t nLinkID);

	//远程操作-------------------------------------------------------------------------------
	//远程录像检索
	typedef BOOL(JCSDK_CALLTYPE *JCSDK_GetRemoteRecFileList)(JCLink_t nLinkID, PJCDateBlock pDate);

	//远程回放
	typedef BOOL(JCSDK_CALLTYPE *JCSDK_RemotePlay)(JCLink_t nLinkID, int nFileID);

	//远程录像下载
	typedef BOOL(JCSDK_CALLTYPE *JCSDK_DownloadRemoteFile)(JCLink_t nLinkID, int nFileID);

	//远程录像控制
	typedef BOOL(JCSDK_CALLTYPE *JCSDK_RemoteRecording)(JCLink_t nLinkID, BOOL bEnable);

	//其他-----------------------------------------------------------------------------------
	//局域网设备搜索
	typedef BOOL(JCSDK_CALLTYPE *JCSDK_LanSeartchDevice)(int nDeviceType, DWORD dwTimeout);

	//截图
	typedef BOOL(JCSDK_CALLTYPE *JCSDK_SaveBitmap)(JCLink_t nLinkID, char *szFilename);

	//远程设置(暂只支持IPC),nLanguage:0中文,1英文
	typedef BOOL(JCSDK_CALLTYPE *JVSDK_RemoteConfig)(JCLink_t nLinkID, int nLanguage);

	typedef void (JCSDK_CALLTYPE *JCSDK_yv12_to_rgb32)(PUCHAR pDest, DWORD dwDestPitch, PUCHAR pSrcY, PUCHAR pSrcU, PUCHAR pSrcV,
													   DWORD dwSrcYPitch, DWORD dwSrcUPitch, DWORD dwSrcVPitch, DWORD dwWidth, DWORD dwHeight);

private:
	struct dll
	{
		JCSDK_InitSDK pInitSDK= nullptr;

		//释放SDK
		JCSDK_ReleaseSDK pReleaseSDK = nullptr;

		//设置回调函数
		JCSDK_RegisterCallback pRegisterCallback = nullptr;

		//连接-----------------------------------------------------------------------------------
		//连接
		JCSDK_Connect pConnect = nullptr;

		//断开连接
		JCSDK_Disconnect pDisconnect= nullptr;

		//获取设备信息
		JCSDK_GetDeviceInfo pGetDeviceInfo= nullptr;

		//解码预览-------------------------------------------------------------------------------
		//开启解码器
		JCSDK_EnableDecoder pEnableDecoder = nullptr;

		//设置视频预览
		JCSDK_SetVideoPreview pSetVideoPreview= nullptr;

		//设置音频预览
		JCSDK_SetAudioPreview pSetAudioPreview = nullptr;

		//获取码流信息
		JCSDK_GetStreamInfo pGetStreamInfo= nullptr;

		//录像功能-------------------------------------------------------------------------------
		//开启录像
		JCSDK_StartRec pStartRec= nullptr;

		//关闭录像
		JCSDK_StopRec pStopRec = nullptr;

		//远程操作-------------------------------------------------------------------------------
		//远程录像检索
		JCSDK_GetRemoteRecFileList pGetRemoteRecFileList= nullptr;

		//远程回放
		JCSDK_RemotePlay pRemotePlay = nullptr;

		//远程录像下载
		JCSDK_DownloadRemoteFile pDownloadRemoteFile = nullptr;

		//远程录像控制
		JCSDK_RemoteRecording pRemoteRecording = nullptr;

		//其他-----------------------------------------------------------------------------------
		//局域网设备搜索
		JCSDK_LanSeartchDevice pLanSeartchDevice = nullptr;

		//截图
		JCSDK_SaveBitmap pSaveBitmap = nullptr;

		//远程设置(暂只支持IPC),nLanguage:0中文,1英文
		JVSDK_RemoteConfig pRemoteConfig = nullptr;

		JCSDK_yv12_to_rgb32 pyv12_to_rgb32 = nullptr;

	} m_apis;

	HMODULE m_library = nullptr;

	bool init_library();
	bool release_library();

public:
	bool init_sdk(int nLocStartPort);
	bool release_sdk();
	bool register_call_back(JCEventCallback_t funEventCallback,
							JCDataCallback_t funDataCallback,
							JCRawDataCallback_t funRawDataCallback,
							JCLanSearchDeviceCallback_t funLanSearchDeviceCallback);
	JCLink_t connect(char *szServer, int nPort, int nChannel,
				 char *szNetUser, char *szPwd, BOOL bNumOrIP, LPVOID pUserData);
	bool disconnect(JCLink_t nLinkID);
	bool get_device_info(JCLink_t nLinkID, PJCDeviceInfo pDevInfo);
	bool enable_decoder(JCLink_t nLinkID, BOOL bEnable);
	bool set_video_preview(JCLink_t nLinkID, HWND hWnd, LPRECT pRect);
	bool set_audio_preview(JCLink_t nLinkID, HWND hWnd);
	bool get_stream_info(JCLink_t nLinkID, PJCStreamInfo pInfo);
	bool start_record(JCLink_t nLinkID, char *szFilename);
	bool stop_record(JCLink_t nLinkID);
	bool get_remote_record_file_list(JCLink_t nLinkID, PJCDateBlock pDate);
	bool remote_play(JCLink_t nLinkID, int nFileID);
	bool download_remote_file(JCLink_t nLinkID, int nFileID);
	bool remote_recording(JCLink_t nLinkID, BOOL bEnable);
	bool search_lan_device(int nDeviceType, DWORD dwTimeout);
	bool save_bitmap(JCLink_t nLinkID, char *szFilename);
	//远程设置(暂只支持IPC),nLanguage:0中文,1英文
	bool remote_config(JCLink_t nLinkID, int nLanguage);
	bool yv12_to_rgb32(PUCHAR pDest, DWORD dwDestPitch, PUCHAR pSrcY, PUCHAR pSrcU, PUCHAR pSrcV,
					   DWORD dwSrcYPitch, DWORD dwSrcUPitch, DWORD dwSrcVPitch, DWORD dwWidth, DWORD dwHeight);

public:
	//sdk_mgr_jovision();
	~sdk_mgr_jovision();
	DECLARE_SINGLETON(sdk_mgr_jovision);

};

}
}
