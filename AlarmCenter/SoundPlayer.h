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
		SI_OFFLINE,		// ����
		SI_BUGLAR,		// ����
		SI_DOORRING,	// ����
		SI_FIRE,		// ��
		SI_WATER,		// ˮ��
		SI_GAS,			// ú��
		SI_PLEASE_HELP,	// в��
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
