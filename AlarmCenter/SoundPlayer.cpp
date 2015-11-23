// SoundPlayer.cpp: implementation of the CSoundPlayer class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "SoundPlayer.h"
#include <mmsystem.h>
#pragma comment(lib, "winmm.lib")

namespace core {
//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////
IMPLEMENT_SINGLETON(CSoundPlayer)

CSoundPlayer::CSoundPlayer()
	: m_siLooping(SI_MAX)
	, m_llOfflineNum(0)
	, m_hEventExit(INVALID_HANDLE_VALUE)
{
	m_hEventExit = CreateEvent(nullptr, FALSE, FALSE, nullptr);
	m_hThread = CreateThread(nullptr, 0, ThreadPlay, this, 0, nullptr);
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
	if (m_siLooping != SI_MAX) {
		m_siLooping = SI_MAX;
	} else {
		while (m_llOfflineNum) {
			DecOffLineMachineNum();
		}
	}

}


void CSoundPlayer::PlayWavSound(SoundIndex si)
{
	CString path = _T("");
	path.Format(_T("%s\\SoundFiles\\%d.wav"), GetModuleFilePath(), si);
	if (CFileOper::PathExists(path)) {
		::PlaySound(path, nullptr, SND_FILENAME | SND_SYNC);
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
	AUTO_LOG_FUNCTION;
	CSoundPlayer *player = reinterpret_cast<CSoundPlayer*>(lParam);
	while (1) {
		if (WaitForSingleObject(player->m_hEventExit, 100) == WAIT_OBJECT_0)
			break;

		if (player->m_siLooping < SI_MAX) {
			player->PlayWavSound(player->m_siLooping);
		} else if (player->m_llOfflineNum > 0) {
			player->PlayWavSound(CSoundPlayer::SI_OFFLINE);
		}
	}

	return 0;
}

NAMESPACE_END
