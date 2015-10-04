#pragma once
#include "afxcmn.h"
#include "afxwin.h"


// CExportHrProcessDlg dialog
class CDatabase;
//namespace core { class HistoryRecord; };

class CExportHrProcessDlg : public CDialogEx
{
	DECLARE_DYNAMIC(CExportHrProcessDlg)

public:
	CExportHrProcessDlg(CWnd* pParent = nullptr);   // standard constructor
	virtual ~CExportHrProcessDlg();

// Dialog Data
	enum { IDD = IDD_DIALOG_EXPORT_HR_PROGRESS };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	DECLARE_MESSAGE_MAP()

public:
	CDatabase* m_pDatabase;
	DWORD m_dwStartTime;
	HANDLE m_hThread;
	volatile BOOL m_bOver;
	volatile int m_nCurProgress;
	int m_nTotalCount;
	CString m_excelPath;
	BOOL m_bOpenAfterExport;
protected:
	//static void __stdcall OnExportHistoryRecordCB(void* udata, const core::HistoryRecord* record);
public:
	afx_msg void OnBnClickedOk();
	afx_msg void OnBnClickedCancel();
	CProgressCtrl m_progress;
	CStatic m_staticProgress;
	CStatic m_staticTime;
	virtual BOOL OnInitDialog();

	afx_msg void OnTimer(UINT_PTR nIDEvent);
	static DWORD WINAPI ThreadWorker(LPVOID lp);
};
