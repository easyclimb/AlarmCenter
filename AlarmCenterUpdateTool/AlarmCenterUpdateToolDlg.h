
// AlarmCenterUpdateToolDlg.h : ͷ�ļ�
//

#pragma once
#include "afxcmn.h"
#include "afxwin.h"


// CAlarmCenterUpdateToolDlg �Ի���
class CAlarmCenterUpdateToolDlg : public CDialogEx
{
// ����
public:
	CAlarmCenterUpdateToolDlg(CWnd* pParent = NULL);	// ��׼���캯��

// �Ի�������
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_ALARMCENTERUPDATETOOL_DIALOG };
#endif

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV ֧��


protected:
	void expand(bool b = true);

// ʵ��
protected:
	HICON m_hIcon;

	// ���ɵ���Ϣӳ�亯��
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
