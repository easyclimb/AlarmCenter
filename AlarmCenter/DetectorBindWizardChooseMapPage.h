#pragma once
#include "afxwin.h"
#include "ListBoxST.h"

namespace core { class CAlarmMachine; class CMapInfo; };
// CDetectorBindWizardChooseMapPage dialog

class CDetectorBindWizardChooseMapPage : public CPropertyPage
{
	DECLARE_DYNAMIC(CDetectorBindWizardChooseMapPage)

public:
	CDetectorBindWizardChooseMapPage();   // standard constructor
	virtual ~CDetectorBindWizardChooseMapPage();
	core::CAlarmMachine* m_machine;
	core::CMapInfo* m_prevMap;
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
