#pragma once

#include "AlarmMachineContainer.h"
#include "afxwin.h"
#include "afxcmn.h"
// CSearchMachineResultDlg dialog

class CSearchMachineResultDlg : public CDialogEx
{
	DECLARE_DYNAMIC(CSearchMachineResultDlg)

public:
	CSearchMachineResultDlg(CWnd* pParent = NULL);   // standard constructor
	virtual ~CSearchMachineResultDlg();

// Dialog Data
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_DIALOG_SEARCH_MACHINE_RESULT };
#endif

private:
	std::shared_ptr<CAlarmMachineContainerDlg> container_ = nullptr;
	bool init_over_ = false;
	std::wstring last_input_content_ = {};

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	DECLARE_MESSAGE_MAP()

	void reposition_items();
public:
	afx_msg void OnBnClickedOk();
	virtual BOOL OnInitDialog();
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg void OnClose();
	afx_msg void OnBnClickedCancel();
	CEdit m_input;
	afx_msg void OnTimer(UINT_PTR nIDEvent);
	afx_msg void OnGetMinMaxInfo(MINMAXINFO* lpMMI);
	CTabCtrl m_tab;
	CListCtrl m_list;
	afx_msg void OnShowWindow(BOOL bShow, UINT nStatus);
	afx_msg void OnEnChangeEdit1();
	afx_msg void OnTcnSelchangeTab1(NMHDR *pNMHDR, LRESULT *pResult);
};
