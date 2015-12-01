#pragma once

#include "DetectorBindWizardChooseCameraPage.h"
#include "DetectorBindWizardChooseMapPage.h"
// CCameraBindWizard

class CCameraBindWizard : public CPropertySheet
{
	DECLARE_DYNAMIC(CCameraBindWizard)

public:
	CCameraBindWizard(UINT nIDCaption, CWnd* pParentWnd = nullptr, UINT iSelectPage = 0);
	CCameraBindWizard(LPCTSTR pszCaption, CWnd* pParentWnd = nullptr, UINT iSelectPage = 0);
	virtual ~CCameraBindWizard();
	CDetectorBindWizardChooseCameraPage m_pageChooseCamera;
	CDetectorBindWizardChooseMapPage m_pageChooseMap;
protected:
	void MyConstruct();
	
protected:
	DECLARE_MESSAGE_MAP()
public:
	virtual BOOL OnInitDialog();
};


