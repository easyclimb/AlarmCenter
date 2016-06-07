// SoundPlayer.h: interface for the sound_manager class.
//
//////////////////////////////////////////////////////////////////////
#pragma once

namespace core {

#define LOOP_PLAY_OFFLINE_SOUND 0

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
	void Stop();
	void LoopPlay(SoundIndex si);
	void PlayOnce(SoundIndex si);
	void AlwayMute(bool b = true) {
		always_mute_ = b;
	}


	virtual ~sound_manager();
#if LOOP_PLAY_OFFLINE_SOUND
	void IncOffLineMachineNum() { InterlockedIncrement(&m_llOfflineNum); }
	void DecOffLineMachineNum() { if (m_llOfflineNum > 0) InterlockedDecrement(&m_llOfflineNum); }
#endif
protected:
	void PlayWavSound(SoundIndex si);
	static DWORD WINAPI ThreadPlay(LPVOID lp);
private:
	volatile bool always_mute_ = false;
	volatile SoundIndex m_siLooping;
#if LOOP_PLAY_OFFLINE_SOUND
	volatile LONG m_llOfflineNum;
#endif
	std::list<SoundIndex> m_si_list_4_play_once;
	std::mutex m_mutex_4_list_play_once;
	HANDLE m_hThread;
	HANDLE m_hEventExit;

	//DECLARE_SINGLETON(sound_manager);

protected:
	sound_manager();
};

};

