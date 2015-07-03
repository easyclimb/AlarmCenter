// SoundPlayer.h: interface for the CSoundPlayer class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_SOUNDPLAYER_H__5A5A8EA4_70E0_4DC8_BDB2_29818A1DA1F6__INCLUDED_)
#define AFX_SOUNDPLAYER_H__5A5A8EA4_70E0_4DC8_BDB2_29818A1DA1F6__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

// RC: Remote controler 遥控器
// 主机返回的声音索引
enum SoundIndex
{
	SI_BUTTON_PRESSED = 0,		// 按键音	0
	SI_SUCCESS,					// 成功
	SI_DINGDONG,				// 叮咚
	SI_ILLEGAL_OPERATION,		// 非法操作
	SI_SECOND,					// 读秒
	SI_EXCEPTION,				// 异常
	SI_QUIT_CONFIG,				// 退出设置
	SI_INPUT_CONFIG_PSW,		// 输入设置密码
	SI_INPUT_NEW_PSW,			// 输入新密码
	SI_INPUT_USER_PSW,			// 输入用户密码
	SI_INPUT_AGAIN,				// 再次输入		10
	SI_ALARM_PHONE,				// "电话报警";
	SI_ALARM_SITE,				// "现场报警";
	SI_SECURE_HALF,				// "半布防";
	SI_SECURE_ALL,				// "全局布防";
	SI_SECURE_CANCEL,			// "撤防";
	SI_INPUT_PHONE_ALARM,		// "请输入报警电话";
	SI_DEL_ALL_PHONE,			// "删除所有电话";
	SI_INPUT_ALARM_TIME,		// "输入报警声响时间";
	SI_INPUT_LEAVE_TIME,		// "输入离开时间";
	SI_INPUT_ENTER_TIME,		// "请输入进入时间";	20
	SI_INPUT_IN_DIAL_TIMES,		// "请输入振铃次数";
	SI_INPUT_OUT_DIAL_TIMES,	// "请输入拨号循环次数";
	SI_HOST_DUIMA,				// "主机进入对码状态";
	SI_PRESS_RC_SECURE_ALL,		// "请按遥控器布防键";
	SI_TRIGGER_DETECTOR,		// "请触发探头";
	SI_DEL_DETECTOR,			// "删除探头";
	SI_DEL_ALL_DETECTOR,		// "删除所有探头";
	SI_AREA_GLOBAL,				// "全局防区";
	SI_AREA_HALF,				// "半局防区";
	SI_AREA_EMERGENCY,			// "紧急报警防区";		30
	SI_AREA_SHIELD,				// "屏蔽防区";
	SI_AREA_DOORRING,			// "门铃防区";
	SI_NEXT,					// "下一个";
	SI_SYSTEM_MALFUNTION,		// "请注意，有非法进入";
	SI_HOST_MODULATION,			// "主机进入调整状态";
	SI_BACK,					// "退回";
	SI_ERROR_INPUT_AGAIN,		// "错误请重新输入";
	SI_INPUT_AREA_PROPERTY,		// "请输入防区属性";
	SI_AREA_ALREADY_EXISTS,		// "该防区已存在";
	SI_RC_ALREADY_EXISTS,		// "该遥控器已存在";	40
	SI_WELCOM,					// "欢迎使用";
	SI_DEL_AREA_FIRST,			// "请先删除该防区";
	SI_DETECTOR_INVALID,		// "请注意有探头无效";
	SI_PHONE_LINE_DISCONNECT,	// "电话线断了";
	SI_HOST_REMOTE_CONTROL,		// "主机远程控制";
	SI_START_LISTEN,			// "开始监听";
	SI_SITE_MUTE,				// "现场静音";
	SI_ON_THE_WAY_DISCONNECT,	// "即将断线";
	SI_ALARM_WISTLE,			// "警笛报警声";
	SI_ALARM_RECORD,			// "录音报警声";		50
	SI_CLEAR_EXCEPTION,			// "清异常指示";
	SI_ALARM,					// "报警";
	SI_INPUT_PHONE_CENTER,		// "输入接警中心电话";
	SI_INPUT_ADEMCO_ID,			// "请输入用户号";
	SI_VIALATION_ENTER,			// "请注意，你已非法进入"
	SI_ATTENTION_ALARMING,		// "请注意，报警了"
	SI_DELETE_PHONE,			// "删除此电话"
	SI_DELETE_REMOTE_RC,		//"删除此遥控"
	SI_DELETE_ALL_RC,			// "删除所有遥控" 59

