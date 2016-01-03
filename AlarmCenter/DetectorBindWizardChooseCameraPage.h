#pragma once
#include "ListBoxST.h"

#include "core.h"
#include "video.h"
// CDetectorBindWizardChooseCameraPage dialog

class CDetectorBindWizardChooseCameraPage : public CPropertyPage
{
	DECLARE_DYNAMIC(CDetectorBindWizardChooseCameraPage)

public:
	CDetectorBindWizardChooseCameraPage();
	virtual ~CDetectorBindWizardChooseCameraPage();
	core::CAlarmMachinePtr m_machine = nullptr;
	//std::pair<int, int> m_pair = std::make_pair<int, int>(0, 0);
	video::ezviz::CVideoDeviceInfoEzvizPtr m_curSelDev = nullptr;
// Dialog Data
	enum { IDD = IDD_PROPPAGE_LARGE_CHOOSE_CAMERA };

protected:
	virtual BOOL OnSetActive();
	virtual BOOL OnInitDialog();
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	gui::control::CListBoxST m_list;
	CImageList m_ImageList;
	CImageList m_ImageListRotate;
	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnLbnSelchangeList1();
};
