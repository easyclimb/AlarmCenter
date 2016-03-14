﻿// SoundPlayer.h: interface for the CSoundPlayer class.
//
//////////////////////////////////////////////////////////////////////
#pragma once

namespace core {

#define LOOP_PLAY_OFFLINE_SOUND 0

class CSoundPlayer  
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
	virtual ~CSoundPlayer();
#if LOOP_PLAY_OFFLINE_SOUND
	void IncOffLineMachineNum() { InterlockedIncrement(&m_llOfflineNum); }
	void DecOffLineMachineNum() { if (m_llOfflineNum > 0) InterlockedDecrement(&m_llOfflineNum); }
#endif
protected:
	void PlayWavSound(SoundIndex si);
	static DWORD WINAPI ThreadPlay(LPVOID lp);
private:
	volatile SoundIndex m_siLooping;
#if LOOP_PLAY_OFFLINE_SOUND
	volatile LONG m_llOfflineNum;
#endif
	HANDLE m_hThread;
	HANDLE m_hEventExit;
	DECLARE_SINGLETON(CSoundPlayer);
};

NAMESPACE_END
