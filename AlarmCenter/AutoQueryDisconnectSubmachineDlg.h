﻿#pragma once
#include "afxcmn.h"
#include "afxwin.h"
#include <list>
#include "core.h"

class CAutoQueryDisconnectSubmachineDlg : public CDialogEx
{
	typedef ademco::AdemcoEventObserver<CAutoQueryDisconnectSubmachineDlg> ObserverType;
	std::shared_ptr<ObserverType> m_observer;
	DECLARE_DYNAMIC(CAutoQueryDisconnectSubmachineDlg)

public:
	CAutoQueryDisconnectSubmachineDlg(CWnd* pParent = nullptr);   // standard constructor
	virtual ~CAutoQueryDisconnectSubmachineDlg();

// Dialog Data
	enum { IDD = IDD_DIALOG_QUERY_DISCONN_SUBMACHINE };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	DECLARE_MESSAGE_MAP()
public:
	CProgressCtrl m_progress;
	CButton m_btnOk;
	CStatic m_staticProgress;
	CStatic m_staticTime;
	CListBox m_list;
	std::list<core::alarm_machine_ptr> m_subMachineList;
	std::list<core::alarm_machine_ptr> m_buffList;
	core::alarm_machine_ptr m_curQueryingSubMachine;
	int m_nAutoStartCounter;
	int m_nAutoQuitCounter;
	DWORD m_dwStartTime;
	DWORD m_dwQueryStartTime;
	DWORD m_nRetryTimes;
	BOOL m_bQuerying;
	BOOL m_bQuerySuccess;
	CString m_strFmQuery;
	CString m_strFmQeurySuccess;
	CString m_strQueryFailed;
	CString m_strCancel;
	virtual BOOL OnInitDialog();
	afx_msg void OnTimer(UINT_PTR nIDEvent);
protected:
	void Reset();
	void QueryNextSubmachine();
public:
	afx_msg void OnBnClickedOk();
	void OnAdemcoEventResult(const ademco::AdemcoEventPtr& ademcoEvent);
	afx_msg void OnDestroy();
	CButton m_btnCancel;
	afx_msg void OnBnClickedCancel();
};
