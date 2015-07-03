#pragma once
#include <list>

namespace gui { 
namespace control { class CScrollHelper; };
};

// CContainerDlg dialog

class CContainerDlg : public CDialogEx
{
	DECLARE_DYNAMIC(CContainerDlg)

public:
	CContainerDlg(CWnd* pParent = NULL);   // standard constructor
	virtual ~CContainerDlg();

// Dialog Data
	enum { IDD = IDD_DIALOG_CONTAINER };
	gui::control::CScrollHelper* m_scrollHelper;
protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	DECLARE_MESSAGE_MAP()

public:
	afx_msg void OnBnClickedOk();
	afx_msg void OnHScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar);
	afx_msg void OnVScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar);
	afx_msg BOOL OnMouseWheel(UINT nFlags, short zDelta, CPoint pt);
	afx_msg void OnSize(UINT nType, int cx, int cy);
};
