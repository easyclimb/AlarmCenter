#pragma once
#include "afxwin.h"
#include "ListBoxST.h"

#include "core.h"
// CDetectorBindWizardChooseDetPage dialog

class CDetectorBindWizardChooseDetPage : public CPropertyPage
{
	DECLARE_DYNAMIC(CDetectorBindWizardChooseDetPage)

public:
	CDetectorBindWizardChooseDetPage();
	virtual ~CDetectorBindWizardChooseDetPage();
	int m_detLibID;
	core::alarm_machine_ptr m_machine;
// Dialog Data
	enum { IDD = IDD_PROPPAGE_LARGE_CHOOSE_DET };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	DECLARE_MESSAGE_MAP()
public:
	virtual BOOL OnSetActive();
	virtual BOOL OnInitDialog();
	gui::control::CListBoxST m_list;
	CImageList m_ImageList;
	CImageList m_ImageListRotate;
	afx_msg void OnLbnSelchangeList1();
};
