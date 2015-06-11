// SoundPlayer.h: interface for the CSoundPlayer class.
//
//////////////////////////////////////////////////////////////////////
#pragma once

namespace core {
class CSoundPlayer  
{
public:
	enum SoundIndex
	{
		SI_OFFLINE,		// ¶ÏÏß
		SI_BUGLAR,		// µÁ¾¯
		SI_DOORRING,	// ÃÅÁå
		SI_FIRE,		// »ð¾¯
		SI_WATER,		// Ë®¾¯
		SI_GAS,			// ÃºÆø
		SI_PLEASE_HELP,	// Ð²ÆÈ
		SI_MAX,
	};
	void Stop();
	void Play(SoundIndex si);
	virtual ~CSoundPlayer();
	void IncOffLineMachineNum() { InterlockedIncrement(&m_llOfflineNum); }
	void DecOffLineMachineNum() { if (m_llOfflineNum > 0) InterlockedDecrement(&m_llOfflineNum); }
protected:
	void PlayWavSound(SoundIndex si);
	static DWORD WINAPI ThreadPlay(LPVOID lp);
private:
	volatile SoundIndex m_siLooping;
	volatile unsigned long long m_llOfflineNum;
	HANDLE m_hThread;
	HANDLE m_hEventExit;
	DECLARE_SINGLETON(CSoundPlayer);
};

NAMESPACE_END
