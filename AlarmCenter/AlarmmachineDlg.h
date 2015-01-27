#pragma once
#include "afxwin.h"
#include "afxbutton.h"

namespace core { class CAlarmMachine; };
// CAlarmmachineDlg dialog

namespace gui {
class CAlarmmachineDlg : public CDialogEx
{
	DECLARE_DYNAMIC(CAlarmmachineDlg)

public:
	CAlarmmachineDlg(CWnd* pParent = NULL);   // standard constructor
	virtual ~CAlarmmachineDlg();

// Dialog Data
	enum { IDD = IDD_DIALOG_MACHINE };

protected:
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
public:
	CMFCButton m_btnArm;
	CMFCButton m_btnDisarm;
	CMFCButton m_btnEmergency;
};






NAMESPACE_END
