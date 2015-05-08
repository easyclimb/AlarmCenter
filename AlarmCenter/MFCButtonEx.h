#pragma once
#include "afxbutton.h"

// namespace gui {
// namespace control {

enum ButtonClick
{
	BC_LEFT,
	BC_RIGHT,
};

typedef void (__stdcall *ButtonClkCB)(ButtonClick bc, void* udata);
typedef void (__stdcall *TimerProcEx)(void* udata);
// CMFCButtonEx

class CMFCButtonEx : public CMFCButton
{
	DECLARE_DYNAMIC(CMFCButtonEx)

public:
	CMFCButtonEx();
	virtual ~CMFCButtonEx();
	
protected:
	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnBnClicked();
	afx_msg void OnRButtonDown(UINT nFlags, CPoint point);
	afx_msg void OnRButtonUp(UINT nFlags, CPoint point);
	afx_msg void OnBnKillfocus();
	//afx_msg HBRUSH OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor);
private:
	BOOL m_bRbtnDown;
	ButtonClkCB _buttonCb;
	void* _udata;
	COLORREF m_clrFace;
	COLORREF m_clrText;
	void *m_timerData;
	TimerProcEx m_timerCB;
public:
	void SetButtonClkCallback(ButtonClkCB cb, void* udata) {
		_buttonCb = cb;
		_udata = udata;
	}

	void SetTimerEx(void* udata, TimerProcEx cb);
	void KillTimerEx();
	//void SetFaceColor(COLORREF clr) { m_clrFace = clr; /*Invalidate();*/ }
	//void SetTextColor(COLORREF clr) { m_clrText = clr; /*Invalidate();*/ }
	afx_msg void OnTimer(UINT_PTR nIDEvent);
};

// NAMESPACE_END
// NAMESPACE_END
