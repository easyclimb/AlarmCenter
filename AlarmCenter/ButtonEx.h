#pragma once

#include <list>

#include "core.h"
namespace core { class CAlarmMachine; };
namespace imagin { class CTimer; };

class CMFCButtonEx;

namespace gui {
namespace control { 
class CStaticBmp; 
class CStaticColorText;
};



class CButtonEx
{
	typedef gui::control::CStaticBmp CIconEx;
	typedef std::shared_ptr<CIconEx> CIconPtr;
	typedef gui::control::CStaticColorText CColorText;
	typedef std::shared_ptr<CColorText> CColorTextPtr;
	typedef ademco::AdemcoEventObserver<CButtonEx> ObserverType;
	std::shared_ptr<ObserverType> m_observer;
	static const int FLASH_GAP = 1000;
	DECLARE_UNCOPYABLE(CButtonEx)
private:
	//control::CButtonST* _button;
	std::shared_ptr<CMFCButtonEx> _button;
	CIconPtr iconOnOffLine_;
	CIconPtr iconStatus_;
	CIconPtr iconExtra_;
	CColorTextPtr color_text_;
	CWnd* _wndParent;
	//DWORD _data;
	//int _ademco_event;
	BOOL _bSwitchColor;
	COLORREF _clrText;
	COLORREF _clrFace;
	std::shared_ptr<imagin::CTimer> _timer;
	std::list<ademco::AdemcoEventPtr> _alarmEventList;
	std::mutex m_lock4AlarmEventList;
	core::CAlarmMachinePtr _machine;
	BOOL _bAlarming;
	CButtonEx() {}
public:
	CButtonEx(const wchar_t* text,
			  const RECT& rc,
			  CWnd* parent,
			  UINT id,
			  core::CAlarmMachinePtr machine);

	~CButtonEx();
	void OnBnClicked();
	void OnRBnClicked();
	void ShowWindow(int nCmdShow);
	void OnAdemcoEventResult(const ademco::AdemcoEventPtr& ademcoEvent);
	void OnTimer(UINT nTimerId);
	void OnImaginTimer();
	core::CAlarmMachinePtr GetMachine() const { return _machine; }
	void MoveWindow(const CRect& rc, BOOL bRepaint = TRUE);
	bool IsValidButton() const;
protected:
	//void UpdateStatus();
	void clear_alarm_event_list();
	void StartTimer();
	void StopTimer();
	void UpdateButtonText();
	void HandleAdemcoEvent(const ademco::AdemcoEventPtr& ademcoEvent);
	void UpdateIconAndColor(bool online, core::MachineStatus status);
};

NAMESPACE_END

