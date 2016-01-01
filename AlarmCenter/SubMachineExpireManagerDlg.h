#pragma once
#include "afxcmn.h"
#include <list>
#include "afxwin.h"
#include "MyListCtrl.h"

#include "core.h"


// CMachineExpireManagerDlg 对话框

class CMachineExpireManagerDlg : public CDialogEx
{
	DECLARE_DYNAMIC(CMachineExpireManagerDlg)

public:
	CMachineExpireManagerDlg(CWnd* pParent = nullptr);   // 标准构造函数
	virtual ~CMachineExpireManagerDlg();

// 对话框数据
	enum { IDD = IDD_DIALOG_SUBMACHINE_EXPIRE_MANAGER };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 支持

	DECLARE_MESSAGE_MAP()
private:
	std::list<core::CAlarmMachinePtr> m_expiredMachineList;
public:
	//core::CAlarmMachinePtr m_machine;
protected:
	typedef struct tagColAtt {
		int nColIndex;
		CString strColText;
		int nPrintX;
		int nSubItemIndex;
	}COLATT;
	void InsertList(core::CAlarmMachinePtr machine);
	BOOL GetSaveAsFilePath(CString& path);
	BOOL Export(const CString& excelPath);
	CString GetExcelDriver();
	BOOL PrintRecord(CListCtrl &list);
public:
	bool m_bSubMachine = false;
	core::CAlarmMachinePtr m_machine;
	void SetExpiredMachineList(core::CAlarmMachineList& list);
	afx_msg void OnBnClickedOk();
	afx_msg void OnBnClickedCancel();
	CMyListCtrl m_list;
	afx_msg void OnBnClickedButtonExtend();
	virtual BOOL OnInitDialog();
	afx_msg void OnBnClickedButtonAll();
	afx_msg void OnBnClickedButtonAllNot();
	afx_msg void OnBnClickedButtonInvert();
	CStatic m_staticSeldLineNum;
	afx_msg void OnNMClickList1(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void OnBnClickedButtonExportSel();
	afx_msg void OnBnClickedButtonPrintSel();
	afx_msg void OnLvnColumnclickList1(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void OnHdnItemclickList1(NMHDR *pNMHDR, LRESULT *pResult);
};
