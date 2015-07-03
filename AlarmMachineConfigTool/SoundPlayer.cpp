// SoundPlayer.cpp: implementation of the CSoundPlayer class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "SoundPlayer.h"
#include <mmsystem.h>
#pragma comment(lib, "winmm.lib")

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

//static CSoundPlayer snd;
static const int LOOP_INTERVAL = 2500;
CLock CSoundPlayer::m_lock4Instance;

CSoundPlayer::CSoundPlayer()
	: m_bLooping(FALSE)
	, m_siLooping(SI_MAX)
	, m_hThreadLoop(INVALID_HANDLE_VALUE)
	, m_hEventClear(INVALID_HANDLE_VALUE)
	, m_hEventExit(INVALID_HANDLE_VALUE)
	, m_hEventReloop(INVALID_HANDLE_VALUE)
//, m_bClearAll(FALSE)
{
	InitializeCriticalSection(&m_cs);
	//SHELLEXECUTEINFO ShExecInfo = { 0 };
	//ShExecInfo.cbSize = sizeof(SHELLEXECUTEINFO);
	//ShExecInfo.fMask = SEE_MASK_FLAG_NO_UI;
	//ShExecInfo.hwnd = NULL;
	//ShExecInfo.lpVerb = NULL;
	//ShExecInfo.lpFile = _T("taskkill.exe");
	//ShExecInfo.lpParameters = _T(" /F /IM sound_player.exe");
	//ShExecInfo.lpDirectory = NULL;
	//ShExecInfo.nShow = SW_HIDE;//窗口状态为隐藏
	//ShExecInfo.hInstApp = NULL;
	//ShellExecuteEx(&ShExecInfo);
	//Sleep(500);
#ifdef ENABLE_NONAMEPIPE_SUBPROCESS_SOUND_PLAYER
	SECURITY_ATTRIBUTES                sa;
	PROCESS_INFORMATION                processInfo;
	STARTUPINFO                        startUpInfo;

	sa.bInheritHandle = TRUE;
	sa.lpSecurityDescriptor = NULL;
	sa.nLength = sizeof(SECURITY_ATTRIBUTES);

	if (!CreatePipe(&m_hPipeRead, &m_hPipeWrite, &sa, 0)) {
		MyErrorMsgBox(AfxGetMainWnd()->m_hWnd, _T("创建匿名管道失败"));
		ExitProcess(0);
		return;
	}

	memset(&startUpInfo, 0, sizeof(STARTUPINFO));

	startUpInfo.cb = sizeof(STARTUPINFO);
	startUpInfo.dwFlags = STARTF_USESHOWWINDOW | STARTF_USESTDHANDLES;
	startUpInfo.wShowWindow = SW_HIDE;

	//子进程的标准输入句柄为父进程管道的读数据句柄
	startUpInfo.hStdInput = m_hPipeRead;

	//子进程的标准输出句柄为父进程管道的写数据句柄
	startUpInfo.hStdOutput = m_hPipeWrite;

	//子进程的标准错误处理句柄和父进程的标准错误处理句柄一致
	startUpInfo.hStdError = NULL;

	wchar_t target[MAX_PATH] = { 0 };
	swprintf_s(target, MAX_PATH, L"%s\\sound_player.exe", GetModuleFilePath());
	if (!CreateProcess(target,
		NULL, NULL, NULL, TRUE,
		CREATE_NEW_CONSOLE, NULL, NULL,
		&startUpInfo, &processInfo)) {
		CLOSEHANDLE(m_hPipeRead);
		CLOSEHANDLE(m_hPipeWrite);
		MyErrorMsgBox(NULL, _T("创建子进程sound_player.exe失败"));
		ExitProcess(0);
		return;
	} else {
		//对于 processInfo.hProcess 和 processInfo.hThread 
		//这两个句柄不需要使用，所以释放资源
		CloseHandle(processInfo.hProcess);
		CloseHandle(processInfo.hThread);
	}
#endif

	m_hEventExit = CreateEvent(NULL, FALSE, FALSE, NULL);
	m_hEventClear = CreateEvent(NULL, FALSE, FALSE, NULL);
	m_hEventReloop = CreateEvent(NULL, FALSE, FALSE, NULL);
	m_hThread = CreateThread(NULL, 0, ThreadPlay, this, 0, NULL);
}

CSoundPlayer::~CSoundPlayer()
{
	SetEvent(m_hEventExit);
	WaitTillThreadExited(m_hThread);
	CLOSEHANDLE(m_hEventExit);
	CLOSEHANDLE(m_hThread);
	CLOSEHANDLE(m_hEventClear);
	CLOSEHANDLE(m_hEventReloop);
	DeleteCriticalSection(&m_cs);
}

void CSoundPlayer::PlayWavSound(SoundIndex si)
{
	CLocalLock lock(&m_cs);
	m_list.AddTail(si);
}

void CSoundPlayer::PlayWavSound(int si)
{
	if (si < SI_MAX)
		PlayWavSound(static_cast<SoundIndex>(si));
}

