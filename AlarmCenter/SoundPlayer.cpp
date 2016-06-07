// SoundPlayer.cpp: implementation of the sound_manager class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "SoundPlayer.h"
#include <mmsystem.h>
#pragma comment(lib, "winmm.lib")
#include "ConfigHelper.h"

namespace core {
//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////
//IMPLEMENT_SINGLETON(sound_manager)

sound_manager::sound_manager()
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


sound_manager::~sound_manager()
{
	AUTO_LOG_FUNCTION;
	SetEvent(m_hEventExit);
	WaitTillThreadExited(m_hThread);
	CLOSEHANDLE(m_hEventExit);
	CLOSEHANDLE(m_hThread);
}


void sound_manager::LoopPlay(SoundIndex si)
{
	m_siLooping = si;
}


void sound_manager::PlayOnce(SoundIndex si)
{
	//PlayWavSound(si);
	std::lock_guard<std::mutex> lock(m_mutex_4_list_play_once);
	m_si_list_4_play_once.push_back(si);
}


void sound_manager::Stop()
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


void sound_manager::PlayWavSound(SoundIndex si)
{
	auto lang = util::CConfigHelper::get_instance()->get_language();
	CString path = _T("");
	switch (lang) {
	case util::AL_TAIWANESE:
		path.Format(_T("%s\\SoundFiles\\zh-tw\\%d.wav"), GetModuleFilePath(), si);
		break;
	case util::AL_ENGLISH:
		path.Format(_T("%s\\SoundFiles\\en-us\\%d.wav"), GetModuleFilePath(), si);
		break;
	case util::AL_CHINESE:
	default:
		path.Format(_T("%s\\SoundFiles\\zh-cn\\%d.wav"), GetModuleFilePath(), si);
		break;
	}
	
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


DWORD WINAPI sound_manager::ThreadPlay(LPVOID lParam)
{
	AUTO_LOG_FUNCTION;
	sound_manager *player = reinterpret_cast<sound_manager*>(lParam);
	while (1) {
		if (WaitForSingleObject(player->m_hEventExit, 100) == WAIT_OBJECT_0)
			break;

		if (player->m_siLooping < SI_MAX) {
			player->PlayWavSound(player->m_siLooping);
		}
#if LOOP_PLAY_OFFLINE_SOUND 
		else if (player->m_llOfflineNum > 0) {
			player->PlayWavSound(sound_manager::SI_OFFLINE);
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

};
