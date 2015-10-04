#pragma once
#include "afxcmn.h"

namespace core { class CAlarmMachine; };
// CRetrieveProgressDlg dialog

class CRetrieveProgressDlg : public CDialogEx
{
	DECLARE_DYNAMIC(CRetrieveProgressDlg)

public:
	CRetrieveProgressDlg(CWnd* pParent = nullptr);   // standard constructor
	virtual ~CRetrieveProgressDlg();

// Dialog Data
	enum { IDD = IDD_DIALOG_RETRIEVE_PROGRESS };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	DECLARE_MESSAGE_MAP()
public:
	core::CAlarmMachine* m_machine;
	int m_zone;
	int m_gg;
	int m_status;
	int m_addr;
	BOOL m_ok;
	afx_msg void OnBnClickedOk();
	CProgressCtrl m_progress;
	virtual void OnCancel();
	void OnAdemcoEventResult(const ademco::AdemcoEvent* ademcoEvent);
	afx_msg void OnTimer(UINT_PTR nIDEvent);
	virtual BOOL OnInitDialog();
	afx_msg void OnDestroy();
};
