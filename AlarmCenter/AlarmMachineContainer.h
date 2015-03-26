#pragma once
#include "resource.h"
#include <list>
#include <afxbutton.h>
// CAlarmMachineContainerDlg dialog

namespace gui { 
namespace control { class CScrollHelper; };
class CButtonEx;
};

namespace core { class CAlarmMachine; class CGroupInfo; };

class CAlarmMachineDlg; 
class CAlarmMachineContainerDlg : public CDialogEx
{
	DECLARE_DYNAMIC(CAlarmMachineContainerDlg)
public:
	
	//static HBITMAP m_hBitmapButton;
	CAlarmMachineContainerDlg(CWnd* pParent = NULL);   // standard constructor
	virtual ~CAlarmMachineContainerDlg();

// Dialog Data
	enum { IDD = IDD_DIALOG_CONTAINER };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	DECLARE_MESSAGE_MAP()
	void ClearButtonList();
	CRect AssignBtnPosition(int ndx);
	void ReAssignBtnPosition();
private:
	gui::control::CScrollHelper* m_scrollHelper;
	//core::CAlarmMachineList m_machineList;
	std::list<gui::CButtonEx*> m_buttonList;
	CAlarmMachineDlg* m_machineDlg;
	core::CGroupInfo* m_curGroupInfo;
	BOOL m_bShowing;
	BOOL m_bFocused;
	CSize m_clientSize;
public:
	virtual BOOL OnInitDialog();
	afx_msg void OnHScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar);
	afx_msg void OnVScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar);
	afx_msg BOOL OnMouseWheel(UINT nFlags, short zDelta, CPoint pt);
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg void OnDestroy();
	afx_msg LRESULT OnBnclkedEx(WPARAM wParam, LPARAM lParam);
	afx_msg HBRUSH OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor);
	afx_msg void OnClose();
public:
	BOOL InsertMachine(core::CAlarmMachine* machine);
	void DeleteMachine(core::CAlarmMachine* machine);
	int GetMachineCount() const { return m_buttonList.size(); }
	
	void ShowMachinesOfGroup(core::CGroupInfo* group);
	
	afx_msg void OnShowWindow(BOOL bShow, UINT nStatus);
	afx_msg void OnMouseMove(UINT nFlags, CPoint point);
	afx_msg void OnMouseLeave();
};
