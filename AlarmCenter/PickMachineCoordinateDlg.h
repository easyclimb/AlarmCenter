#pragma once

#include "baidu.h"
namespace core { class CAlarmMachine; };
class CBaiduMapDlg;
// CPickMachineCoordinateDlg dialog

class CPickMachineCoordinateDlg : public CDialogEx
{
	DECLARE_DYNAMIC(CPickMachineCoordinateDlg)

public:
	CPickMachineCoordinateDlg(CWnd* pParent = NULL);   // standard constructor
	virtual ~CPickMachineCoordinateDlg();

// Dialog Data
	enum { IDD = IDD_DIALOG_PICK_MACHINE_COOR };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	DECLARE_MESSAGE_MAP()
public:
	core::CAlarmMachine* m_machine;
	CBaiduMapDlg* m_map;
	afx_msg void OnBnClickedOk();
	afx_msg void OnBnClickedButtonAutoLocate();
	virtual BOOL OnInitDialog();
	afx_msg void OnDestroy();
	afx_msg void OnBnClickedButtonSetPt();
protected:
	afx_msg LRESULT OnChosenBaiduPt(WPARAM wParam, LPARAM lParam);
public:
	afx_msg void OnBnClickedButtonShowPath();
};