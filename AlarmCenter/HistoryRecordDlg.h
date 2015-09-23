#pragma once



#include "afxwin.h"
#include "MyListCtrl.h"
// CHistoryRecordDlg dialog
namespace core { class HistoryRecord; };
class CDatabase;

class CHistoryRecordDlg : public CDialogEx
{
	DECLARE_DYNAMIC(CHistoryRecordDlg)
	class CAutoRedrawListCtrl
	{
	public:
		CAutoRedrawListCtrl(CListCtrl& ctrl) : pctrl(NULL)
		{
			ASSERT(ctrl.IsKindOf(RUNTIME_CLASS(CListCtrl)));
			ctrl.SetRedraw(FALSE);
			pctrl = &ctrl;
		}
		~CAutoRedrawListCtrl()
		{
			if (pctrl) {
				ASSERT(pctrl->IsKindOf(RUNTIME_CLASS(CListCtrl)));
				pctrl->SetRedraw();
			}
		}
	private:
		CAutoRedrawListCtrl() {}
		CAutoRedrawListCtrl(const CAutoRedrawListCtrl&) {}
		CListCtrl* pctrl;
	};
	typedef struct tagColAtt
	{
		int nColIndex;
		CString strColText;
		int nPrintX;
		int nSubItemIndex;
	}COLATT;
	// Construction
public:
	CHistoryRecordDlg(CWnd* pParent = NULL);   // standard constructor
	
	// Dialog Data
	//{{AFX_DATA(CHistoryDialog)
	enum { IDD = IDD_DIALOG_HISTORY };
	int m_ademco_id;
	int m_zone_value;
	CButton m_btnSelAlarmByDate;
	CButton m_btnSelByDate;
	CDateTimeCtrl m_begDate;
	CDateTimeCtrl m_begTime;
	CDateTimeCtrl m_endDate;
	CDateTimeCtrl m_endTime;
	CButton m_staticPerPage;
	CComboBox m_cmbPerPage;
	CButton m_btnPrint;
	CButton	m_btnExport;
	CButton	m_btnPrev;
	CButton	m_btnNext;
	CButton	m_btnLast;
	CButton	m_btnFirst;
	CButton	m_btnClr;
	CStatic	m_page;
	CMyListCtrl	m_listCtrlRecord;
	CButton m_btnSelByUser;
	CButton m_btnSelByMachine;
	CButton m_btnExportSel;
	CButton m_btnSelAll;
	CButton m_btnSelInvert;
	CButton m_btnSelNone;
	//}}AFX_DATA
	// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CHistoryRecordDlg)
public:
//	virtual BOOL PreTranslateMessage(MSG* pMsg);
protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

	// Implementation
protected:
	void ClearListCtrlAndFreeData();
	BOOL GetDateTimeValue(CDateTimeCtrl& ctrl, CTime& value);
	BOOL PrintRecord(CListCtrl& list);
	void RepositionItems();
	void LoadRecordsBasedOnPage(const int nPage);
	void InsertListContent(const core::HistoryRecord* record);
	void InitListCtrlHeader(void);
	CString GetExcelDriver();
	BOOL GetSaveAsFilePath(CString& path);
	void OnExportTraverseHistoryRecord(const core::HistoryRecord* record);
	typedef void(__stdcall *TraverseHistoryRecordCB)(void* udata);
	static void __stdcall ExportTraverseHistoryRecord(void* udata);
	static void __stdcall ExportTraverseSeledHistoryRecord(void* udata);
	BOOL Export(const CString& excelPath, TraverseHistoryRecordCB cb);
	BOOL GetBegEndDateTime(CString& strBeg, CString& strEnd);
public:
	static void __stdcall OnExportHistoryRecordCB(void* udata, 
												  const core::HistoryRecord* record);
	static void __stdcall OnShowHistoryRecordCB(void* udata,
												const core::HistoryRecord* record);
protected:
	// Generated message map functions
	//{{AFX_MSG(CHistoryRecordDlg)
	virtual void OnOK();
	afx_msg void OnShowWindow(BOOL bShow, UINT nStatus);
	virtual BOOL OnInitDialog();
	afx_msg void OnButtonDeleteAllRecord();
	afx_msg void OnButtonPageFirst();
	afx_msg void OnButtonPageLast();
	afx_msg void OnButtonPageNext();
	afx_msg void OnButtonPagePrev();
	afx_msg void OnUpdateButtonSeperator(CCmdUI* pCmdUI);
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg void OnButtonExport();
	afx_msg void OnSelchangeComboPerpage();
	//afx_msg void OnButtonPrint();
	afx_msg void OnButtonSelByDate();
	afx_msg void OnButtonSelByLevelAndDate();
	afx_msg void OnDestroy();
	afx_msg void OnBnClickedButtonExportSel();
	afx_msg void OnBnClickedButtonSelByUser();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
private:
	int m_nPageCur;
	int m_nPageTotal;
	int m_nPerPage;
	HICON m_hIcon;
	BOOL m_bDraging;
	CPoint m_ptBeg;
	CPoint m_ptEnd;
	CDC* m_dcList;
	CTime m_startTime;
	CTime m_currentTime;	
	CDatabase* m_pDatabase;
public:
	afx_msg void OnBnClickedButtonSelByMachine();
	afx_msg void OnBnClickedButtonSelAll();
	afx_msg void OnBnClickedButtonSelInvert();
	afx_msg void OnBnClickedButtonSelNone();
	afx_msg void OnBnClickedButtonPrint();
	afx_msg void OnNMRClickListRecord(NMHDR *pNMHDR, LRESULT *pResult);
};

