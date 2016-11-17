#pragma once
#include "afxcmn.h"
#include "afxwin.h"
#include <memory>
#include "core.h"

// CExportHrProcessDlg dialog
class CDatabase;
//namespace core { class HistoryRecord; };

class CExportHrProcessDlg : public CDialogEx
{
	class TraverseRecordObserver;
	std::shared_ptr<TraverseRecordObserver> m_traverse_record_observer;

	class travers_alarm_observer;
	std::shared_ptr<travers_alarm_observer> m_traverse_alarm_observer;

	DECLARE_DYNAMIC(CExportHrProcessDlg)

public:
	CExportHrProcessDlg(CWnd* pParent = nullptr);   // standard constructor
	virtual ~CExportHrProcessDlg();

// Dialog Data
	enum { IDD = IDD_DIALOG_EXPORT_HR_PROGRESS };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	DECLARE_MESSAGE_MAP()

	bool running_ = false;
	std::thread thread_ = {};
	void ThreadWorker();

public:
	std::shared_ptr<CDatabase> m_pDatabase;
	DWORD m_dwStartTime;
	
	volatile BOOL m_bOver;
	volatile int m_nCurProgress;
	int m_nTotalCount;
	CString m_excelPath;
	BOOL m_bOpenAfterExport;

	bool export_history_ = true;

public:
	afx_msg void OnBnClickedOk();
	afx_msg void OnBnClickedCancel();
	CProgressCtrl m_progress;
	CStatic m_staticProgress;
	CStatic m_staticTime;
	virtual BOOL OnInitDialog();
	afx_msg void OnTimer(UINT_PTR nIDEvent);
	
};
