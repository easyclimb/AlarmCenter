#pragma once
#include "afxcmn.h"

namespace core { class CAlarmMachine; };
// CSubMachineExpireManagerDlg �Ի���

class CSubMachineExpireManagerDlg : public CDialogEx
{
	DECLARE_DYNAMIC(CSubMachineExpireManagerDlg)

public:
	CSubMachineExpireManagerDlg(CWnd* pParent = NULL);   // ��׼���캯��
	virtual ~CSubMachineExpireManagerDlg();

// �Ի�������
	enum { IDD = IDD_DIALOG_SUBMACHINE_EXPIRE_MANAGER };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV ֧��

	DECLARE_MESSAGE_MAP()
public:
	core::CAlarmMachine* m_machine;
protected:
	void InsertList(const core::CAlarmMachine* subMachineInfo);
public:
	afx_msg void OnBnClickedOk();
	afx_msg void OnBnClickedCancel();
	CListCtrl m_list;
	afx_msg void OnBnClickedButtonExtend();
	virtual BOOL OnInitDialog();
	afx_msg void OnBnClickedButtonAll();
	afx_msg void OnBnClickedButtonAllNot();
	afx_msg void OnBnClickedButtonInvert();
	afx_msg void OnNMCustomdrawList1(NMHDR *pNMHDR, LRESULT *pResult);
};
