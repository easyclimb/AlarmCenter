#if !defined(AFX_LISTBOXEX_H__54726234_C6C5_4892_9B36_0416146A6BBA__INCLUDED_)
#define AFX_LISTBOXEX_H__54726234_C6C5_4892_9B36_0416146A6BBA__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// ListBoxEx.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CListBoxEx window
namespace gui {
namespace control {

class CListBoxEx : public CListBox
{
// Construction
public:
	CListBoxEx();

// Attributes
public:
	void SetTooltipText(LPCTSTR lpszText, BOOL bActivate);
	void SetTooltipText(int nText, BOOL bActivate);
// Operations
public:

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CListBoxEx)
	public:
	virtual BOOL PreTranslateMessage(MSG* pMsg);
	//}}AFX_VIRTUAL

// Implementation
public:
	void InitToolTip();
	virtual ~CListBoxEx();

	// Generated message map functions
protected:
	//{{AFX_MSG(CListBoxEx)
	afx_msg void OnMouseMove(UINT nFlags, CPoint point);
	//}}AFX_MSG

	DECLARE_MESSAGE_MAP()
private:
	CToolTipCtrl m_ToolTip;
	int m_nLastHit;
};

NAMESPACE_END
NAMESPACE_END
/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_LISTBOXEX_H__54726234_C6C5_4892_9B36_0416146A6BBA__INCLUDED_)
