#pragma once

#include <list>

#include "core.h"
namespace core { class alarm_machine; };
namespace imagin { class CTimer; };

class CMFCButtonEx;

namespace gui {
namespace control { 
class CStaticBmp; 
class CStaticColorText;
};



class CButtonEx : private boost::noncopyable
{
	enum CursorInRegion {
		CIR_TEXT,
		CIR_ICON1,
		CIR_ICON2,
		CIR_ICON3,
	};
	typedef gui::control::CStaticBmp CIconEx;
	typedef std::shared_ptr<CIconEx> CIconPtr;
	typedef gui::control::CStaticColorText CColorText;
	typedef std::shared_ptr<CColorText> CColorTextPtr;
	typedef ademco::AdemcoEventObserver<CButtonEx> ObserverType;
	std::shared_ptr<ObserverType> m_observer;
	static const int FLASH_GAP = 1000;
private:
	//control::CButtonST* _button;
	std::shared_ptr<CMFCButtonEx> _button;
	CIconPtr iconOnOffLine_;
	CIconPtr iconStatus_;
	CIconPtr iconExtra_;
	CRect rect4IconExtra_;
	CColorTextPtr color_text_;
	CColorTextPtr text_extra_ = nullptr;
	CRect rect4TextExtra_ = {};
	CursorInRegion last_time_cursor_in_region_ = CIR_TEXT;
	CWnd* _wndParent;
	//DWORD _data;
	//int _ademco_event;
	BOOL _bSwitchColor;
	COLORREF _clrText;
	COLORREF _clrFace;
	std::shared_ptr<imagin::CTimer> _timer;
	std::list<ademco::AdemcoEventPtr> _alarmEventList;
	std::mutex m_lock4AlarmEventList;
	core::alarm_machine_ptr _machine;
	BOOL _bAlarming;
	CButtonEx() {}
public:
	CButtonEx(const wchar_t* text,
			  const RECT& rc,
			  CWnd* parent,
			  UINT id,
			  core::alarm_machine_ptr machine);

	~CButtonEx();
	void OnBnClicked();
	void OnRBnClicked();
	void OnMouseMove();
	void ShowButton(int nCmdShow);
	void OnAdemcoEventResult(const ademco::AdemcoEventPtr& ademcoEvent);
	void OnTimer(UINT nTimerId);
	void OnImaginTimer();
	core::alarm_machine_ptr GetMachine() const { return _machine; }
	void MoveWindow(const CRect& rc, BOOL bRepaint = TRUE);
	bool IsValidButton() const;
protected:
	//void UpdateStatus();
	void clear_alarm_event_list();
	void StartTimer();
	void StopTimer();
	void UpdateButtonText();
	void HandleAdemcoEvent(const ademco::AdemcoEventPtr& ademcoEvent);
	void UpdateIconAndColor(bool online, core::machine_status status);
	void UpdateToolTipText();
};

};

