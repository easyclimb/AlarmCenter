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

namespace core { class group_info; };

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
	
	CRect AssignBtnPosition(int ndx);
	void ReAssignBtnPosition();
private:
	std::unique_ptr<gui::control::CScrollHelper> m_scrollHelper;
	typedef std::shared_ptr<gui::CButtonEx> CButtonExPtr;
	typedef std::pair<bool, CButtonExPtr> CButtonExWithShowOrHide;
	typedef std::shared_ptr<CAlarmMachineDlg> CAlarmMachineDlgPtr;
	typedef std::pair<CButtonExWithShowOrHide, CAlarmMachineDlgPtr> MachineButtonAndDialog;
	std::map<core::alarm_machine_ptr, MachineButtonAndDialog> m_machineDlgMap;
	core::group_info_ptr m_curGroupInfo;
	//core::alarm_machine_list m_curMachineList;
	std::map<core::group_info_ptr, core::alarm_machine_list> m_groupMap;
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
	BOOL Reset(core::alarm_machine_list& list);
	BOOL InsertMachine(const core::alarm_machine_ptr& machine, int ndx, bool need_check_dup);
	void DeleteMachine(const core::alarm_machine_ptr& machine);
	int GetMachineCount() const { return m_machineDlgMap.size(); }
	void ClearButtonList();
	void Refresh();
	
	void ShowMachinesOfGroup(const core::group_info_ptr& group);
	bool m_bSubmachineContainer = false;
	core::alarm_machine_ptr m_machine;
	afx_msg void OnShowWindow(BOOL bShow, UINT nStatus);
	afx_msg void OnMouseMove(UINT nFlags, CPoint point);
	afx_msg void OnMouseLeave();
protected:
	afx_msg LRESULT OnMsgAdemcoevent(WPARAM wParam, LPARAM lParam);
};
