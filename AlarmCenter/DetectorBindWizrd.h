#pragma once

#include "DetectorBindWizardChooseZonePage.h"
#include "DetectorBindWizardChooseDetPage.h"
#include "DetectorBindWizardChooseMapPage.h"
// CDetectorBindWizrd



class CDetectorBindWizrd : public CPropertySheet
{
	DECLARE_DYNAMIC(CDetectorBindWizrd)

public:
	CDetectorBindWizrd(UINT nIDCaption, CWnd* pParentWnd = nullptr, UINT iSelectPage = 0);
	CDetectorBindWizrd(LPCTSTR pszCaption, CWnd* pParentWnd = nullptr, UINT iSelectPage = 0);
	virtual ~CDetectorBindWizrd();
public:
	CDetectorBindWizardChooseZonePage m_pageChooseZone;
	CDetectorBindWizardChooseDetPage m_pageChooseDet;
	CDetectorBindWizardChooseMapPage m_pageChooseMap;
protected:
	void MyConstruct();
protected:
	DECLARE_MESSAGE_MAP()
public:
	virtual BOOL OnInitDialog();
};


