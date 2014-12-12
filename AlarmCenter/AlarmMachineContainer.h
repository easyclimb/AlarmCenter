#pragma once


// CAlarmMachineContainer dialog

class CAlarmMachineContainer : public CDialogEx
{
	DECLARE_DYNAMIC(CAlarmMachineContainer)

public:
	CAlarmMachineContainer(CWnd* pParent = NULL);   // standard constructor
	virtual ~CAlarmMachineContainer();

// Dialog Data
	enum { IDD = IDD_DIALOG_CONTAINER };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	DECLARE_MESSAGE_MAP()
};