	SI_WARNING,					// "提示"		60
	SI_AREA_FIRE,				// "火警防区"
	SI_AREA_DURESS,				// "胁迫防区"
	SI_AREA_GAS,				// "煤气防区"
	SI_AREA_WATER,				// "水警防区"
	SI_AREA,					// "防区"
	SI_GAS_LEAKAGE,				// ”煤气泄漏"
	SI_PLEASE_HELP,				// "请救助"
	SI_WATER_LEAKAGE,			// "水泄漏"
	SI_FIRE,					// "有火情"
	SI_0, SI_1, SI_2, SI_3, SI_4,	// 0 ~ 9	70
	SI_5, SI_6, SI_7, SI_8, SI_9,	//			79
	SI_FIRE_TRUCK_SOUND,		// "消防车音效"	80
	SI_GAS_SOUND,				// "煤气音效"
	SI_WATER_SOUND,				// "水警音效"
	SI_ALL_ALARM,				// "全部报警"
	SI_PART_ALARM,				// "部分报警"
	SI_SEL_ALARM_CENTER,		// "接警中心报警选择"	85
	SI_NET_MACHINE,				// "网络主机"	86
	SI_DISCONNECT,				// 已断开连接 87
	SI_MAX,
};


class CSoundPlayer  
{
	typedef struct LOOP_SOUND
	{
		int id;
		SoundIndex si;
		DWORD startTime;
		DWORD lastTime;
		DWORD expireTime;
		LOOP_SOUND() : id(0), si(SI_MAX), startTime(0), lastTime(0), expireTime(0) {}
		LOOP_SOUND(int id, SoundIndex si, DWORD expireTime)
		{
			this->id = id;
			this->si = si;
			this->startTime = GetTickCount();
			this->lastTime = 0;
			this->expireTime = expireTime;
		}
	}LOOP_SOUND, *PLOOP_SOUND;
	typedef CList<PLOOP_SOUND, PLOOP_SOUND&> CLoopSoundList;
public:
	void StopPlay();
	void PlayWavSound(int si);
	void PlayWavSound(SoundIndex si);
	static CSoundPlayer* GetInstance()
	{
		m_lock4Instance.Lock();
		static CSoundPlayer *sp = NULL;
		if (sp == NULL) {
			sp = new CSoundPlayer();
		}
		m_lock4Instance.UnLock();
		return sp;
	}
	virtual ~CSoundPlayer();
protected:
	CSoundPlayer();
	void Play(SoundIndex si);
	void ClearLoopList();
	static DWORD WINAPI ThreadPlay(LPVOID lp);
private:
	//volatile BOOL m_bClearAll;
	BOOL m_bLooping;
	SoundIndex m_siLooping;
	HANDLE m_hThread;
	HANDLE m_hEventExit;
	CList<SoundIndex, SoundIndex&> m_list;
	CRITICAL_SECTION m_cs;
	HANDLE m_hThreadLoop;
	HANDLE m_hEventClear;
	HANDLE m_hEventReloop;
	static CLock m_lock4Instance;
	CLoopSoundList m_loopList;
public:
	void LoopPlay(int ademco_id, SoundIndex si, DWORD expireTime);
	void StopLoopPlay(int ademco_id);
	void PlayWavSoundImmediately(SoundIndex si);
};

#endif // !defined(AFX_SOUNDPLAYER_H__5A5A8EA4_70E0_4DC8_BDB2_29818A1DA1F6__INCLUDED_)
