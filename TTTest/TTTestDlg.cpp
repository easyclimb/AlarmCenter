// TTTestDlg.cpp : implementation file
//

#include "stdafx.h"
#include "TTTest.h"
#include "TTTestDlg.h"
#include "BalloonMsg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


// CTTTestDlg dialog



CTTTestDlg::CTTTestDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CTTTestDlg::IDD, pParent)
{
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
}

void CTTTestDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_EDIT1, m_ctrlEdit);
	DDX_Text(pDX, IDC_EDIT1, m_strJunk);
	
	if ( pDX->m_bSaveAndValidate )
	{
		if ( m_strJunk.IsEmpty() )
		{
			// Use SafeShowMsg here - if balloons are disabled, we'll see a conventional message box
			pDX->PrepareEditCtrl( IDC_EDIT1 );
			CBalloonMsg::SafeShowMsg( MB_OK | MB_ICONEXCLAMATION, IDS_ERR_TITLE, IDS_ERR_BODY, &m_ctrlEdit );
			pDX->Fail();
		}
		
	}
}

BEGIN_MESSAGE_MAP(CTTTestDlg, CDialog)
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	//}}AFX_MSG_MAP
	ON_BN_CLICKED(IDC_BUTTON1, OnBnClickedButton1)
	ON_EN_CHANGE(IDC_EDIT1, OnEnChangeEdit1)
END_MESSAGE_MAP()


// CTTTestDlg message handlers

BOOL CTTTestDlg::OnInitDialog()
{
	CDialog::OnInitDialog();

	// Set the icon for this dialog.  The framework does this automatically
	//  when the application's main window is not a dialog
	SetIcon(m_hIcon, TRUE);			// Set big icon
	SetIcon(m_hIcon, FALSE);		// Set small icon

	// TODO: Add extra initialization here
	
	return TRUE;  // return TRUE  unless you set the focus to a control
}

// If you add a minimize button to your dialog, you will need the code below
//  to draw the icon.  For MFC applications using the document/view model,
//  this is automatically done for you by the framework.

void CTTTestDlg::OnPaint() 
{
	if (IsIconic())
	{
		CPaintDC dc(this); // device context for painting

		SendMessage(WM_ICONERASEBKGND, reinterpret_cast<WPARAM>(dc.GetSafeHdc()), 0);

		// Center icon in client rectangle
		int cxIcon = GetSystemMetrics(SM_CXICON);
		int cyIcon = GetSystemMetrics(SM_CYICON);
		CRect rect;
		GetClientRect(&rect);
		int x = (rect.Width() - cxIcon + 1) / 2;
		int y = (rect.Height() - cyIcon + 1) / 2;

		// Draw the icon
		dc.DrawIcon(x, y, m_hIcon);
	}
	else
	{
		CDialog::OnPaint();
	}
}

// The system calls this function to obtain the cursor to display while the user drags
//  the minimized window.
HCURSOR CTTTestDlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}

void CTTTestDlg::OnBnClickedButton1()
{
	CBalloonMsg::ShowForCtrl( _T("Test Title"), _T("Test Text"), &m_ctrlEdit, (HICON) 1 );
}

void CTTTestDlg::OnEnChangeEdit1()
// If the user types something, ditch the balloon fast!
{
	CString	strTest;
	//
	
	m_ctrlEdit.GetWindowText( strTest );
	if ( !strTest.IsEmpty() )
		CBalloonMsg::RequestCloseAll();
}
