
// AlarmCenterDlg.h : header file
//

#pragma once

//#include "C:/Global/JTL/vector/vector.h"

#include "afxwin.h"
#include "StaticBmp.h"
#include <vector>
//#include "afxbutton.h"

namespace gui { class CButtonST; };

class CAlarmMachineContainerDlg;
// CAlarmCenterDlg dialog
class CAlarmCenterDlg : public CDialogEx
{
// Construction
public:
	CAlarmCenterDlg(CWnd* pParent = NULL);	// standard constructor

// Dialog Data
	enum { IDD = IDD_ALARMCENTER_DIALOG };

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV support


// Implementation
protected:
	HICON m_hIcon;
	
	// Generated message map functions
	virtual BOOL OnInitDialog();
	afx_msg void OnSysCommand(UINT nID, LPARAM lParam);
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
	DECLARE_MESSAGE_MAP()
public:
	void InitAlarmMacines();
private:
	//JTL::Vector<gui::CButtonST*> m_vectorButtons;
	std::vector<gui::CButtonST*> m_vectorButtons;
	CAlarmMachineContainerDlg* m_wndContainer;
	HICON m_hIconComputer;
	HICON m_hIconConnection;
	HICON m_hIconInternet;
public:
	CStatic m_groupMachineList;
	CStatic m_staticSysTime;
	void InitDisplay();
	//CStatic m_staticComputer;
	//CStatic m_staticConnection;
	gui::control::CStaticBmp m_staticInternet;
	afx_msg void OnBnClickedButton1();
	CStatic m_groupControlPanel;
	afx_msg void OnTimer(UINT_PTR nIDEvent);
	afx_msg void OnDestroy();
	CButton* m_btn1;
	//CMFCButton* m_btn1;
};
