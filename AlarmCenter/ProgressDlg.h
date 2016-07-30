#pragma once
#include "afxcmn.h"
#include "afxwin.h"
#include <list>
#include "core.h"

// CLoadFromDBProgressDlg dialog

class CLoadFromDBProgressDlg : public CDialogEx
{
	DECLARE_DYNAMIC(CLoadFromDBProgressDlg)
	std::list<core::progress_ex_ptr> m_progressList;
	std::mutex m_lock4Progress;
public:
	CLoadFromDBProgressDlg(CWnd* pParent = nullptr);   // standard constructor
	virtual ~CLoadFromDBProgressDlg();

// Dialog Data
	enum { IDD = IDD_DIALOG_PROGRESS };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	DECLARE_MESSAGE_MAP()

	std::thread thread_ = {};
public:
	void AddProgress(const core::progress_ex_ptr&);
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
	afx_msg void OnDestroy();
	afx_msg void OnClose();
	afx_msg void OnTimer(UINT_PTR nIDEvent);
	CStatic m_static_total_progress;
	CStatic m_static_load_zone_progress;
};
