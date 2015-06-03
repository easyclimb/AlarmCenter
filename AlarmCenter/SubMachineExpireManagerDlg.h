#pragma once
#include "afxcmn.h"

namespace core { class CAlarmMachine; };
// CSubMachineExpireManagerDlg 对话框

class CSubMachineExpireManagerDlg : public CDialogEx
{
	DECLARE_DYNAMIC(CSubMachineExpireManagerDlg)

public:
	CSubMachineExpireManagerDlg(CWnd* pParent = NULL);   // 标准构造函数
	virtual ~CSubMachineExpireManagerDlg();

// 对话框数据
	enum { IDD = IDD_DIALOG_SUBMACHINE_EXPIRE_MANAGER };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 支持

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
};
