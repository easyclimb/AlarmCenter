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
#if LOOP_PLAY_OFFLINE_SOUND
	, m_llOfflineNum(0)
#endif
	, m_si_list_4_play_once()
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


void CSoundPlayer::LoopPlay(SoundIndex si)
{
	m_siLooping = si;
}


void CSoundPlayer::PlayOnce(SoundIndex si)
{
	//PlayWavSound(si);
	std::lock_guard<std::mutex> lock(m_mutex_4_list_play_once);
	m_si_list_4_play_once.push_back(si);
}


void CSoundPlayer::Stop()
{
	if (m_siLooping != SI_MAX) {
		m_siLooping = SI_MAX;
	} 
#if LOOP_PLAY_OFFLINE_SOUND 
	else {
		while (m_llOfflineNum) {
			DecOffLineMachineNum();
		}
	}
#endif

	std::lock_guard<std::mutex> lock(m_mutex_4_list_play_once);
	m_si_list_4_play_once.clear();
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
		JLOG(path);
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
		}
#if LOOP_PLAY_OFFLINE_SOUND 
		else if (player->m_llOfflineNum > 0) {
			player->PlayWavSound(CSoundPlayer::SI_OFFLINE);
		}
#endif
		
		if (!player->m_si_list_4_play_once.empty()) {
			auto si = SI_MAX;
			{
				std::lock_guard<std::mutex> lock(player->m_mutex_4_list_play_once);
				si = player->m_si_list_4_play_once.front();
				player->m_si_list_4_play_once.pop_front();
			}

			if(si != SI_MAX)
				player->PlayWavSound(si);
		}
	}

	return 0;
}

NAMESPACE_END
