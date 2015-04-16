#pragma once
#include "afxcmn.h"
#include "afxwin.h"
#include <list>

// CLoadFromDBProgressDlg dialog

class CLoadFromDBProgressDlg : public CDialogEx
{
	DECLARE_DYNAMIC(CLoadFromDBProgressDlg)
	typedef struct PROGRESS_EX
	{
		bool _main;
		int _progress;
		int _value;
		int _total;
		PROGRESS_EX(bool main, int progress, int value, int total) :
			_main(main), _progress(progress), _value(value), _total(total) {}
	}PROGRESS_EX, *PPROGRESS_EX;
	std::list<PPROGRESS_EX> m_progressList;
	CLock m_lock4Progress;
public:
	CLoadFromDBProgressDlg(CWnd* pParent = NULL);   // standard constructor
	virtual ~CLoadFromDBProgressDlg();

// Dialog Data
	enum { IDD = IDD_DIALOG_PROGRESS };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	DECLARE_MESSAGE_MAP()
public:
	void AddProgress(bool main, int progress, int value, int total);
	afx_msg void OnBnClickedOk();
	afx_msg void OnBnClickedCancel();
	CProgressCtrl m_progress;
	CStatic m_staticNote;
	virtual BOOL OnInitDialog();
	CStatic m_staticNote2;
	CProgressCtrl m_progress2;
	CStatic m_staticTime;
	DWORD m_dwStartTime;
	DWORD m_dwCheckTime;
	HANDLE m_hThread;
	afx_msg void OnDestroy();
	afx_msg LRESULT OnProgressEx(WPARAM wParam, LPARAM lParam);
	afx_msg void OnClose();
	afx_msg void OnTimer(UINT_PTR nIDEvent);
};
