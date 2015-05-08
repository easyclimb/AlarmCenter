#pragma once
#include "afxwin.h"

#include "MyWebBrowser.h"
// CDialog222222 dialog

class CDialog222222 : public CDialogEx
{
	DECLARE_DYNAMIC(CDialog222222)

public:
	CDialog222222(CWnd* pParent = NULL);   // standard constructor
	virtual ~CDialog222222();

// Dialog Data
	enum { IDD = IDD_DIALOG2 };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	DECLARE_MESSAGE_MAP()
public:
	CMyWebBrowser m_web;
	virtual BOOL OnInitDialog();
};
