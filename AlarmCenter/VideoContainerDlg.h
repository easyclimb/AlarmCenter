#pragma once


// CVideoContainerDlg dialog

class CVideoContainerDlg : public CDialogEx
{
	DECLARE_DYNAMIC(CVideoContainerDlg)

public:
	CVideoContainerDlg(CWnd* pParent = nullptr);   // standard constructor
	virtual ~CVideoContainerDlg();

// Dialog Data
	enum { IDD = IDD_DIALOG_VIDEO_CONTAINER };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnBnClickedOk();
	afx_msg void OnBnClickedCancel();
};
