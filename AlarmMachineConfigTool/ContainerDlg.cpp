// ContainerDlg.cpp : implementation file
//

#include "stdafx.h"
#include "AlarmMachineConfigTool.h"
#include "ContainerDlg.h"
#include "afxdialogex.h"
#include "ScrollHelper.h"

// CContainerDlg dialog

IMPLEMENT_DYNAMIC(CContainerDlg, CDialogEx)

CContainerDlg::CContainerDlg(CWnd* pParent /*=NULL*/)
	: CDialogEx(CContainerDlg::IDD, pParent)
	, m_scrollHelper(NULL)
{
	m_scrollHelper = new gui::control::CScrollHelper();
	m_scrollHelper->AttachWnd(this);
}

CContainerDlg::~CContainerDlg()
{
	delete m_scrollHelper;
}

void CContainerDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
}


BEGIN_MESSAGE_MAP(CContainerDlg, CDialogEx)
	ON_BN_CLICKED(IDOK, &CContainerDlg::OnBnClickedOk)
	ON_WM_HSCROLL()
	ON_WM_VSCROLL()
	ON_WM_MOUSEWHEEL()
	ON_WM_SIZE()
END_MESSAGE_MAP()


// CContainerDlg message handlers


void CContainerDlg::OnBnClickedOk()
{
}


void CContainerDlg::OnHScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar)
{
	m_scrollHelper->OnHScroll(nSBCode, nPos, pScrollBar);
}


void CContainerDlg::OnVScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar)
{
	m_scrollHelper->OnVScroll(nSBCode, nPos, pScrollBar);
}


BOOL CContainerDlg::OnMouseWheel(UINT nFlags, short zDelta, CPoint pt)
{
	BOOL wasScrolled = m_scrollHelper->OnMouseWheel(nFlags, zDelta, pt);
	return wasScrolled;
}


void CContainerDlg::OnSize(UINT nType, int cx, int cy)
{
	CDialogEx::OnSize(nType, cx, cy);
	m_scrollHelper->OnSize(nType, cx, cy);
}

