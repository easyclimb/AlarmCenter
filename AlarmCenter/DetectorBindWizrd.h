#pragma once

#include "DetectorBindWizardChooseZonePage.h"
#include "DetectorBindWizardChooseDetPage.h"
// CDetectorBindWizrd

class CDetectorBindWizrd : public CPropertySheet
{
	DECLARE_DYNAMIC(CDetectorBindWizrd)

public:
	CDetectorBindWizrd(UINT nIDCaption, CWnd* pParentWnd = NULL, UINT iSelectPage = 0);
	CDetectorBindWizrd(LPCTSTR pszCaption, CWnd* pParentWnd = NULL, UINT iSelectPage = 0);
	virtual ~CDetectorBindWizrd();
private:
	CDetectorBindWizardChooseZonePage m_pageChooseZone;
	CDetectorBindWizardChooseDetPage m_pageChooseDet;
protected:
	void MyConstruct();
protected:
	DECLARE_MESSAGE_MAP()
public:
	virtual BOOL OnInitDialog();
};


