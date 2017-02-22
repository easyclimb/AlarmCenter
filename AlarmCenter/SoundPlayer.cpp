// SoundPlayer.cpp: implementation of the sound_manager class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "SoundPlayer.h"
#include <mmsystem.h>
#pragma comment(lib, "winmm.lib")
#include "ConfigHelper.h"
#include "C:/dev/Global/win32/MtVerify.h"
#include "C:/dev/Global/win32/mfc/FileOper.h"

namespace core {
//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

sound_manager::sound_manager()
	: m_siLooping(SI_MAX)
	, m_llOfflineNum(0)
	, m_si_list_4_play_once()
{
	thread_ = std::thread(&sound_manager::ThreadPlay, this);

}


sound_manager::~sound_manager()
{
	AUTO_LOG_FUNCTION;
	
	running_ = false;
	thread_.join();
}


void sound_manager::LoopPlay(SoundIndex si)
{
	m_siLooping = si;
}


void sound_manager::PlayOnce(SoundIndex si)
{
	std::lock_guard<std::mutex> lock(m_mutex_4_list_play_once);
	m_si_list_4_play_once.push_back(si);
}


bool sound_manager::is_alarm_sound(SoundIndex si)
{
	switch (si) {
	case core::sound_manager::SI_BUGLAR:
	case core::sound_manager::SI_DOORRING:
	case core::sound_manager::SI_FIRE:
	case core::sound_manager::SI_WATER:
	case core::sound_manager::SI_GAS:
	case core::sound_manager::SI_PLEASE_HELP:
		return true;
		break;
	default:
		return false;
		break;
	}
}

bool sound_manager::is_exception_sound(SoundIndex si)
{
	return si == SI_EXCEPTION;
}

void sound_manager::Play(SoundIndex si)
{
	auto cfg = util::CConfigHelper::get_instance();
	if (si == SI_OFFLINE) {
		if (cfg->get_play_offline_sound()) {
			if (cfg->get_play_offline_loop()) {
				LoopPlay(SI_OFFLINE);
			} else {
				PlayOnce(SI_OFFLINE);
			}
		}
	} else if (is_alarm_sound(si)) {
		if (cfg->get_play_alarm_sound()) {
			if (cfg->get_play_alarm_loop()) {
				LoopPlay(si);
			} else {
				PlayOnce(si);
			}
		}
	} else if (is_exception_sound(si)) {
		if (cfg->get_play_exception_sound()) {
			if (cfg->get_play_exception_loop()) {
				LoopPlay(si);
			} else {
				PlayOnce(si);
			}
		}
	}
}

void sound_manager::Stop()
{
	if (m_siLooping != SI_MAX) {
		m_siLooping = SI_MAX;
	}  else {
		while (m_llOfflineNum) {
			DecOffLineMachineNum();
		}
	}

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


void sound_manager::ThreadPlay()
{
	AUTO_LOG_FUNCTION;
	
	while (running_) {

		std::this_thread::sleep_for(std::chrono::milliseconds(100));

		if (always_mute_) {
			continue;
		}

		if (m_siLooping < SI_MAX) {
			PlayWavSound(m_siLooping);
		} else if (m_llOfflineNum > 0) {
			auto cfg = util::CConfigHelper::get_instance();
			if (cfg->get_play_offline_sound() && cfg->get_play_offline_loop()) {
				PlayWavSound(sound_manager::SI_OFFLINE);
			} else {
				m_llOfflineNum = 0;
			}
		}
		
		if (!m_si_list_4_play_once.empty()) {
			auto si = SI_MAX;

			{
				std::lock_guard<std::mutex> lock(m_mutex_4_list_play_once);
				si = m_si_list_4_play_once.front();
				m_si_list_4_play_once.pop_front();
			}

			if(si != SI_MAX)
				PlayWavSound(si);
		}
	}

}

};
