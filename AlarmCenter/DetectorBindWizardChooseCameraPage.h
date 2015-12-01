#pragma once
#include "ListBoxST.h"

namespace core { class CAlarmMachine; };
namespace video { namespace ezviz { class CVideoDeviceInfoEzviz; }; };
// CDetectorBindWizardChooseCameraPage dialog

class CDetectorBindWizardChooseCameraPage : public CPropertyPage
{
	DECLARE_DYNAMIC(CDetectorBindWizardChooseCameraPage)

public:
	CDetectorBindWizardChooseCameraPage();
	virtual ~CDetectorBindWizardChooseCameraPage();
	core::CAlarmMachine* m_machine = nullptr;
	//std::pair<int, int> m_pair = std::make_pair<int, int>(0, 0);
	video::ezviz::CVideoDeviceInfoEzviz* m_curSelDev = nullptr;
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
	afx_msg void OnLbnSelchangeList1();
};
