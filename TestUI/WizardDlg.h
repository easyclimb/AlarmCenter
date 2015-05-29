#pragma once


// CWizardDlg dialog

class CWizardDlg : public CPropertyPage
{
	DECLARE_DYNAMIC(CWizardDlg)

public:
	CWizardDlg();
	virtual ~CWizardDlg();

// Dialog Data
	enum { IDD = IDD_DIALOG_WIZARD };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	DECLARE_MESSAGE_MAP()
};
