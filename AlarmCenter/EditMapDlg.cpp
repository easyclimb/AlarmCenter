// EditMapDlg.cpp : implementation file
//

#include "stdafx.h"
#include "AlarmCenter.h"
#include "EditMapDlg.h"
#include "afxdialogex.h"


// CEditMapDlg dialog

IMPLEMENT_DYNAMIC(CEditMapDlg, CDialogEx)

CEditMapDlg::CEditMapDlg(CWnd* pParent /*=NULL*/)
	: CDialogEx(CEditMapDlg::IDD, pParent)
{

}

CEditMapDlg::~CEditMapDlg()
{
}

void CEditMapDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_EDIT_ALIAS, m_alias);
	DDX_Control(pDX, IDC_EDIT_FILE, m_file);
	DDX_Control(pDX, IDC_TREE1, m_tree);
	DDX_Control(pDX, IDC_STATIC_PREVIEW, m_preview);
}


BEGIN_MESSAGE_MAP(CEditMapDlg, CDialogEx)
	ON_BN_CLICKED(IDOK, &CEditMapDlg::OnBnClickedOk)
	ON_BN_CLICKED(IDC_BUTTON_ADD_MAP, &CEditMapDlg::OnBnClickedButtonAddMap)
	ON_BN_CLICKED(IDC_BUTTON_DEL_MAP, &CEditMapDlg::OnBnClickedButtonDelMap)
	ON_EN_CHANGE(IDC_EDIT_ALIAS, &CEditMapDlg::OnEnChangeEditAlias)
	ON_BN_CLICKED(IDC_BUTTON_CHANGE_FILE, &CEditMapDlg::OnBnClickedButtonChangeFile)
END_MESSAGE_MAP()


// CEditMapDlg message handlers


void CEditMapDlg::OnBnClickedOk()
{

	CDialogEx::OnOK();
}


void CEditMapDlg::OnBnClickedButtonAddMap()
{

}


void CEditMapDlg::OnBnClickedButtonDelMap()
{

}


void CEditMapDlg::OnEnChangeEditAlias()
{
	
}


void CEditMapDlg::OnBnClickedButtonChangeFile()
{

}
