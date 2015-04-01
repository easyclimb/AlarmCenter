#pragma once
#include "afxcmn.h"
#include "afxwin.h"


// CLoadFromDBProgressDlg dialog

class CLoadFromDBProgressDlg : public CDialogEx
{
	DECLARE_DYNAMIC(CLoadFromDBProgressDlg)

public:
	CLoadFromDBProgressDlg(CWnd* pParent = NULL);   // standard constructor
	virtual ~CLoadFromDBProgressDlg();

// Dialog Data
	enum { IDD = IDD_DIALOG_PROGRESS };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnBnClickedOk();
	afx_msg void OnBnClickedCancel();
	CProgressCtrl m_progress;
	CStatic m_staticNote;
	virtual BOOL OnInitDialog();
	CStatic m_staticNote2;
	CProgressCtrl m_progress2;
	CStatic m_staticTime;
	DWORD m_dwStartTime;
	DWORD m_dwCheckTime;
	HANDLE m_hThread;
	afx_msg void OnDestroy();
	afx_msg LRESULT OnProgressEx(WPARAM wParam, LPARAM lParam);
	afx_msg void OnClose();
};
