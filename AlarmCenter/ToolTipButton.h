#if !defined(AFX_TOOLTIPBUTTON_H__0C6FB64C_A643_4D08_9CF8_52343FA6B0C6__INCLUDED_)
#define AFX_TOOLTIPBUTTON_H__0C6FB64C_A643_4D08_9CF8_52343FA6B0C6__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// ToolTipButton.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CToolTipButton window



class CToolTipButton : public CButton
{
// Construction
public:
	CToolTipButton();
	void SetTooltipText(LPCTSTR lpszText, BOOL bActivate);
	void SetTooltipText(int nText, BOOL bActivate);
	void InitToolTip();
// Attributes
public:

// Operations
public:

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CToolTipButton)
	public:
	virtual BOOL PreTranslateMessage(MSG* pMsg);
	//}}AFX_VIRTUAL

// Implementation
public:
	virtual ~CToolTipButton();

	// Generated message map functions
protected:
	//{{AFX_MSG(CToolTipButton)
		// NOTE - the ClassWizard will add and remove member functions here.
	//}}AFX_MSG

	DECLARE_MESSAGE_MAP()
private:
	CToolTipCtrl m_ToolTip;
	BOOL m_bMouseIn;
public:
	afx_msg void OnMouseMove(UINT nFlags, CPoint point);
	afx_msg void OnMouseLeave();
};

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_TOOLTIPBUTTON_H__0C6FB64C_A643_4D08_9CF8_52343FA6B0C6__INCLUDED_)
