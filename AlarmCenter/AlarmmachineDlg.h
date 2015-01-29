#pragma once
#include "afxwin.h"
#include "afxbutton.h"

namespace core { class CAlarmMachine; };
// CAlarmMachineDlg dialog

namespace gui {

class CMapView;
class CAlarmMachineDlg : public CDialogEx
{
	DECLARE_DYNAMIC(CAlarmMachineDlg)

public:
	CAlarmMachineDlg(CWnd* pParent = NULL);   // standard constructor
	virtual ~CAlarmMachineDlg();

// Dialog Data
	enum { IDD = IDD_DIALOG_MACHINE };

protected:
	void CreateMap();
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	DECLARE_MESSAGE_MAP()
public:
	CStatic m_groupControlPanel;
	CStatic m_groupContent;
	void SetMachineInfo(core::CAlarmMachine* machine);
	virtual BOOL OnInitDialog();
	afx_msg void OnDestroy();
	void OnStatusChange(core::MachineStatus status);
private:
	core::CAlarmMachine* m_machine;
	CMapView* m_mapView;
public:
	CMFCButton m_btnArm;
	CMFCButton m_btnDisarm;
	CMFCButton m_btnEmergency;
	CMFCButton m_btnClearMsg;
	CStatic m_staticNet;
	CStatic m_staticStatus;
};






NAMESPACE_END
