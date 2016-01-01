#pragma once
#include "afxwin.h"
#include "ListBoxST.h"

#include "core.h"
// CDetectorBindWizardChooseMapPage dialog

class CDetectorBindWizardChooseMapPage : public CPropertyPage
{
	DECLARE_DYNAMIC(CDetectorBindWizardChooseMapPage)

public:
	CDetectorBindWizardChooseMapPage();   // standard constructor
	virtual ~CDetectorBindWizardChooseMapPage();
	core::CAlarmMachinePtr m_machine;
	core::CMapInfoPtr m_prevMap;
	int m_mapId;
	CImageList m_ImageList;
// Dialog Data
	enum { IDD = IDD_PROPPAGE_LARGE_CHOOSE_MAP };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	DECLARE_MESSAGE_MAP()
public:
	virtual BOOL OnSetActive();
	virtual BOOL OnInitDialog();
	gui::control::CListBoxST m_list;
	afx_msg void OnLbnSelchangeList1();
};
