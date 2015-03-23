#pragma once

namespace core { class CAlarmMachine; };
// CDetectorBindWizardChooseMapPage dialog

class CDetectorBindWizardChooseMapPage : public CPropertyPage
{
	DECLARE_DYNAMIC(CDetectorBindWizardChooseMapPage)

public:
	CDetectorBindWizardChooseMapPage();   // standard constructor
	virtual ~CDetectorBindWizardChooseMapPage();
	core::CAlarmMachine* m_machine;
// Dialog Data
	enum { IDD = IDD_PROPPAGE_LARGE_CHOOSE_MAP };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	DECLARE_MESSAGE_MAP()
public:
	virtual BOOL OnSetActive();
	virtual BOOL OnInitDialog();
};
