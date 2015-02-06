#pragma once

#include <list>
namespace core { class CAlarmMachine; };
namespace imagin { class CTimer; };
namespace gui {
namespace control {
	class CButtonST;
	class CMFCButtonEx;
};


class CButtonEx
{
	static const int FLASH_GAP = 1000;
	DECLARE_UNCOPYABLE(CButtonEx)
private:
	//control::CButtonST* _button;
	control::CMFCButtonEx* _button;
	CWnd* _wndParent;
	//DWORD _data;
	//int _ademco_event;
	BOOL _bRed;
	imagin::CTimer* _timer;
	std::list<ademco::AdemcoEvent*> _alarmEventList;
	CLock m_lock4AlarmEventList;
	core::CAlarmMachine* _machine;
	CButtonEx() {}
public:
	CButtonEx(const wchar_t* text,
			  const RECT& rc,
			  CWnd* parent,
			  UINT id,
			  core::CAlarmMachine* machine);

	~CButtonEx();
	void OnBnClicked();
	void OnRBnClicked();
	void ShowWindow(int nCmdShow);
	void OnAdemcoEventResult(const ademco::AdemcoEvent* ademcoEvent);
	void OnTimer();
	//static bool IsStandardStatus(int status);
protected:
	//void UpdateStatus();
	void clear_alarm_event_list();
};

NAMESPACE_END

