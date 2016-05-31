#include "stdafx.h"
#include "AlarmCenter.h"
#include "JovisonSdkMgr.h"


namespace video {

namespace jovision {

IMPLEMENT_SINGLETON(CJovisonSdkMgr);


CJovisonSdkMgr::CJovisonSdkMgr()
{
	init_library();
}


CJovisonSdkMgr::~CJovisonSdkMgr()
{
	release_library();
}


bool CJovisonSdkMgr::init_library()
{
	CString path; path.Format(L"%s\\contrib\\jovision", GetModuleFilePath());
	SetCurrentDirectory(path);
	path += L"\\JCSDK.dll";
	// 2015-11-11 20:53:34 sometimes it will failed, so retry some times.
	for (int i = 0; i < 1; i++) {
		m_library = LoadLibrary(path);
		if (m_library == nullptr) {
			JLOG(L"load %s falied #%d, err: %d\n", path, i, GetLastError());
		} else {
			break;
		}
	}
	if (m_library == nullptr) {
		JLOG(L"load %s falied, err: %d\n", path, GetLastError());
		//QuitApplication(9958);
		return false;
	}
	SetCurrentDirectory(GetModuleFilePath());

	bool ok = false;

	do {
		/*m_apis.pInitSdk = (JCSDK_InitSDK)GetProcAddress(m_library, "JCSDK_InitSDK"); 
		if (m_apis.pInitSdk == nullptr) {
			JLOGA("get proc addr faild: %s", "JCSDK_InitSDK"); 
			break; 
		}*/
#define STR(S) #S
#define GET_JC_PROC(proc) m_apis.p##proc = (JCSDK_##proc)GetProcAddress(m_library, STR(JCSDK_##proc)); if (m_apis.p##proc == nullptr) {  JLOGA("get proc addr faild: %s", STR(JCSDK_##proc)); ok = false; break; }
		
		GET_JC_PROC(InitSDK);
		GET_JC_PROC(ReleaseSDK);
		GET_JC_PROC(RegisterCallback);
		GET_JC_PROC(Connect);
		GET_JC_PROC(Disconnect);
		GET_JC_PROC(GetDeviceInfo);
		GET_JC_PROC(EnableDecoder);
		GET_JC_PROC(SetVideoPreview);
		GET_JC_PROC(SetAudioPreview);
		GET_JC_PROC(GetStreamInfo);
		GET_JC_PROC(StartRec);
		GET_JC_PROC(StopRec);
		GET_JC_PROC(GetRemoteRecFileList);
		GET_JC_PROC(RemotePlay);
		GET_JC_PROC(DownloadRemoteFile);
		GET_JC_PROC(RemoteRecording);
		GET_JC_PROC(LanSeartchDevice);
		GET_JC_PROC(SaveBitmap);
		m_apis.pRemoteConfig = (JVSDK_RemoteConfig)GetProcAddress(m_library, "JVSDK_RemoteConfig");
		if (m_apis.pRemoteConfig == nullptr) {
			JLOGA("get proc addr faild: %s", "JVSDK_RemoteConfig");
			break;
		}
		GET_JC_PROC(yv12_to_rgb32);

	} while (0);

	if (!ok) {
		QuitApplication(9958);
		return false;
	}

	return true;
}


bool CJovisonSdkMgr::release_library()
{
	if (m_library && m_apis.pReleaseSDK) {
		m_apis.pReleaseSDK();
		//FreeLibrary(m_library);
		m_library = nullptr;
		return true;
	}
	return false;
}


bool CJovisonSdkMgr::init_sdk(int nLocStartPort)
{
	if (m_apis.pInitSDK) {
		return m_apis.pInitSDK(nLocStartPort) ? true : false;
	}
	return false;
}


bool CJovisonSdkMgr::release_sdk()
{
	if (m_apis.pReleaseSDK) {
		m_apis.pReleaseSDK();
		return true;
	}
	return false;
}


bool CJovisonSdkMgr::register_call_back(JCEventCallback_t funEventCallback,
										JCDataCallback_t funDataCallback,
										JCRawDataCallback_t funRawDataCallback,
										JCLanSearchDeviceCallback_t funLanSearchDeviceCallback)
{
	if (m_apis.pRegisterCallback) {
		m_apis.pRegisterCallback(funEventCallback, funDataCallback, funRawDataCallback, funLanSearchDeviceCallback);
		return true;
	}
	return false;
}


JCLink_t CJovisonSdkMgr::connect(char *szServer, int nPort, int nChannel,
							 char *szNetUser, char *szPwd, BOOL bNumOrIP, LPVOID pUserData)
{
	if (m_apis.pConnect) {
		return m_apis.pConnect(szServer, nPort, nChannel, szNetUser, szPwd, bNumOrIP, pUserData);
	}
	return -1;
}


bool CJovisonSdkMgr::disconnect(JCLink_t nLinkID)
{
	if (m_apis.pDisconnect) {
		return m_apis.pDisconnect(nLinkID) ? true : false;
	}
	return false;
}


bool CJovisonSdkMgr::get_device_info(JCLink_t nLinkID, PJCDeviceInfo pDevInfo)
{
	if (m_apis.pGetDeviceInfo) {
		return m_apis.pGetDeviceInfo(nLinkID, pDevInfo) ? true : false;
	}
	return false;
}


bool CJovisonSdkMgr::enable_decoder(JCLink_t nLinkID, BOOL bEnable)
{
	if (m_apis.pEnableDecoder) {
		return m_apis.pEnableDecoder(nLinkID, bEnable) ? true : false;
	}
	return false;
}


bool CJovisonSdkMgr::set_video_preview(JCLink_t nLinkID, HWND hWnd, LPRECT pRect)
{
	if (m_apis.pSetVideoPreview) {
		return m_apis.pSetVideoPreview(nLinkID, hWnd, pRect) ? true : false;
	}
	return false;
}


bool CJovisonSdkMgr::set_audio_preview(JCLink_t nLinkID, HWND hWnd)
{
	if (m_apis.pSetAudioPreview) {
		return m_apis.pSetAudioPreview(nLinkID, hWnd) ? true : false;
	}
	return false;
}


bool CJovisonSdkMgr::get_stream_info(JCLink_t nLinkID, PJCStreamInfo pInfo)
{
	if (m_apis.pGetStreamInfo) {
		return m_apis.pGetStreamInfo(nLinkID, pInfo) ? true : false;
	}
	return false;
}


bool CJovisonSdkMgr::start_record(JCLink_t nLinkID, char *szFilename)
{
	if (m_apis.pStartRec) {
		return m_apis.pStartRec(nLinkID, szFilename) ? true : false;
	}
	return false;
}


bool CJovisonSdkMgr::stop_record(JCLink_t nLinkID)
{
	if (m_apis.pStopRec) {
		return m_apis.pStopRec(nLinkID) ? true : false;
	}
	return false;
}


bool CJovisonSdkMgr::get_remote_record_file_list(JCLink_t nLinkID, PJCDateBlock pDate)
{
	if (m_apis.pGetRemoteRecFileList) {
		m_apis.pGetRemoteRecFileList(nLinkID, pDate) ? true : false;
	}
	return false;
}


bool CJovisonSdkMgr::remote_play(JCLink_t nLinkID, int nFileID)
{
	if (m_apis.pRemotePlay) {
		return m_apis.pRemotePlay(nLinkID, nFileID) ? true : false;
	}
	return false;
}


bool CJovisonSdkMgr::download_remote_file(JCLink_t nLinkID, int nFileID)
{
	if (m_apis.pDownloadRemoteFile) {
		return m_apis.pDownloadRemoteFile(nLinkID, nFileID) ? true : false;
	}
	return false;
}


bool CJovisonSdkMgr::remote_recording(JCLink_t nLinkID, BOOL bEnable)
{
	if (m_apis.pRemoteRecording) {
		return m_apis.pRemoteRecording(nLinkID, bEnable) ? true : false;
	}
	return false;
}


bool CJovisonSdkMgr::search_lan_device(int nDeviceType, DWORD dwTimeout)
{
	if (m_apis.pLanSeartchDevice) {
		return m_apis.pLanSeartchDevice(nDeviceType, dwTimeout) ? true : false;
	}
	return false;
}


bool CJovisonSdkMgr::save_bitmap(JCLink_t nLinkID, char *szFilename)
{
	if (m_apis.pSaveBitmap) {
		return m_apis.pSaveBitmap(nLinkID, szFilename) ? true : false;
	}
	return false;
}


//Զ������(��ֻ֧��IPC),nLanguage:0����,1Ӣ��
bool CJovisonSdkMgr::remote_config(JCLink_t nLinkID, int nLanguage)
{
	if (m_apis.pRemoteConfig) {
		return m_apis.pRemoteConfig(nLinkID, nLanguage) ? true : false;
	}
	return false;
}


bool CJovisonSdkMgr::yv12_to_rgb32(PUCHAR pDest, DWORD dwDestPitch, PUCHAR pSrcY, PUCHAR pSrcU, PUCHAR pSrcV,
								   DWORD dwSrcYPitch, DWORD dwSrcUPitch, DWORD dwSrcVPitch, DWORD dwWidth, DWORD dwHeight)
{
	if (m_apis.pyv12_to_rgb32) {
		m_apis.pyv12_to_rgb32(pDest, dwDestPitch, pSrcY, pSrcU, pSrcV, dwSrcYPitch, dwSrcUPitch, dwSrcVPitch, dwWidth, dwHeight);
		return true;
	}
	return false;
}




}
}
