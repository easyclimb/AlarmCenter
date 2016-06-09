#pragma once
#include "video.h"

namespace video {

namespace jovision {

namespace detail {

#define JCSDK_CALLTYPE __stdcall

#define JCSDK_INVALID_LINKVALUE -1


#define JCMAX_LINKNUM 512



}

using namespace video::jovision::detail;

class sdk_mgr_jovision : public dp::singleton<sdk_mgr_jovision>
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
	//DECLARE_SINGLETON(sdk_mgr_jovision);

protected:

	sdk_mgr_jovision();
};

}
}
