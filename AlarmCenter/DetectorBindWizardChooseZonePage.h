#pragma once
#include "afxwin.h"

namespace core { class CAlarmMachine; };
// CDetectorBindWizardChooseZonePage dialog

class CDetectorBindWizardChooseZonePage : public CPropertyPage
{
	DECLARE_DYNAMIC(CDetectorBindWizardChooseZonePage)

public:
	CDetectorBindWizardChooseZonePage();
	virtual ~CDetectorBindWizardChooseZonePage();

// Dialog Data
	enum { IDD = IDD_PROPPAGE_LARGE_CHOOSE_ZONE };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	DECLARE_MESSAGE_MAP()
public:
	virtual BOOL OnSetActive();
	CListBox m_list;
	int m_zoneValue;
	core::CAlarmMachine* m_machine;
	CFont m_font;
	afx_msg void OnLbnSelchangeList1();
	virtual BOOL OnInitDialog();
};
