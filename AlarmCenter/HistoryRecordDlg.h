#pragma once



#include "afxwin.h"
#include "MyListCtrl.h"
#include "HistoryRecord.h"
// CHistoryRecordDlg dialog
#include "core.h"
#include "C:\dev\AlarmCenter\AlarmCenter\GridCtrl_src\GridCtrl.h"
#include "afxcmn.h"
class CDatabase;

namespace core {
class alarm_info;
typedef std::shared_ptr<alarm_info> alarm_ptr;

}

class CHistoryRecordDlg : public CDialogEx
{
	class TraverseRecordObserver;
	std::shared_ptr<TraverseRecordObserver> m_traverse_record_observer;

	class ShowRecordObserver;
	std::shared_ptr<ShowRecordObserver> m_show_record_observer;

	class ShowAlarmObserver;
	std::shared_ptr<ShowAlarmObserver> show_alarm_obs_ = {};

	class CurUserChangedObserver;
	std::shared_ptr<CurUserChangedObserver> m_cur_user_changed_observer;


	DECLARE_DYNAMIC(CHistoryRecordDlg)
	

	// Construction
	CWnd* m_parent = nullptr;
public:
	CHistoryRecordDlg(CWnd* pParent = nullptr);   // standard constructor
	void MySonYourFatherIsAlarmMachineDlg(CWnd* parent);

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
	CTabCtrl m_tab;
	gui::control::grid_ctrl::CGridCtrl m_grid;
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

	typedef void(__stdcall *TraverseHistoryRecordCB)(void* udata);

	void InitData();
	void ClearListCtrlAndFreeData();
	BOOL GetDateTimeValue(CDateTimeCtrl& ctrl, CTime& value);
	BOOL PrintRecord(CListCtrl& list);
	void RepositionItems();
	void LoadRecordsBasedOnPage(const int nPage);
	void refresh_pages();
	void InsertListContent(const core::history_record_ptr& record);
	void insert_grid_content(const core::alarm_ptr& alarm);
	void InitListCtrlHeader(void);
	void init_grid();
	CString GetExcelDriver();
	BOOL GetSaveAsFilePath(CString& path);
	void OnExportTraverseHistoryRecord(const core::history_record_ptr& record);
	void OnExportTraverseAlarmRecord(const core::alarm_ptr& record);
	BOOL Export(const CString& excelPath, TraverseHistoryRecordCB cb);
	BOOL GetBegEndDateTime(CString& strBeg, CString& strEnd);
	CString GetRecordLevelString(core::record_level level);

public:
	static void __stdcall OnExportHistoryRecordCB(void* udata, const core::history_record_ptr& record);
	static void __stdcall ExportTraverseSeledHistoryRecord(void* udata);

	static void __stdcall OnExportAlarmRecordCB(void* udata, const core::alarm_ptr& record);

private:
	enum {
		show_history,
		show_alarm,
	} show_what_ = show_history;

	int m_nPageCur;
	int m_nPageTotal;
	int m_nPerPage;
	int page_cur_ = 0;
	int page_total_ = 0;
	int per_page_ = 30;
	HICON m_hIcon;
	BOOL m_bDraging;
	CPoint m_ptBeg;
	CPoint m_ptEnd;
	CDC* m_dcList;
	CTime m_startTime;
	CTime m_currentTime;
	CDatabase* m_pDatabase;
	CImageList m_img_list;

protected:
	DECLARE_MESSAGE_MAP()
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
	afx_msg void OnButtonSelByDate();
	afx_msg void OnButtonSelByLevelAndDate();
	afx_msg void OnDestroy();
	afx_msg void OnBnClickedButtonExportSel();
	afx_msg void OnBnClickedButtonSelByUser();
	afx_msg void OnBnClickedButtonSelByMachine();
	afx_msg void OnBnClickedButtonSelAll();
	afx_msg void OnBnClickedButtonSelInvert();
	afx_msg void OnBnClickedButtonSelNone();
	afx_msg void OnBnClickedButtonPrint();
	afx_msg void OnNMRClickListRecord(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void OnClose();
	afx_msg LRESULT OnExitAlarmCenter(WPARAM wParam, LPARAM lParam);
	afx_msg void OnNMDblclkListRecord(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void OnTcnSelchangeTab(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void OnGridDblClick(NMHDR *pNotifyStruct, LRESULT* pResult);
	//}}AFX_MSG
public:
	CButton m_btnSelByGuard;
	afx_msg void OnBnClickedButtonSelByGuard();
};

