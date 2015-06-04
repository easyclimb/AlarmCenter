#pragma once
#include "afxcmn.h"
#include <list>
#include "afxwin.h"

namespace core { class CAlarmMachine; };
// CMachineExpireManagerDlg 对话框

class CMachineExpireManagerDlg : public CDialogEx
{
	DECLARE_DYNAMIC(CMachineExpireManagerDlg)

public:
	CMachineExpireManagerDlg(CWnd* pParent = NULL);   // 标准构造函数
	virtual ~CMachineExpireManagerDlg();

// 对话框数据
	enum { IDD = IDD_DIALOG_SUBMACHINE_EXPIRE_MANAGER };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 支持

	DECLARE_MESSAGE_MAP()
private:
	std::list<core::CAlarmMachine*> m_expiredMachineList;
public:
	//core::CAlarmMachine* m_machine;
protected:
	void InsertList(const core::CAlarmMachine* machine);
	BOOL GetSaveAsFilePath(CString& path);
	BOOL CMachineExpireManagerDlg::Export(const CString& excelPath);
	CString GetExcelDriver();
public:
	void SetExpiredMachineList(std::list<core::CAlarmMachine*>& list);
	afx_msg void OnBnClickedOk();
	afx_msg void OnBnClickedCancel();
	CListCtrl m_list;
	afx_msg void OnBnClickedButtonExtend();
	virtual BOOL OnInitDialog();
	afx_msg void OnBnClickedButtonAll();
	afx_msg void OnBnClickedButtonAllNot();
	afx_msg void OnBnClickedButtonInvert();
	afx_msg void OnNMCustomdrawList1(NMHDR *pNMHDR, LRESULT *pResult);
	CStatic m_staticSeldLineNum;
	afx_msg void OnNMClickList1(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void OnBnClickedButtonExportSel();
	afx_msg void OnBnClickedButtonPrintSel();
};
