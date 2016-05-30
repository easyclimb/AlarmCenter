
// AlarmCenterUpdateToolDlg.h : 头文件
//

#pragma once
#include "afxcmn.h"
#include "afxwin.h"


// CAlarmCenterUpdateToolDlg 对话框
class CAlarmCenterUpdateToolDlg : public CDialogEx
{
// 构造
public:
	CAlarmCenterUpdateToolDlg(CWnd* pParent = NULL);	// 标准构造函数

// 对话框数据
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_ALARMCENTERUPDATETOOL_DIALOG };
#endif

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV 支持


protected:
	void expand(bool b = true);

// 实现
protected:
	HICON m_hIcon;

	// 生成的消息映射函数
	virtual BOOL OnInitDialog();
	afx_msg void OnSysCommand(UINT nID, LPARAM lParam);
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnBnClickedOk();
	afx_msg void OnBnClickedCancel();
	afx_msg void OnClose();
	CProgressCtrl m_progress;
	CStatic m_time;
	CStatic m_static_progress;
	CButton m_btn_see;
	CStatic m_group_detail;
	CListBox m_detail;
	afx_msg void OnBnClickedButtonSee();
	afx_msg void OnTimer(UINT_PTR nIDEvent);
	CButton m_btn_ok;
};
