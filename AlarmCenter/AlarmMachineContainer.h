#pragma once
#include "resource.h"
#include <list>
#include <afxbutton.h>
// CAlarmMachineContainerDlg dialog

namespace gui { 
	namespace control {
		class CScrollHelper;
	};
	class CButtonEx;
	class CAlarmmachineDlg; 
};
namespace core { class CAlarmMachine; };

class CAlarmMachineContainerDlg : public CDialogEx
{
	DECLARE_DYNAMIC(CAlarmMachineContainerDlg)
public:
	static HICON m_hIconArm;
	static HICON m_hIconDisarm;
	static HICON m_hIconNetOk;
	static HICON m_hIconNetFailed;
	CAlarmMachineContainerDlg(CWnd* pParent = NULL);   // standard constructor
	virtual ~CAlarmMachineContainerDlg();

// Dialog Data
	enum { IDD = IDD_DIALOG_CONTAINER };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	DECLARE_MESSAGE_MAP()
private:
	gui::control::CScrollHelper* m_scrollHelper;
	//core::CAlarmMachineList m_machineList;
	std::list<gui::CButtonEx*> m_machineList;
	gui::CAlarmmachineDlg* m_machineDlg;
public:
	afx_msg void OnHScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar);
	afx_msg void OnVScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar);
	afx_msg BOOL OnMouseWheel(UINT nFlags, short zDelta, CPoint pt);
	afx_msg void OnSize(UINT nType, int cx, int cy);
	BOOL InsertMachine(core::CAlarmMachine* machine);
	virtual BOOL OnInitDialog();
	afx_msg void OnDestroy();
protected:
	afx_msg LRESULT OnBnclkedEx(WPARAM wParam, LPARAM lParam);
};
