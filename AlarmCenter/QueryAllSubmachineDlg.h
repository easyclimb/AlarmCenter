#pragma once
#include "afxwin.h"
#include "afxcmn.h"

namespace core { class CAlarmMachine; };
// CQueryAllSubmachineDlg dialog

class CQueryAllSubmachineDlg : public CDialogEx
{
	DECLARE_DYNAMIC(CQueryAllSubmachineDlg)

public:
	CQueryAllSubmachineDlg(CWnd* pParent = NULL);   // standard constructor
	virtual ~CQueryAllSubmachineDlg();
	core::CAlarmMachine* m_machine;
// Dialog Data
	enum { IDD = IDD_DIALOG_QUERY_ALL_SUBMACHINE };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	DECLARE_MESSAGE_MAP()
public:
	CListBox m_list;
	CButton m_btnOk;
	afx_msg void OnBnClickedOk();
	virtual BOOL OnInitDialog();
	CStatic m_staticProgress;
	CStatic m_staticTime;
	CProgressCtrl m_progress;
	BOOL m_bQuerying;
	DWORD m_dwStartTime;
	DWORD m_dwQueryStartTime;
	int m_nRetryTimes;
	CString m_strFmQuery;
	CString m_strFmQeurySuccess;
	CString m_strQueryFailed;
	CStringList m_strList;
	CLock m_lock;
	core::CAlarmMachine* m_curQueryingSubMachine;
	volatile BOOL m_bQuerySuccess;
protected:
	void QueryNextSubMachine();
	void Reset();
	
public:
	afx_msg void OnTimer(UINT_PTR nIDEvent);
	void OnAdemcoEventResult(const ademco::AdemcoEvent* ademcoEvent);
	afx_msg void OnDestroy();
};
