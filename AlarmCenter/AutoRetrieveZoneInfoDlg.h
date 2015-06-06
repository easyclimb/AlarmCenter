#pragma once
#include "afxwin.h"
#include "afxcmn.h"

namespace core { class CAlarmMachine; };
// CAutoRetrieveZoneInfoDlg 对话框

class CAutoRetrieveZoneInfoDlg : public CDialogEx
{
	DECLARE_DYNAMIC(CAutoRetrieveZoneInfoDlg)

public:
	CAutoRetrieveZoneInfoDlg(CWnd* pParent = NULL);   // 标准构造函数
	virtual ~CAutoRetrieveZoneInfoDlg();

// 对话框数据
	enum { IDD = IDD_DIALOG_AUTO_RETRIEVE_ZONE_INFO };
	core::CAlarmMachine* m_machine;
protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 支持

	DECLARE_MESSAGE_MAP()
private:
	
	DWORD m_dwStartTime;
protected:
	bool RetrieveZoneInfo(int zoneValue, CString& msg);
public:
	afx_msg void OnBnClickedOk();
	virtual BOOL OnInitDialog();
	CButton m_btnStart;
	CProgressCtrl m_progress;
	CStatic m_staticProgress;
	CStatic m_staticTime;
	afx_msg void OnBnClickedButtonStart();
	CListBox m_listctrl;
	afx_msg void OnTimer(UINT_PTR nIDEvent);
};
