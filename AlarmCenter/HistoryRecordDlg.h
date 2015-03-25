#pragma once


#include "HistoryRecord.h"
#include "afxwin.h"
// CHistoryRecordDlg dialog

class CHistoryRecordDlg : public CDialogEx
{
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
	BOOL Export(CString excelPath, core::CRecordList& list);
	// Dialog Data
	//{{AFX_DATA(CHistoryDialog)
	enum { IDD = IDD_DIALOG_HISTORY };
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
	CListCtrl	m_listCtrlRecord;
	//}}AFX_DATA


	// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CHistoryRecordDlg)
public:
	virtual BOOL PreTranslateMessage(MSG* pMsg);
protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

	// Implementation
protected:
	BOOL GetDateTimeValue(CDateTimeCtrl& ctrl, CTime& value);
	BOOL PrintRecord(CListCtrl& list);
	void RepositionItems();
	void LoadRecordsBasedOnPage(const int nPage);
	void InsertListContent(const core::HistoryRecord* record);
	void InitListCtrlHeader(void);
	CString GetExcelDriver();
	// Generated message map functions
	//{{AFX_MSG(CHistoryRecordDlg)
	virtual void OnOK();
	afx_msg void OnShowWindow(BOOL bShow, UINT nStatus);
	virtual BOOL OnInitDialog();
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg void OnButtonDeleteAllRecord();
	afx_msg void OnButtonPageFirst();
	afx_msg void OnButtonPageLast();
	afx_msg void OnButtonPageNext();
	afx_msg void OnButtonPagePrev();
	afx_msg void OnUpdateButtonSeperator(CCmdUI* pCmdUI);
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg void OnButtonExport();
	afx_msg void OnSelchangeComboPerpage();
	afx_msg void OnButtonPrint();
	afx_msg void OnButtonSelByDate();
	afx_msg void OnButtonSelAlarmByDate();
	afx_msg void OnDestroy();
	afx_msg void OnClickListRecord(NMHDR* pNMHDR, LRESULT* pResult);
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
public:
	CButton m_btnSelByUser;
	CButton m_btnSelByMachine;
};

