// SoundPlayer.h: interface for the sound_manager class.
//
//////////////////////////////////////////////////////////////////////
#pragma once

namespace core {


class sound_manager : public dp::singleton<sound_manager>
{
public:
	enum SoundIndex
	{
		SI_OFFLINE,		// 离线
		SI_BUGLAR,		// 盗警
		SI_DOORRING,	// 门铃
		SI_FIRE,		// 火警
		SI_WATER,		// 水警
		SI_GAS,			// 煤气
		SI_PLEASE_HELP,	// 胁迫
		SI_MAX,
	};

	

	void Play(SoundIndex si);
	void Stop();
	
	void AlwaysMute(bool b = true) {
		always_mute_ = b;
	}


	virtual ~sound_manager();
	void IncOffLineMachineNum() { InterlockedIncrement(&m_llOfflineNum); }
	void DecOffLineMachineNum() { if (m_llOfflineNum > 0) InterlockedDecrement(&m_llOfflineNum); }

protected:
	void PlayWavSound(SoundIndex si);
	void ThreadPlay();
	void LoopPlay(SoundIndex si);
	void PlayOnce(SoundIndex si);
	static bool is_alarm_sound(SoundIndex si);
	static bool is_exception_sound(SoundIndex si);


private:
	volatile bool always_mute_ = false;
	volatile SoundIndex m_siLooping;
	volatile LONG m_llOfflineNum;

	std::list<SoundIndex> m_si_list_4_play_once;
	std::mutex m_mutex_4_list_play_once;

	bool running_ = true;
	std::thread thread_ = {};

protected:
	sound_manager();
};

};

