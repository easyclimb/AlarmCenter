// AlarmMachineContainer.cpp : implementation file
//

#include "stdafx.h"
#include "AlarmCenter.h"
#include "AlarmMachineContainer.h"
#include "afxdialogex.h"


// CAlarmMachineContainer dialog

IMPLEMENT_DYNAMIC(CAlarmMachineContainer, CDialogEx)

CAlarmMachineContainer::CAlarmMachineContainer(CWnd* pParent /*=NULL*/)
	: CDialogEx(CAlarmMachineContainer::IDD, pParent)
{

}

CAlarmMachineContainer::~CAlarmMachineContainer()
{
}

void CAlarmMachineContainer::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
}


BEGIN_MESSAGE_MAP(CAlarmMachineContainer, CDialogEx)
END_MESSAGE_MAP()


// CAlarmMachineContainer message handlers