DWORD WINAPI CSoundPlayer::ThreadPlay(LPVOID lParam)
{
	CSoundPlayer *snd = reinterpret_cast<CSoundPlayer*>(lParam);
	TCHAR out[1024];
	_stprintf_s(out, _T("core dump CSoundPlayer::ThreadPlay tid %d"), GetCurrentThreadId());
	OutputDebugString(out);
	while (1) {
		if (WaitForSingleObject(snd->m_hEventExit, 100) == WAIT_OBJECT_0)
			break;

		if (WaitForSingleObject(snd->m_hEventClear, 5) == WAIT_OBJECT_0) {
			EnterCriticalSection(&snd->m_cs);
			snd->m_list.RemoveAll();
			snd->ClearLoopList();
			LeaveCriticalSection(&snd->m_cs);
			continue;
		}

		if (snd->m_list.GetCount() > 0) {
			EnterCriticalSection(&snd->m_cs);
			snd->Play(snd->m_list.RemoveHead());
			LeaveCriticalSection(&snd->m_cs);
		}

		if (WaitForSingleObject(snd->m_hEventReloop, 1) == WAIT_OBJECT_0) {
			EnterCriticalSection(&snd->m_cs);
			snd->ClearLoopList();
			LeaveCriticalSection(&snd->m_cs);
			continue;
		}

		EnterCriticalSection(&snd->m_cs);
		POSITION pos = snd->m_loopList.GetHeadPosition();
		LeaveCriticalSection(&snd->m_cs);
		while (pos) {			
			if (WaitForSingleObject(snd->m_hEventReloop, 1) == WAIT_OBJECT_0) {
				break;
			}
			if (WaitForSingleObject(snd->m_hEventClear, 1) == WAIT_OBJECT_0) {
				SetEvent(snd->m_hEventClear);
				break;
			}

			EnterCriticalSection(&snd->m_cs);
			PLOOP_SOUND pLoop = snd->m_loopList.GetAt(pos);
			if (IsBadWritePtr(pLoop, sizeof(LOOP_SOUND))) {
				LeaveCriticalSection(&snd->m_cs);
				break;
			}

			if (pLoop->id < 0 || pLoop->id >= SI_MAX) {
				LeaveCriticalSection(&snd->m_cs);
				break;
			}

			if (GetTickCount() - pLoop->startTime >= pLoop->expireTime) {
				SAFEDELETEP(pLoop);
				snd->m_loopList.RemoveAt(pos);
				LeaveCriticalSection(&snd->m_cs);
				break;
			} else if(pLoop->lastTime == 0 || (GetTickCount() - pLoop->lastTime > 2500)){
				if (pLoop->id < 0 || pLoop->id >= SI_MAX) {
					LeaveCriticalSection(&snd->m_cs);
					break;
				}
				pLoop->lastTime = GetTickCount();
				snd->Play(pLoop->si);
				snd->m_loopList.GetNext(pos);
			}
			LeaveCriticalSection(&snd->m_cs);
		}
	}

	return 0;
}

void CSoundPlayer::Play(SoundIndex si)
{
	CString path = _T("");
	path.Format(_T("%s\\OriginSoundFiles\\%d.wav"), GetModuleFilePath(), si);
	if (!CFileOper::PathExists(path)) {
		path += _T(" ");
		path += L"File not Found";
		CLog::WriteLog(path);
		return;
	} else {
		::PlaySound(path, NULL, SND_FILENAME | SND_SYNC);
	}
}

void CSoundPlayer::StopPlay()
{
	SetEvent(m_hEventClear);
}


void CSoundPlayer::LoopPlay(int ademco_id, SoundIndex si, DWORD expireTime)
{
	if (si >= SI_MAX)
		return;
	CLocalLock lock(&m_cs);
	PLOOP_SOUND pLoop = new LOOP_SOUND(ademco_id, si, expireTime);
	m_loopList.AddTail(pLoop);
}


void CSoundPlayer::ClearLoopList()
{
	POSITION pos = m_loopList.GetHeadPosition();
	while (pos) {
		PLOOP_SOUND pLoop = m_loopList.GetNext(pos);
		SAFEDELETEP(pLoop);
	}
	m_loopList.RemoveAll();
}


void CSoundPlayer::StopLoopPlay(int ademco_id)
{
	CLocalLock lock(&m_cs);
	POSITION pos = m_loopList.GetHeadPosition();
	while (pos) {
		PLOOP_SOUND pLoop = m_loopList.GetAt(pos);
		if (pLoop->id == ademco_id) {
			SAFEDELETEP(pLoop);
			m_loopList.RemoveAt(pos);
			pos = m_loopList.GetHeadPosition();
			continue;
		}
		m_loopList.GetNext(pos);
	}
	SetEvent(m_hEventReloop);
}


void CSoundPlayer::PlayWavSoundImmediately(SoundIndex si)
{
	CString path = _T("");
	path.Format(_T("%s\\SoundFiles\\%d.wav"), GetModuleFilePath(), si);
	if (!CFileOper::PathExists(path)) {
		path += _T(" ");
		path += L"File not found";
		AfxMessageBox(path);
	} else {
		::PlaySound(path, NULL, SND_FILENAME | SND_SYNC);
	}
}
