#pragma once
#include "afxwin.h"


// CEditDetectorDlg dialog

class CEditDetectorDlg : public CDialogEx
{
	DECLARE_DYNAMIC(CEditDetectorDlg)

public:
	CEditDetectorDlg(CWnd* pParent = NULL);   // standard constructor
	virtual ~CEditDetectorDlg();

// Dialog Data
	enum { IDD = IDD_DIALOG_EDIT_DETECTOR };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnBnClickedOk();
	CListBox m_list;
	CButton m_btnBindMap;
	CButton m_btnUnbindMap;
	CButton m_btnBindZone;
	CButton m_btnUnbindZone;
	CComboBox m_cmbAngle;
	CButton m_btnRotateUnticlock;
	CButton m_btnRotateClock;
	CButton m_btnDistanceFar;
	CButton m_btnDistanceNear;
	CButton m_btnMoveUp;
	CButton m_btnMoveLeft;
	CButton m_btnMoveDown;
	CButton m_btnMoveRight;
};
