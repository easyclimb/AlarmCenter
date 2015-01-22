#pragma once


// CAlarmmachineDlg dialog

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
};
