#pragma once


// CDetectorBindWizardChooseDetPage dialog

class CDetectorBindWizardChooseDetPage : public CPropertyPage
{
	DECLARE_DYNAMIC(CDetectorBindWizardChooseDetPage)

public:
	CDetectorBindWizardChooseDetPage();
	virtual ~CDetectorBindWizardChooseDetPage();

// Dialog Data
	enum { IDD = IDD_PROPPAGE_LARGE_CHOOSE_DET };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	DECLARE_MESSAGE_MAP()
public:
	virtual BOOL OnSetActive();
};
