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


namespace core {
//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////
IMPLEMENT_SINGLETON(CSoundPlayer)

CSoundPlayer::CSoundPlayer()
	: m_siLooping(SI_MAX)
	, m_hEventExit(INVALID_HANDLE_VALUE)
{
	m_hEventExit = CreateEvent(NULL, FALSE, FALSE, NULL);
	m_hThread = CreateThread(NULL, 0, ThreadPlay, this, 0, NULL);
}


CSoundPlayer::~CSoundPlayer()
{
	AUTO_LOG_FUNCTION;
	SetEvent(m_hEventExit);
	WaitTillThreadExited(m_hThread);
	CLOSEHANDLE(m_hEventExit);
	CLOSEHANDLE(m_hThread);
}


void CSoundPlayer::Play(SoundIndex si)
{
	m_siLooping = si;
}


void CSoundPlayer::Stop()
{
	m_siLooping = SI_MAX;
}


void CSoundPlayer::PlayWavSound(SoundIndex si)
{
	CString path = _T("");
	path.Format(_T("%s\\SoundFiles\\%d.wav"), GetModuleFilePath(), si);
	if (CFileOper::PathExists(path)) {
		::PlaySound(path, NULL, SND_FILENAME | SND_SYNC);
	} else {
		path += _T("\r\n");
		path += L"file not found";
		AfxMessageBox(path);
		CLog::WriteLog(path);
		return;
	}
}


DWORD WINAPI CSoundPlayer::ThreadPlay(LPVOID lParam)
{
	CSoundPlayer *player = reinterpret_cast<CSoundPlayer*>(lParam);
	while (1) {
		if (WaitForSingleObject(player->m_hEventExit, 100) == WAIT_OBJECT_0)
			break;

		if (player->m_siLooping < SI_MAX) {
			player->PlayWavSound(player->m_siLooping);
		}
	}

	return 0;
}

NAMESPACE_END
