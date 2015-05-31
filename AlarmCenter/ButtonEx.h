#pragma once

#include <list>
namespace core { class CAlarmMachine; };
namespace imagin { class CTimer; };

class CMFCButtonEx;

namespace gui {
namespace control { class CButtonST; };

class CButtonEx
{
	static const int FLASH_GAP = 1000;
	DECLARE_UNCOPYABLE(CButtonEx)
private:
	//control::CButtonST* _button;
	CMFCButtonEx* _button;
	CWnd* _wndParent;
	//DWORD _data;
	//int _ademco_event;
	BOOL _bSwitchColor;
	COLORREF _clrText;
	COLORREF _clrFace;
	imagin::CTimer* _timer;
	std::list<ademco::AdemcoEvent*> _alarmEventList;
	CLock m_lock4AlarmEventList;
	core::CAlarmMachine* _machine;
	BOOL _bAlarming;
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
	void OnTimer(UINT nTimerId);
	void OnImaginTimer();
	core::CAlarmMachine* GetMachine() const { return _machine; }
	void MoveWindow(const CRect& rc, BOOL bRepaint = TRUE);
	bool IsValidButton() const;
protected:
	//void UpdateStatus();
	void clear_alarm_event_list();
	void StartTimer();
	void StopTimer();
	void UpdateButtonText();
	void HandleAdemcoEvent(const ademco::AdemcoEvent* ademcoEvent);
};

NAMESPACE_END

