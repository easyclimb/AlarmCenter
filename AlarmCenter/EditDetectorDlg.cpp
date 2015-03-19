// EditDetectorDlg.cpp : implementation file
//

#include "stdafx.h"
#include "AlarmCenter.h"
#include "EditDetectorDlg.h"
#include "afxdialogex.h"


// CEditDetectorDlg dialog

IMPLEMENT_DYNAMIC(CEditDetectorDlg, CDialogEx)

CEditDetectorDlg::CEditDetectorDlg(CWnd* pParent /*=NULL*/)
	: CDialogEx(CEditDetectorDlg::IDD, pParent)
{

}

CEditDetectorDlg::~CEditDetectorDlg()
{
}

void CEditDetectorDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_LIST1, m_list);
	DDX_Control(pDX, IDC_BUTTON_BIND_MAP, m_btnBindMap);
	DDX_Control(pDX, IDC_BUTTON_UNBIND_MAP, m_btnUnbindMap);
	DDX_Control(pDX, IDC_BUTTON_BIND_ZONE, m_btnBindZone);
	DDX_Control(pDX, IDC_BUTTON_UNBIND_ZONE, m_btnUnbindZone);
	DDX_Control(pDX, IDC_COMBO_ANGLE, m_cmbAngle);
	DDX_Control(pDX, IDC_BUTTON_ROTATE_UNTICLOCK, m_btnRotateUnticlock);
	DDX_Control(pDX, IDC_BUTTON_ROTATE_CLOCK, m_btnRotateClock);
	DDX_Control(pDX, IDC_BUTTON_DISTANCE_FAR, m_btnDistanceFar);
	DDX_Control(pDX, IDC_BUTTON_DISTANCE_NEAR, m_btnDistanceNear);
	DDX_Control(pDX, IDC_BUTTON_MOVE_UP, m_btnMoveUp);
	DDX_Control(pDX, IDC_BUTTON_MOVE_LEFT, m_btnMoveLeft);
	DDX_Control(pDX, IDC_BUTTON_MOVE_DOWN, m_btnMoveDown);
	DDX_Control(pDX, IDC_BUTTON_MOVE_RIGHT, m_btnMoveRight);
}


BEGIN_MESSAGE_MAP(CEditDetectorDlg, CDialogEx)
	ON_BN_CLICKED(IDOK, &CEditDetectorDlg::OnBnClickedOk)
END_MESSAGE_MAP()


// CEditDetectorDlg message handlers


void CEditDetectorDlg::OnBnClickedOk()
{
	return;
}
