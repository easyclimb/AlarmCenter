#pragma once
#include "afxcmn.h"
#include "afxwin.h"

#include "core.h"
// CRestoreMachineDlg dialog

class CRestoreMachineDlg : public CDialogEx
{
	typedef ademco::AdemcoEventObserver<CRestoreMachineDlg> ObserverType;
	std::shared_ptr<ObserverType> m_observer;
	DECLARE_DYNAMIC(CRestoreMachineDlg)

public:
	CRestoreMachineDlg(CWnd* pParent = nullptr);   // standard constructor
	virtual ~CRestoreMachineDlg();

// Dialog Data
	enum { IDD = IDD_DIALOG_RESTORE_MACHINE };
	core::CAlarmMachinePtr m_machine;
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
	int m_nZoneCnt;
	CString m_strFmRestore;
	CString m_strFmRestoreZone;
	CString m_strFmRestoreSubmachine;
	CString m_strFmRestoreSuccess;
	CString m_strRestoreFailed;
	CStringList m_strList;
	std::mutex m_lock;
	core::CZoneInfoPtr m_curRestoringZoneInfo;
	volatile BOOL m_bRestoreSuccess;

	afx_msg void OnTimer(UINT_PTR nIDEvent);
	void OnAdemcoEventResult(const ademco::AdemcoEventPtr& ademcoEvent);
	afx_msg void OnBnClickedOk();
	afx_msg void OnDestroy();
};
