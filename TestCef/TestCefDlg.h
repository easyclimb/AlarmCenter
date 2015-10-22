
// TestCefDlg.h : 头文件
//

#pragma once

#include "ClientHandler.h"

// CTestCefDlg 对话框
class CTestCefDlg : public CDialogEx
{
// 构造
public:
	CTestCefDlg(CWnd* pParent = NULL);	// 标准构造函数

// 对话框数据
	enum { IDD = IDD_TESTCEF_DIALOG };

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV 支持

private:
	ClientHandler* m_Handler;
// 实现
protected:
	HICON m_hIcon;
	//CefRefPtr<CefClient> m_client;
	// 生成的消息映射函数
	virtual BOOL OnInitDialog();
	afx_msg void OnSysCommand(UINT nID, LPARAM lParam);
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnDestroy();
};
