#pragma once
#include "afxcmn.h"
#include "afxwin.h"

namespace core { class CAlarmMachine; class CZoneInfo; };
// CRestoreMachineDlg dialog

class CRestoreMachineDlg : public CDialogEx
{
	DECLARE_DYNAMIC(CRestoreMachineDlg)

public:
	CRestoreMachineDlg(CWnd* pParent = NULL);   // standard constructor
	virtual ~CRestoreMachineDlg();

// Dialog Data
	enum { IDD = IDD_DIALOG_RESTORE_MACHINE };
	core::CAlarmMachine* m_machine;
protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	DECLARE_MESSAGE_MAP()
	void RestoreNextZone();
	void Reset();
public:
	CProgressCtrl m_progress;
	CStatic m_staticProgress;
	CStatic m_staticTime;
	CButton m_btnOk;
	CListBox m_list;
	virtual BOOL OnInitDialog();

	BOOL m_bRestoring;
	DWORD m_dwStartTime;
	DWORD m_dwRestoreStartTime;
	int m_nRetryTimes;
	CString m_strFmRestore;
	CString m_strFmRestoreZone;
	CString m_strFmRestoreSubmachine;
	CString m_strFmRestoreSuccess;
	CString m_strRestoreFailed;
	CStringList m_strList;
	CLock m_lock;
	core::CZoneInfo* m_curRestoringZoneInfo;
	volatile BOOL m_bRestoreSuccess;

	afx_msg void OnTimer(UINT_PTR nIDEvent);
	void OnAdemcoEventResult(const ademco::AdemcoEvent* ademcoEvent);
	afx_msg void OnBnClickedOk();
};
