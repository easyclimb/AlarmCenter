#pragma once
#include "afxwin.h"

#include "ListBoxST.h"
// CChooseDetDlg dialog

class CChooseDetDlg : public CDialogEx
{
	DECLARE_DYNAMIC(CChooseDetDlg)

public:
	CChooseDetDlg(CWnd* pParent = NULL);   // standard constructor
	virtual ~CChooseDetDlg();

// Dialog Data
	enum { IDD = IDD_DIALOG_CHOOSE_DET_DLG };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	CImageList m_ImageList;
	CImageList m_ImageListRotate;
	DECLARE_MESSAGE_MAP()
public:
	int m_detType2Show;
	int m_chosenDetectorID;
	CStatic m_staticNote;
	gui::control::CListBoxST m_list;
	afx_msg void OnBnClickedOk();
	virtual BOOL OnInitDialog();
	afx_msg void OnLbnSelchangeListDetector();
};
