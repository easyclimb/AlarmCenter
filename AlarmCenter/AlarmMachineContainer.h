#pragma once
#include <list>
#include <map>
#include <memory>
#include <afxbutton.h>
#include "core.h"

namespace gui {
	namespace control { class CScrollHelper; };
	class CButtonEx;
};

namespace core { class CGroupInfo; };

class CAlarmMachineDlg; 
class CAlarmMachineContainerDlg : public CDialogEx
{
	DECLARE_DYNAMIC(CAlarmMachineContainerDlg)
public:
	
	//static HBITMAP m_hBitmapButton;
	CAlarmMachineContainerDlg(CWnd* pParent = nullptr);   // standard constructor
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
	std::unique_ptr<gui::control::CScrollHelper> m_scrollHelper;
	typedef std::shared_ptr<gui::CButtonEx> CButtonExPtr;
	std::list<CButtonExPtr> m_buttonList;
	typedef std::shared_ptr<CAlarmMachineDlg> CAlarmMachineDlgPtr;
	std::map<core::CAlarmMachinePtr, CAlarmMachineDlgPtr> m_machineDlgMap;
	core::CGroupInfoPtr m_curGroupInfo;
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
	BOOL InsertMachine(const core::CAlarmMachinePtr& machine);
	void DeleteMachine(const core::CAlarmMachinePtr& machine);
	int GetMachineCount() const { return m_buttonList.size(); }
	
	void ShowMachinesOfGroup(const core::CGroupInfoPtr& group);
	bool m_bSubmachineContainer = false;
	core::CAlarmMachinePtr m_machine;
	afx_msg void OnShowWindow(BOOL bShow, UINT nStatus);
	afx_msg void OnMouseMove(UINT nFlags, CPoint point);
	afx_msg void OnMouseLeave();
};
