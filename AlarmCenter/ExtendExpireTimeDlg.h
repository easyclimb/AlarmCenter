#pragma once
#include "afxdtctl.h"
#include "afxwin.h"


// CExtendExpireTimeDlg dialog

class CExtendExpireTimeDlg : public CDialogEx
{
	DECLARE_DYNAMIC(CExtendExpireTimeDlg)

public:
	
	CExtendExpireTimeDlg(CWnd* pParent = nullptr);   // standard constructor
	virtual ~CExtendExpireTimeDlg();

// Dialog Data
	enum { IDD = IDD_DIALOG_EXTEND_EXPIRE_TIME };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	DECLARE_MESSAGE_MAP()
public:
	std::chrono::system_clock::time_point m_dateTime;
	CDateTimeCtrl m_date;
	CDateTimeCtrl m_time;
	afx_msg void OnBnClickedOk();
	virtual BOOL OnInitDialog();
};
