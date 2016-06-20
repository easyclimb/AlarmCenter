#pragma once


// CVideoRecordPlayerDlg dialog

class CVideoRecordPlayerDlg : public CDialogEx
{
	DECLARE_DYNAMIC(CVideoRecordPlayerDlg)

public:
	CVideoRecordPlayerDlg(CWnd* pParent = NULL);   // standard constructor
	virtual ~CVideoRecordPlayerDlg();

// Dialog Data
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_DIALOG_VIDEO_RECORD_PLAYER };
#endif

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	DECLARE_MESSAGE_MAP()
public:
	virtual BOOL OnInitDialog();
	afx_msg void OnBnClickedOk();
	afx_msg void OnBnClickedCancel();
	afx_msg void OnClose();
};
