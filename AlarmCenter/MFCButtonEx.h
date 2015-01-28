#pragma once
#include "afxbutton.h"

namespace gui {

enum ButtonClick
{
	BC_LEFT,
	BC_RIGHT,
};

typedef void (_stdcall *ButtonClkCB)(ButtonClick bc, void* udata);

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
	afx_msg HBRUSH OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor);
private:
	BOOL m_bRbtnDown;
	ButtonClkCB _buttonCb;
	void* _udata;
	COLORREF m_clrFace;
	COLORREF m_clrText;
public:
	void SetButtonClkCallback(ButtonClkCB cb, void* udata) {
		_buttonCb = cb;
		_udata = udata;
	}
	void SetFaceColor(COLORREF clr) { m_clrFace = clr; /*Invalidate();*/ }
	void SetTextColor(COLORREF clr) { m_clrText = clr; /*Invalidate();*/ }
};

NAMESPACE_END