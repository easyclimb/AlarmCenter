#pragma once
#include "afxcmn.h"
#include <list>
#include "afxwin.h"


#include "core.h"


#define USE_MFC_GRID_CTRL

#ifdef USE_MFC_GRID_CTRL
#include "GridCtrl_src/GridCtrl.h"
#else
#include "MyListCtrl.h"
#endif


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
	std::list<core::alarm_machine_ptr> m_expiredMachineList;
	bool m_b_initialized_ = false;
protected:
	typedef struct tagColAtt {
		int nColIndex;
		CString strColText;
		int nPrintX;
		int nSubItemIndex;
	}COLATT;
	//void InsertList(const core::alarm_machine_ptr& machine);
	BOOL GetSaveAsFilePath(CString& path);
	BOOL Export(const CString& excelPath);
	CString GetExcelDriver();
	BOOL PrintRecord(CListCtrl &list);
	BOOL UpdateMachineInfo(int row, int col, const CString& txt);
	void RepositionItems();
	void SetExpireTime(CPoint pos);
	void SetRemindTime(CPoint pos);
	void InitializeGrid();
public:
	bool m_bUpdatedMachineName = false;
	bool m_bSubMachine = false;
	core::alarm_machine_ptr m_machine;
	void SetExpiredMachineList(core::alarm_machine_list& list);
	afx_msg void OnBnClickedOk();
	afx_msg void OnBnClickedCancel();
	
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
	afx_msg void OnGridStartEdit(NMHDR *pNotifyStruct, LRESULT* pResult);
	afx_msg void OnGridEndEdit(NMHDR *pNotifyStruct, LRESULT* pResult);
	afx_msg void OnGridItemChanged(NMHDR *pNotifyStruct, LRESULT* pResult);

#ifdef USE_MFC_GRID_CTRL
	gui::control::grid_ctrl::CGridCtrl m_grid;
protected:
	void PrintRecordOnGrid();
#else
	CMyListCtrl m_list;
#endif
public:
	afx_msg void OnSize(UINT nType, int cx, int cy);
	CButton m_btn_all;
	CButton m_btn_all_not;
	CButton m_btn_export_sel_to_excel;
	CButton m_btn_print_sel;
	CButton m_btn_set_sel_remind_time;
	CButton m_btn_extend_sel_expired_time;
	CStatic m_static_label;
	afx_msg void OnSizing(UINT fwSide, LPRECT pRect);
	afx_msg void OnGetMinMaxInfo(MINMAXINFO* lpMMI);
	afx_msg void OnBnClickedButtonSetRemindTime();
	CButton m_btn_set_type;
	CButton m_btn_type_manager;
	afx_msg void OnBnClickedButtonTypeManager();
	afx_msg void OnBnClickedButtonSetType();
};
