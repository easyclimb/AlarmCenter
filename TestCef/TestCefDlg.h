
// TestCefDlg.h : ͷ�ļ�
//

#pragma once
#include "afxwin.h"


// CTestCefDlg �Ի���
class CTestCefDlg : public CDialogEx
{
// ����
public:
	CTestCefDlg(CWnd* pParent = NULL);	// ��׼���캯��
	virtual ~CTestCefDlg();
// �Ի�������
	enum { IDD = IDD_TESTCEF_DIALOG };

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV ֧��

private:
	//ClientHandler* m_Handler;
// ʵ��
protected:
	HICON m_hIcon;
	//CefRefPtr<CefClient> m_client;
	// ���ɵ���Ϣӳ�亯��
	virtual BOOL OnInitDialog();
	afx_msg void OnSysCommand(UINT nID, LPARAM lParam);
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnDestroy();
protected:
public:
	CStatic m_page;
	afx_msg void OnSize(UINT nType, int cx, int cy);
};
