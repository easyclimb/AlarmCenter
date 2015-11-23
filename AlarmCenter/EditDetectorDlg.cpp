// EditDetectorDlg.cpp : implementation file
//

#include "stdafx.h"
#include "AlarmCenter.h"
#include "EditDetectorDlg.h"
#include "afxdialogex.h"
#include "AlarmMachine.h"
#include "MapInfo.h"
#include "ZoneInfo.h"
#include "DetectorInfo.h"
#include "DetectorLib.h"
#include "BmpEx.h"
//#include "EditZoneDlg.h"
#include "DetectorBindWizrd.h"
//#include "EditMapDlg.h"
#include "UserInfo.h"

#include <vector>
//#include <algorithm>

using namespace core;


static const int NDX_ALL = 0;
static const int NDX_BIND = 1;
static const int NDX_UNBIND = 2;

static const int DEFAULT_STEP = 5;

bool MyCompareDetectorInfoFunc(const CDetectorInfo* det1, const CDetectorInfo* det2)
{
	bool bind1 = det1->get_zone_info_id() != -1;
	bool bind2 = det2->get_zone_info_id() != -1;
	if (bind1 && bind2) {
		int zone1 = det1->get_zone_value();
		int zone2 = det2->get_zone_value();
		return  bool(zone1 < zone2);
	} else if (bind1) {
		return false;
	} else if (bind2) {
		return true;
	}
	return false;
}

// CEditDetectorDlg dialog

IMPLEMENT_DYNAMIC(CEditDetectorDlg, CDialogEx)

CEditDetectorDlg::CEditDetectorDlg(CWnd* pParent /*=nullptr*/)
	: CDialogEx(CEditDetectorDlg::IDD, pParent)
	, m_prevSelMapInfo(nullptr)
	, m_prevSelZoneInfo(nullptr)
{

}

CEditDetectorDlg::~CEditDetectorDlg()
{
}

void CEditDetectorDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_LIST1, m_list);
	DDX_Control(pDX, IDC_BUTTON_BIND_MAP, m_btnBindMap);
	DDX_Control(pDX, IDC_BUTTON_UNBIND_MAP, m_btnUnbindMap);
	DDX_Control(pDX, IDC_BUTTON_BIND_ZONE, m_btnBindZone);
	DDX_Control(pDX, IDC_BUTTON_UNBIND_ZONE, m_btnUnbindZone);
	DDX_Control(pDX, IDC_COMBO_ANGLE, m_cmbAngle);
	DDX_Control(pDX, IDC_BUTTON_ROTATE_UNTICLOCK, m_btnRotateUnticlock);
	DDX_Control(pDX, IDC_BUTTON_ROTATE_CLOCK, m_btnRotateClock);
	DDX_Control(pDX, IDC_BUTTON_DISTANCE_FAR, m_btnDistanceFar);
	DDX_Control(pDX, IDC_BUTTON_DISTANCE_NEAR, m_btnDistanceNear);
	DDX_Control(pDX, IDC_BUTTON_MOVE_UP, m_btnMoveUp);
	DDX_Control(pDX, IDC_BUTTON_MOVE_LEFT, m_btnMoveLeft);
	DDX_Control(pDX, IDC_BUTTON_MOVE_DOWN, m_btnMoveDown);
	DDX_Control(pDX, IDC_BUTTON_MOVE_RIGHT, m_btnMoveRight);
	DDX_Control(pDX, IDC_COMBO_SEE, m_cmbSee);
	DDX_Control(pDX, IDC_EDIT_ZONE, m_editZone);
	DDX_Control(pDX, IDC_EDIT_MAP, m_editMap);
	DDX_Control(pDX, IDC_BUTTON_DEL_DETECTOR, m_btnDeleteDetector);
}


BEGIN_MESSAGE_MAP(CEditDetectorDlg, CDialogEx)
	ON_BN_CLICKED(IDOK, &CEditDetectorDlg::OnBnClickedOk)
	ON_CBN_SELCHANGE(IDC_COMBO_SEE, &CEditDetectorDlg::OnCbnSelchangeComboSee)
	ON_LBN_SELCHANGE(IDC_LIST1, &CEditDetectorDlg::OnLbnSelchangeListDetector)
	ON_BN_CLICKED(IDC_BUTTON_BIND_ZONE, &CEditDetectorDlg::OnBnClickedButtonBindZone)
	ON_BN_CLICKED(IDC_BUTTON_UNBIND_ZONE, &CEditDetectorDlg::OnBnClickedButtonUnbindZone)
	ON_BN_CLICKED(IDC_BUTTON_EDIT_ZONE, &CEditDetectorDlg::OnBnClickedButtonEditZone)
	ON_WM_CLOSE()
	ON_BN_CLICKED(IDC_BUTTON_BIND_MAP, &CEditDetectorDlg::OnBnClickedButtonBindMap)
	ON_BN_CLICKED(IDC_BUTTON_UNBIND_MAP, &CEditDetectorDlg::OnBnClickedButtonUnbindMap)
	ON_BN_CLICKED(IDC_BUTTON_ADD_DETECTOR, &CEditDetectorDlg::OnBnClickedButtonAddDetector)
	ON_BN_CLICKED(IDC_BUTTON_EDIT_MAP, &CEditDetectorDlg::OnBnClickedButtonEditMap)
	ON_BN_CLICKED(IDC_BUTTON_DEL_DETECTOR, &CEditDetectorDlg::OnBnClickedButtonDelDetector)
	ON_BN_CLICKED(IDC_BUTTON_MOVE_UP, &CEditDetectorDlg::OnBnClickedButtonMoveUp)
	ON_BN_CLICKED(IDC_BUTTON_MOVE_DOWN, &CEditDetectorDlg::OnBnClickedButtonMoveDown)
	ON_BN_CLICKED(IDC_BUTTON_MOVE_LEFT, &CEditDetectorDlg::OnBnClickedButtonMoveLeft)
	ON_BN_CLICKED(IDC_BUTTON_MOVE_RIGHT, &CEditDetectorDlg::OnBnClickedButtonMoveRight)
	ON_BN_CLICKED(IDC_BUTTON_DISTANCE_FAR, &CEditDetectorDlg::OnBnClickedButtonDistanceFar)
	ON_BN_CLICKED(IDC_BUTTON_DISTANCE_NEAR, &CEditDetectorDlg::OnBnClickedButtonDistanceNear)
	ON_BN_CLICKED(IDC_BUTTON_ROTATE_UNTICLOCK, &CEditDetectorDlg::OnBnClickedButtonRotateUnticlock)
	ON_BN_CLICKED(IDC_BUTTON_ROTATE_CLOCK, &CEditDetectorDlg::OnBnClickedButtonRotateClock)
END_MESSAGE_MAP()


// CEditDetectorDlg message handlers


void CEditDetectorDlg::OnBnClickedOk()
{
	return;
}


BOOL CEditDetectorDlg::OnInitDialog()
{
	AUTO_LOG_FUNCTION;
	CDialogEx::OnInitDialog();
	if (!m_machine) return FALSE;

	//CRect rc(0, 0, ::GetSystemMetrics(SM_CXSCREEN), ::GetSystemMetrics(SM_CYSCREEN));
	CRect rc;
	GetWindowRect(rc);
	CRect rcNew(rc);
	rcNew.left = 10;
	rcNew.right = 10 + rc.Width();
	MoveWindow(rcNew);

	CString txt;
	int angles[] = { 1, 5, 10, 15, 30, 45, 90, 180, 240 };
	for (int i = 0; i < sizeof(angles) / sizeof(int); i++) {
		txt.Format(L"%d", angles[i]);
		m_cmbAngle.InsertString(i, txt);
	}

	InitComboSeeAndDetList();

	CString acc, key;
	acc.LoadStringW(IDS_STRING_ACCELERATOR);

	key.LoadStringW(IDS_STRING_A);
	txt.Format(L"%s%s", acc, key);
	m_btnRotateUnticlock.SetTooltipText(txt, TRUE);

	key.LoadStringW(IDS_STRING_S);
	txt.Format(L"%s%s", acc, key);
	m_btnRotateClock.SetTooltipText(txt, TRUE);

	key.LoadStringW(IDS_STRING_ADD);
	txt.Format(L"%s%s", acc, key);
	m_btnDistanceFar.SetTooltipText(txt, TRUE);

	key.LoadStringW(IDS_STRING_SUB);
	txt.Format(L"%s%s", acc, key);
	m_btnDistanceNear.SetTooltipText(txt, TRUE);

	key.LoadStringW(IDS_STRING_UP);
	txt.Format(L"%s%s", acc, key);
	m_btnMoveUp.SetTooltipText(txt, TRUE);

	key.LoadStringW(IDS_STRING_DOWN);
	txt.Format(L"%s%s", acc, key);
	m_btnMoveDown.SetTooltipText(txt, TRUE);

	key.LoadStringW(IDS_STRING_LEFT);
	txt.Format(L"%s%s", acc, key);
	m_btnMoveLeft.SetTooltipText(txt, TRUE);

	key.LoadStringW(IDS_STRING_RIGHT);
	txt.Format(L"%s%s", acc, key);
	m_btnMoveRight.SetTooltipText(txt, TRUE);

	CUserManager* userMgr = CUserManager::GetInstance();
	const CUserInfo* user = userMgr->GetCurUserInfo();
	core::UserPriority user_priority = user->get_user_priority();
	switch (user_priority) {
		case core::UP_SUPER:
		case core::UP_ADMIN:
			m_btnDeleteDetector.EnableWindow(1);
			m_btnUnbindMap.EnableWindow(1);
			m_btnUnbindZone.EnableWindow(1);
			break;
		case core::UP_OPERATOR:
		default:
			m_btnDeleteDetector.EnableWindow(0);
			m_btnUnbindMap.EnableWindow(0);
			m_btnUnbindZone.EnableWindow(0);
			break;
	}

	return TRUE;  // return TRUE unless you set the focus to a control
	// EXCEPTION: OCX Property Pages should return FALSE
}


void CEditDetectorDlg::DisableRightUi()
{
	AUTO_LOG_FUNCTION;
	if (m_prevSelZoneInfo) {
		m_prevSelZoneInfo->InversionControl(ICZC_KILL_FOCUS);
		m_prevSelZoneInfo = nullptr;
	}
	m_editZone.SetWindowTextW(L"");
	m_btnBindZone.EnableWindow(0);
	m_btnUnbindZone.EnableWindow(0);
	m_editMap.SetWindowTextW(L"");
	m_btnBindMap.EnableWindow(0);
	m_btnUnbindMap.EnableWindow(0);
	m_btnRotateUnticlock.EnableWindow(0);
	m_btnRotateClock.EnableWindow(0);
	m_btnDistanceFar.EnableWindow(0);
	m_btnDistanceNear.EnableWindow(0);
	m_btnMoveDown.EnableWindow(0);
	m_btnMoveLeft.EnableWindow(0);
	m_btnMoveRight.EnableWindow(0);
	m_btnMoveUp.EnableWindow(0);
}


void CEditDetectorDlg::InitComboSeeAndDetList()
{
	int prevSel = 0;
	if (m_cmbSee.GetCount() > 0) {
		prevSel = m_cmbSee.GetCurSel();
	}
	m_cmbSee.ResetContent();
	m_list.ResetContent();
	m_detList.clear();
	m_bindList.clear();
	m_unbindList.clear();

	CZoneInfoList zoneList;
	m_machine->GetAllZoneInfo(zoneList);
	for (auto zoneInfo : zoneList) {
		CDetectorInfo* detInfo = zoneInfo->GetDetectorInfo();
		if (detInfo) {
			m_detList.push_back(detInfo);
			if (-1 == detInfo->get_map_id()) {
				m_unbindList.push_back(detInfo);
			} else {
				m_bindList.push_back(detInfo);
			}
		}
	}
	CString sAll, sBind, sUnbind;
	sAll.LoadStringW(IDS_STRING_ALL_DET);
	sBind.LoadStringW(IDS_STRING_BIND_DET);
	sUnbind.LoadStringW(IDS_STRING_UNBIND_DET);
	VERIFY(NDX_ALL == m_cmbSee.InsertString(NDX_ALL, sAll));
	m_cmbSee.SetItemData(NDX_ALL, NDX_ALL);
	VERIFY(NDX_BIND == m_cmbSee.InsertString(NDX_BIND, sBind));
	m_cmbSee.SetItemData(NDX_BIND, NDX_BIND);
	VERIFY(NDX_UNBIND == m_cmbSee.InsertString(NDX_UNBIND, sUnbind));
	m_cmbSee.SetItemData(NDX_UNBIND, NDX_UNBIND);

	int ndx = NDX_UNBIND + 1;
	CMapInfoList mapList;
	m_machine->GetAllMapInfo(mapList);
	for (auto mapInfo : mapList) {
		mapInfo->GetNoZoneDetectorInfo(m_detList);
		mapInfo->GetNoZoneDetectorInfo(m_unbindList);
		ndx = m_cmbSee.InsertString(ndx, mapInfo->get_alias());
		m_cmbSee.SetItemData(ndx, reinterpret_cast<DWORD_PTR>(mapInfo));
		ndx++;
	}
	m_cmbSee.SetCurSel(prevSel);
	OnCbnSelchangeComboSee();
}


void CEditDetectorDlg::FormatDetectorText(const CDetectorInfo* const detectorInfo, 
										  CString& txt)
{
	AUTO_LOG_FUNCTION;
	ASSERT(detectorInfo);
	CString snull, fmZone, fmSubmachine, fmMap;
	snull.LoadStringW(IDS_STRING_NULL);
	fmZone.LoadStringW(IDS_STRING_ZONE);
	fmSubmachine.LoadStringW(IDS_STRING_SUBMACHINE);
	fmMap.LoadStringW(IDS_STRING_MAP);
	CDetectorLib* detLib = CDetectorLib::GetInstance();
	const CDetectorLibData* data = detLib->GetDetectorLibData(detectorInfo->get_detector_lib_id());
	CZoneInfo* zoneInfo = m_machine->GetZone(detectorInfo->get_zone_value());
	CMapInfo* mapInfo = m_machine->GetMapInfo(detectorInfo->get_map_id());
	CString szone;
	if (zoneInfo) {
		if (m_machine->get_is_submachine()) {
			szone.Format(L"%s%02d(%s)", fmZone, zoneInfo->get_sub_zone(),
						 zoneInfo->get_alias());
		} else {
			if (ZT_SUB_MACHINE == zoneInfo->get_type()) {
				szone.Format(L"%s%03d(%s)", fmSubmachine, zoneInfo->get_zone_value(),
							 zoneInfo->get_alias());
			} else {
				szone.Format(L"%s%03d(%s)", fmZone, zoneInfo->get_zone_value(),
							 zoneInfo->get_alias());
			}
		}
	} else {
		szone.Format(L"%s(%s)", fmZone, snull);
	}
	CString smap;
	if (mapInfo) {
		smap.Format(L"%s(%s)", fmMap, mapInfo->get_alias());
	} else {
		smap.Format(L"%s(%s)", fmMap, snull);
	}
	txt.Format(L"%s--%s--%s", data->get_detector_name(), szone, smap);
}


void CEditDetectorDlg::LoadDetectors(std::list<CDetectorInfo*>& list)
{
	AUTO_LOG_FUNCTION;
	using namespace gui::control;
	if (list.size() == 0)
		return;

	m_ImageList.Create(THUMBNAILWIDTH, THUMBNAILWIDTH, ILC_COLOR24, 0, 1);
	m_ImageListRotate.Create(THUMBNAILWIDTH, THUMBNAILWIDTH, ILC_COLOR24, 0, 1);
	m_ImageList.SetImageCount(list.size());
	m_ImageListRotate.SetImageCount(list.size());

	int ndx = 0;
	CDetectorLib* detLib = CDetectorLib::GetInstance();
	for (auto detInfo :list) {
		const CDetectorLibData* data = detLib->GetDetectorLibData(detInfo->get_detector_lib_id());
		HBITMAP hBitmap = CBmpEx::GetHBitmapThumbnail(data->get_path(), THUMBNAILWIDTH, THUMBNAILWIDTH);
		if (hBitmap) {
			CBitmap *pImage = new CBitmap();
			pImage->Attach(hBitmap);
			m_ImageList.Replace(ndx, pImage, nullptr);
			if (data->get_type() == DT_DOUBLE) {
				HBITMAP hBitmapPair = CBmpEx::GetHBitmapThumbnail(data->get_path_pair(),
																  THUMBNAILWIDTH,
																  THUMBNAILWIDTH);
				CBitmap *pImagePair = new CBitmap();
				pImagePair->Attach(hBitmapPair);
				m_ImageListRotate.Replace(ndx, pImagePair, nullptr);
				delete pImagePair;	pImagePair = nullptr;
			} else {
				m_ImageListRotate.Replace(ndx, pImage, nullptr);
			}
			delete pImage; pImage = nullptr;
			ndx++;
		}
	}
	m_list.SetImageList(&m_ImageList, &m_ImageListRotate);
	
	ndx = 0;
	CString txt;
	for (auto detInfo : list) {
		const CDetectorLibData* data = detLib->GetDetectorLibData(detInfo->get_detector_lib_id());
		FormatDetectorText(detInfo, txt);
		m_list.InsertString(ndx, txt, ndx, (data->get_type() == DT_DOUBLE) ? ndx : -1);
		m_list.SetItemData(ndx, reinterpret_cast<DWORD>(detInfo));
		ndx++;
	}
	
}


void CEditDetectorDlg::OnCbnSelchangeComboSee()
{
	AUTO_LOG_FUNCTION;
	int ndx = m_cmbSee.GetCurSel();	if (ndx < 0) return;
	int prev_ndx = m_list.GetCurSel();
	m_list.ResetContent();
	if (m_ImageList.GetSafeHandle() != nullptr) {
		m_ImageList.DeleteImageList();
	}
	if (m_ImageListRotate.GetSafeHandle() != nullptr) {
		m_ImageListRotate.DeleteImageList();
	}

	if (NDX_ALL == ndx) {
		LoadDetectors(m_detList);
	} else if (NDX_BIND == ndx) {
		LoadDetectors(m_bindList);
	} else if (NDX_UNBIND == ndx) {
		LoadDetectors(m_unbindList);
	} else {
		DWORD data = m_cmbSee.GetItemData(ndx);
		CMapInfo* mapInfo = reinterpret_cast<CMapInfo*>(data);
		ASSERT(mapInfo);
		mapInfo->InversionControl(ICMC_SHOW);
		std::list<CDetectorBindInterface*> interfaceList;
		mapInfo->GetAllInterfaceInfo(interfaceList);
		std::list<CDetectorInfo*> detList;
		for (auto pInterface : interfaceList) {
			ASSERT(pInterface);
			if (DIT_ZONE_INFO == pInterface->GetInterfaceType()) {
				CDetectorInfo* detInfo = pInterface->GetDetectorInfo();
				ASSERT(detInfo);
				detList.push_back(detInfo);
			}
		}
		LoadDetectors(detList);
	}

	if (prev_ndx == -1)
		prev_ndx = 0;
	m_list.SetCurSel(prev_ndx);
	OnLbnSelchangeListDetector();
}


void CEditDetectorDlg::OnLbnSelchangeListDetector()
{
	AUTO_LOG_FUNCTION;
	int ndx = m_list.GetCurSel(); if (ndx < 0) {
		DisableRightUi();
		return;
	}
	CDetectorInfo* detInfo = reinterpret_cast<CDetectorInfo*>(m_list.GetItemData(ndx));
	if (nullptr == detInfo) {
		DisableRightUi();
		return;
	}
	CString snull;
	snull.LoadStringW(IDS_STRING_NULL);
	CZoneInfo* zoneInfo = m_machine->GetZone(detInfo->get_zone_value());
	CMapInfo* mapInfo = m_machine->GetMapInfo(detInfo->get_map_id());
	BOOL bBind2Zone = (nullptr != zoneInfo);
	BOOL bBind2Map = (nullptr != mapInfo);

	CString szone = snull;
	if (bBind2Zone) {
		if (m_prevSelZoneInfo) {
			m_prevSelZoneInfo->InversionControl(ICZC_KILL_FOCUS);
		}
		zoneInfo->InversionControl(ICZC_SET_FOCUS);
		m_prevSelZoneInfo = zoneInfo;
		if (m_machine->get_is_submachine()) 
			szone.Format(L"%02d", zoneInfo->get_sub_zone());
		else 
			szone.Format(L"%03d", zoneInfo->get_zone_value());
	} 
	m_editZone.SetWindowTextW(szone);
	m_btnBindZone.EnableWindow(!bBind2Zone);
	m_btnUnbindZone.EnableWindow(bBind2Zone);

	CString smap = snull;
	if (bBind2Map) {
		smap = mapInfo->get_alias();
		if (m_prevSelMapInfo) {
			m_prevSelMapInfo->InversionControl(ICMC_MODE_NORMAL);
		}
		// trick to show mapview.
		mapInfo->InversionControl(ICMC_MODE_EDIT);
		mapInfo->InversionControl(ICMC_SHOW);
		m_prevSelMapInfo = mapInfo;
	} 
	m_editMap.SetWindowTextW(smap);
	m_btnBindMap.EnableWindow(!bBind2Map);
	m_btnUnbindMap.EnableWindow(bBind2Map);

	m_btnRotateClock.EnableWindow(bBind2Zone && bBind2Map);
	m_btnRotateUnticlock.EnableWindow(bBind2Zone && bBind2Map);

	CDetectorLib* detLib = CDetectorLib::GetInstance();
	const CDetectorLibData* data = detLib->GetDetectorLibData(detInfo->get_detector_lib_id());
	BOOL bDouble = data->get_type() & DT_DOUBLE;
	m_btnDistanceFar.EnableWindow(bDouble && bBind2Zone && bBind2Map);
	m_btnDistanceNear.EnableWindow(bDouble && bBind2Zone && bBind2Map);

	m_btnMoveUp.EnableWindow(bBind2Zone && bBind2Map);
	m_btnMoveDown.EnableWindow(bBind2Zone && bBind2Map);
	m_btnMoveLeft.EnableWindow(bBind2Zone && bBind2Map);
	m_btnMoveRight.EnableWindow(bBind2Zone && bBind2Map);
}


void CEditDetectorDlg::OnBnClickedButtonBindZone()
{
	AUTO_LOG_FUNCTION;
	int ndx = m_list.GetCurSel(); if (ndx < 0) return;
	CDetectorInfo* detInfo = reinterpret_cast<CDetectorInfo*>(m_list.GetItemData(ndx));
	if (nullptr == detInfo) return;
	CZoneInfo* zoneInfo = m_machine->GetZone(detInfo->get_zone_value());
	CMapInfo* mapInfo = m_machine->GetMapInfo(detInfo->get_map_id());
	BOOL bBind2Zone = (nullptr != zoneInfo);
	BOOL bBind2Map = (nullptr != mapInfo);
	if (bBind2Zone || !bBind2Map) return;
	CString txt;
	// 1.选择一个无探头的防区
	CRect rc;
	m_btnBindZone.GetWindowRect(rc);
	zoneInfo = ChooseNoDetZoneInfo(CPoint(rc.right, rc.top));
	if (nullptr == zoneInfo)
		return;

	// 2.判断探头类型与防区类型是否一致
	CDetectorLib* detLib = CDetectorLib::GetInstance();
	const CDetectorLibData* data = detLib->GetDetectorLibData(detInfo->get_detector_lib_id());
	bool bDetectorSubMachine = (DT_SUB_MACHINE == data->get_type());
	bool bZoneInfoSubMachine = (ZT_SUB_MACHINE == zoneInfo->get_type());
	if (bDetectorSubMachine != bZoneInfoSubMachine) {
		if (bDetectorSubMachine) {
			txt.LoadStringW(IDS_STRING_Q_NOT_SM_ZONE);
		} else {
			txt.LoadStringW(IDS_STRING_Q_NOT_ZONE_ZONE);
		}
		int ret = MessageBox(txt, nullptr, MB_OKCANCEL | MB_ICONQUESTION);
		if (IDOK != ret) {
			JLOG(L"user canceled bind zone\n"); return;
		}
	}
		
	// 2.更新数据库
	if (!zoneInfo->execute_set_detector_info(detInfo)) {
		ASSERT(0); JLOG(L"update db failed.\n"); return;
	}

	// 3.更新info
	mapInfo->RemoveNoZoneDetectorInfo(detInfo);
	CMapInfo* oldMap = zoneInfo->GetMapInfo();
	if (oldMap == nullptr) {
		mapInfo->AddInterface(zoneInfo);
	} else if (oldMap != mapInfo) {
		oldMap->RemoveInterface(zoneInfo);
		mapInfo->AddInterface(zoneInfo);
	}
	zoneInfo->SetMapInfo(mapInfo);

	m_unbindList.remove(detInfo);
	m_bindList.push_back(detInfo);
	m_bindList.sort(MyCompareDetectorInfoFunc);

	// 4.显示探头
	mapInfo->SetActiveInterfaceInfo(zoneInfo);
	mapInfo->InversionControl(ICMC_NEW_DETECTOR);

	// 5.更新显示
	m_list.DeleteString(ndx);
	FormatDetectorText(detInfo, txt);
	VERIFY(ndx == m_list.InsertString(ndx, txt, ndx, (data->get_type() == DT_DOUBLE) ? ndx : -1));
	m_list.SetItemData(ndx, reinterpret_cast<DWORD>(detInfo));
	m_list.SetCurSel(ndx);
	OnLbnSelchangeListDetector();
}


CZoneInfo* CEditDetectorDlg::ChooseNoDetZoneInfo(const CPoint& pt)
{
	CString txt, sprefix, szone, fmZone, fmSubmachine;
	fmZone.LoadStringW(IDS_STRING_ZONE);
	fmSubmachine.LoadStringW(IDS_STRING_SUBMACHINE);

	CMenu menu;
	menu.CreatePopupMenu();
	std::vector<CZoneInfo*> vZoneInfo;
	vZoneInfo.push_back(nullptr); // 留空第0项

	CZoneInfoList list;
	m_machine->GetAllZoneInfo(list);
	for (auto zoneInfo : list) {
		if (nullptr == zoneInfo->GetDetectorInfo()) {
			if (nullptr != zoneInfo->GetSubMachineInfo()) {
				sprefix = fmSubmachine;
			} else {
				sprefix = fmZone;
			}
			if (m_machine->get_is_submachine()) {
				szone.Format(L"%02d", zoneInfo->get_sub_zone());
			} else {
				szone.Format(L"%03d", zoneInfo->get_zone_value());
			}
			txt.Format(L"%s%s(%s)", sprefix, szone, zoneInfo->get_alias());
			menu.AppendMenuW(MF_STRING, vZoneInfo.size(), txt);
			vZoneInfo.push_back(zoneInfo);
		}
	}
	if (vZoneInfo.size() == 1) {
		CString q; q.LoadStringW(IDS_STRING_Q_NO_MORE_ZONE_TO_BIND);
		MessageBox(q); return nullptr;
	}

	DWORD ret = menu.TrackPopupMenu(TPM_LEFTALIGN | TPM_RIGHTBUTTON | TPM_RETURNCMD,
									pt.x, pt.y, this);
	if (ret == 0 || vZoneInfo.size() < ret)
		return nullptr;
	return vZoneInfo[ret];
}


void CEditDetectorDlg::OnBnClickedButtonUnbindZone()
{
	AUTO_LOG_FUNCTION;
	int ndx = m_list.GetCurSel(); if (ndx < 0) return;
	CDetectorInfo* detInfo = reinterpret_cast<CDetectorInfo*>(m_list.GetItemData(ndx));
	if (nullptr == detInfo) return;
	CZoneInfo* zoneInfo = m_machine->GetZone(detInfo->get_zone_value());
	CMapInfo* mapInfo = m_machine->GetMapInfo(detInfo->get_map_id());
	BOOL bBind2Zone = (nullptr != zoneInfo);
	BOOL bBind2Map = (nullptr != mapInfo);
	if (!bBind2Zone) return;
	CString txt;
	CDetectorLib* detLib = CDetectorLib::GetInstance();
	const CDetectorLibData* data = detLib->GetDetectorLibData(detInfo->get_detector_lib_id());

	if (bBind2Map) {	// 有地图
		// 1.删除detector
		mapInfo->SetActiveInterfaceInfo(zoneInfo);
		mapInfo->InversionControl(ICMC_DEL_DETECTOR);

		// 2.更新数据库
		if (!zoneInfo->execute_rem_detector_info()) {
			return;
		}

		// 3.更新info
		m_bindList.remove(detInfo);
		m_unbindList.push_back(detInfo);
		m_unbindList.sort(MyCompareDetectorInfoFunc);

		// 4.更新显示
		m_list.DeleteString(ndx);
		FormatDetectorText(detInfo, txt);
		VERIFY(ndx == m_list.InsertString(ndx, txt, ndx, (data->get_type() == DT_DOUBLE) ? ndx : -1));
		m_list.SetItemData(ndx, reinterpret_cast<DWORD>(detInfo));
		m_list.SetCurSel(ndx);
		OnLbnSelchangeListDetector();
	} else {			// 无地图
		if (!zoneInfo->execute_del_detector_info()) {
			return;
		}
		m_bindList.remove(detInfo);
		InitComboSeeAndDetList();
	}
}


void CEditDetectorDlg::OnBnClickedButtonEditZone()
{
	/*CEditZoneDlg dlg;
	dlg.m_machine = m_machine;
	dlg.DoModal();*/
}


void CEditDetectorDlg::OnBnClickedButtonEditMap()
{
	//CEditMapDlg dlg;
	//dlg.m_machine = m_machine;
	//dlg.DoModal();
}


void CEditDetectorDlg::OnClose()
{
	if (m_prevSelMapInfo) {
		m_prevSelMapInfo->InversionControl(ICMC_MODE_NORMAL);
		m_prevSelMapInfo->InversionControl(ICMC_SHOW);
		m_prevSelMapInfo = nullptr;
	}
	if (m_prevSelZoneInfo) {
		m_prevSelZoneInfo->InversionControl(ICZC_KILL_FOCUS);
		m_prevSelZoneInfo = nullptr;
	}
	CDialogEx::OnClose();
}


void CEditDetectorDlg::OnBnClickedButtonBindMap()
{
	AUTO_LOG_FUNCTION;
	int ndx = m_list.GetCurSel(); if (ndx < 0) return;
	CDetectorInfo* detInfo = reinterpret_cast<CDetectorInfo*>(m_list.GetItemData(ndx));
	if (nullptr == detInfo) return;
	CZoneInfo* zoneInfo = m_machine->GetZone(detInfo->get_zone_value());
	CMapInfo* mapInfo = m_machine->GetMapInfo(detInfo->get_map_id());
	BOOL bBind2Zone = (nullptr != zoneInfo);
	BOOL bBind2Map = (nullptr != mapInfo);
	if (!bBind2Zone || bBind2Map) return;

	// 1.选择一个地图
#pragma region choose a map
	CString txt, fmNull;
	fmNull.LoadStringW(IDS_STRING_NULL);
	
	CMenu menu;
	menu.CreatePopupMenu();
	std::vector<CMapInfo*> vMapInfo;
	vMapInfo.push_back(nullptr); // 留空第0项

	CMapInfoList list;
	m_machine->GetAllMapInfo(list);
	for (auto _mapInfo : list) {
		txt = _mapInfo->get_alias();
		if (txt.IsEmpty()) {
			txt = _mapInfo->get_path();
		}
		menu.AppendMenuW(MF_STRING, vMapInfo.size(), txt);
		vMapInfo.push_back(_mapInfo);
	}
	if (vMapInfo.size() == 1) {
		CString q; q.LoadStringW(IDS_STRING_I_NO_MAP);
		MessageBox(q); return;
	}

	CRect rc;
	m_btnBindMap.GetWindowRect(rc);
	DWORD ret = menu.TrackPopupMenu(TPM_LEFTALIGN | TPM_RIGHTBUTTON | TPM_RETURNCMD,
									rc.right, rc.top, this);
	if (ret == 0 || vMapInfo.size() < ret)
		return;
	mapInfo = vMapInfo[ret];
#pragma endregion

	// 2.更新数据库
	if (!zoneInfo->execute_bind_detector_info_to_map_info(mapInfo)) {
		ASSERT(0); JLOG(L"update db failed.\n"); return;
	}

	// 3.更新info
	CMapInfo* oldMap = zoneInfo->GetMapInfo();
	if (oldMap == nullptr) {
		mapInfo->AddInterface(zoneInfo);
	} else if (oldMap != mapInfo) {
		oldMap->RemoveInterface(zoneInfo);
		mapInfo->AddInterface(zoneInfo);
	}
	zoneInfo->SetMapInfo(mapInfo);
	m_unbindList.remove(detInfo);
	m_bindList.push_back(detInfo);
	m_bindList.sort(MyCompareDetectorInfoFunc);

	// 4.显示探头
	mapInfo->SetActiveInterfaceInfo(zoneInfo);
	mapInfo->InversionControl(ICMC_NEW_DETECTOR);

	// 5.更新显示
	m_list.DeleteString(ndx);
	FormatDetectorText(detInfo, txt);
	CDetectorLib* detLib = CDetectorLib::GetInstance();
	const CDetectorLibData* data = detLib->GetDetectorLibData(detInfo->get_detector_lib_id());
	VERIFY(ndx == m_list.InsertString(ndx, txt, ndx, (data->get_type() == DT_DOUBLE) ? ndx : -1));
	m_list.SetItemData(ndx, reinterpret_cast<DWORD>(detInfo));
	m_list.SetCurSel(ndx);
	OnLbnSelchangeListDetector();
}


void CEditDetectorDlg::OnBnClickedButtonUnbindMap()
{
	AUTO_LOG_FUNCTION;
	int ndx = m_list.GetCurSel(); if (ndx < 0) return;
	CDetectorInfo* detInfo = reinterpret_cast<CDetectorInfo*>(m_list.GetItemData(ndx));
	if (nullptr == detInfo) return;
	CZoneInfo* zoneInfo = m_machine->GetZone(detInfo->get_zone_value());
	CMapInfo* mapInfo = m_machine->GetMapInfo(detInfo->get_map_id());
	BOOL bBind2Zone = (nullptr != zoneInfo);
	BOOL bBind2Map = (nullptr != mapInfo);
	if (!bBind2Map) return;
	CString txt;
	CDetectorLib* detLib = CDetectorLib::GetInstance();
	const CDetectorLibData* data = detLib->GetDetectorLibData(detInfo->get_detector_lib_id());

	if (bBind2Zone) {	// 有防区
		// 1.删除detector
		mapInfo->SetActiveInterfaceInfo(zoneInfo);
		mapInfo->InversionControl(ICMC_DEL_DETECTOR);

		// 2.更新数据库
		if (!zoneInfo->execute_unbind_detector_info_from_map_info()) {
			return;
		}

		// 3.更新缓存
		m_bindList.remove(detInfo);
		m_unbindList.push_back(detInfo);
		m_unbindList.sort(MyCompareDetectorInfoFunc);

		// 4.更新显示
		m_list.DeleteString(ndx);
		FormatDetectorText(detInfo, txt);
		VERIFY(ndx == m_list.InsertString(ndx, txt, ndx, (data->get_type() == DT_DOUBLE) ? ndx : -1));
		m_list.SetItemData(ndx, reinterpret_cast<DWORD>(detInfo));
		m_list.SetCurSel(ndx);
		OnLbnSelchangeListDetector();
	} else {			// 无防区
		if (!mapInfo->execute_delete_no_zone_detector_info(detInfo)) {
			return;
		}
		m_bindList.remove(detInfo);
		InitComboSeeAndDetList();
	}
}


void CEditDetectorDlg::OnBnClickedButtonAddDetector()
{
	CDetectorBindWizrd dlg(IDS_STRING_WIZARD_ADD_DET, this);
	dlg.m_pageChooseZone.m_machine = m_machine;
	dlg.m_pageChooseDet.m_machine = m_machine;
	dlg.m_pageChooseMap.m_machine = m_machine;
	if (ID_WIZFINISH != dlg.DoModal())
		return;

	int zoneValue = dlg.m_pageChooseZone.m_zoneValue;
	int detLibId = dlg.m_pageChooseDet.m_detLibID;
	int mapId = dlg.m_pageChooseMap.m_mapId;

	CDetectorLib* lib = CDetectorLib::GetInstance();
	CZoneInfo* zoneInfo = m_machine->GetZone(zoneValue);
	const CDetectorLibData* data = lib->GetDetectorLibData(detLibId);
	CMapInfo* mapInfo = m_machine->GetMapInfo(mapId);

	CString q;
	if (zoneInfo == nullptr) {
		q.LoadStringW(IDS_STRING_NO_CHOOSE_ZONE);
		/*int ret = */MessageBox(q, nullptr, MB_ICONINFORMATION);
		//if (ret == IDYES) 
		//	OnBnClickedButtonEditZone();
		return;
	}

	if (data == nullptr) {
		q.LoadStringW(IDS_STRING_NO_CHOOSE_DET);
		MessageBox(q, nullptr, MB_ICONERROR);
		return;
	}

	if (mapInfo == nullptr) {
		q.LoadStringW(IDS_STRING_NO_CHOOSE_MAP);
		/*int ret = */MessageBox(q, nullptr, MB_ICONINFORMATION);
		//if (ret == IDYES) 
		//	OnBnClickedButtonEditMap();
		return;
	}

	// 1.创建探头信息
	//static int cx = ::GetSystemMetrics(SM_CXSCREEN);
	//static int cy = ::GetSystemMetrics(SM_CYSCREEN);
	//static int x = cx * 2 / 3;
	//static int y = cy / 2;
	static int x = 300;
	static int y = 200;

	CDetectorInfo* detInfo = new CDetectorInfo();
	detInfo->set_x(x);
	detInfo->set_y(y);
	detInfo->set_distance(100);
	detInfo->set_angle(0);
	detInfo->set_detector_lib_id(detLibId);
	if (!zoneInfo->execute_create_detector_info_and_bind_map_info(detInfo, mapInfo)) {
		return;
	}

	m_bindList.push_back(detInfo);
	m_bindList.sort(MyCompareDetectorInfoFunc);

	// 2.显示探头
	mapInfo->SetActiveInterfaceInfo(zoneInfo);
	mapInfo->InversionControl(ICMC_NEW_DETECTOR);

	// 3.更新显示
	InitComboSeeAndDetList();
	int ndx = 0;
	for (int i = NDX_UNBIND + 1; i < m_cmbSee.GetCount(); i++) {
		CMapInfo* tmp_mapInfo = reinterpret_cast<CMapInfo*>(m_cmbSee.GetItemData(i));
		if (tmp_mapInfo && tmp_mapInfo == mapInfo) {
			ndx = i;
			break;
		}
	}
	m_cmbSee.SetCurSel(ndx);
	OnCbnSelchangeComboSee();
	
	for (ndx = 0; ndx < m_list.GetCount(); ndx++) {
		CDetectorInfo* tmp_detInfo = reinterpret_cast<CDetectorInfo*>(m_list.GetItemData(ndx));
		if (tmp_detInfo && tmp_detInfo == detInfo) {
			break;
		}
	}
	m_list.SetCurSel(ndx);
	OnLbnSelchangeListDetector();
}


void CEditDetectorDlg::OnBnClickedButtonDelDetector()
{
	AUTO_LOG_FUNCTION;
	int ndx = m_list.GetCurSel(); if (ndx < 0) return;
	CDetectorInfo* detInfo = reinterpret_cast<CDetectorInfo*>(m_list.GetItemData(ndx));
	if (nullptr == detInfo) return;
	CZoneInfo* zoneInfo = m_machine->GetZone(detInfo->get_zone_value());
	CMapInfo* mapInfo = m_machine->GetMapInfo(detInfo->get_map_id());
	BOOL bBind2Zone = (nullptr != zoneInfo);
	BOOL bBind2Map = (nullptr != mapInfo);

	if (bBind2Zone) {
		OnBnClickedButtonUnbindZone();
	} 
	
	if (bBind2Map) {
		OnBnClickedButtonUnbindMap();
	} 
}


void CEditDetectorDlg::OnBnClickedButtonMoveUp()
{
	AUTO_LOG_FUNCTION;
	MoveWithDirection(DMD_UP);
}


void CEditDetectorDlg::OnBnClickedButtonMoveDown()
{
	AUTO_LOG_FUNCTION;
	MoveWithDirection(DMD_DOWN);
}


void CEditDetectorDlg::OnBnClickedButtonMoveLeft()
{
	AUTO_LOG_FUNCTION;
	MoveWithDirection(DMD_LEFT);
}


void CEditDetectorDlg::OnBnClickedButtonMoveRight()
{
	AUTO_LOG_FUNCTION;
	MoveWithDirection(DMD_RIGHT);
}


void CEditDetectorDlg::OnBnClickedButtonDistanceFar()
{
	AUTO_LOG_FUNCTION;
	ChangeDistance();
}


void CEditDetectorDlg::OnBnClickedButtonDistanceNear()
{
	AUTO_LOG_FUNCTION;
	ChangeDistance(false);
}


void CEditDetectorDlg::OnBnClickedButtonRotateUnticlock()
{
	AUTO_LOG_FUNCTION;
	CString sstep;
	m_cmbAngle.GetWindowTextW(sstep);
	int step = _ttoi(sstep) % 360;
	sstep.Format(L"%d", step);
	m_cmbAngle.SetWindowTextW(sstep);
	if (step == 0) return;

	RotateDetector(step);
}


void CEditDetectorDlg::OnBnClickedButtonRotateClock()
{
	AUTO_LOG_FUNCTION;
	CString sstep;
	m_cmbAngle.GetWindowTextW(sstep);
	int step = _ttoi(sstep) % 360;
	sstep.Format(L"%d", step);
	m_cmbAngle.SetWindowTextW(sstep);
	if (step == 0) return; 
	RotateDetector(360 - step);
}


void CEditDetectorDlg::RotateDetector(int step)
{
	AUTO_LOG_FUNCTION;
	int ndx = m_list.GetCurSel(); if (ndx < 0) return;
	CDetectorInfo* detInfo = reinterpret_cast<CDetectorInfo*>(m_list.GetItemData(ndx));
	if (nullptr == detInfo) return;
	CZoneInfo* zoneInfo = m_machine->GetZone(detInfo->get_zone_value());
	CMapInfo* mapInfo = m_machine->GetMapInfo(detInfo->get_map_id());
	if (zoneInfo == nullptr || mapInfo == nullptr) return;

	int angle = detInfo->get_angle();
	angle = (angle + step + 360) % 360;
	detInfo->set_angle(angle);
	zoneInfo->InversionControl(ICZC_ROTATE);
	zoneInfo->execute_update_detector_info_field(CZoneInfo::DIF_ANGLE, angle);
}


void CEditDetectorDlg::ChangeDistance(bool bFar)
{
	AUTO_LOG_FUNCTION;
	int ndx = m_list.GetCurSel(); if (ndx < 0) return;
	CDetectorInfo* detInfo = reinterpret_cast<CDetectorInfo*>(m_list.GetItemData(ndx));
	if (nullptr == detInfo) return;
	CZoneInfo* zoneInfo = m_machine->GetZone(detInfo->get_zone_value());
	CMapInfo* mapInfo = m_machine->GetMapInfo(detInfo->get_map_id());
	if (zoneInfo == nullptr || mapInfo == nullptr) return;
	
	int distance = detInfo->get_distance() + (bFar ? DEFAULT_STEP : -DEFAULT_STEP);
	if (distance < DEFAULT_STEP)
		return;
	detInfo->set_distance(distance);
	zoneInfo->InversionControl(ICZC_DISTANCE);
	zoneInfo->execute_update_detector_info_field(CZoneInfo::DIF_DISTANCE, distance);
}


void CEditDetectorDlg::MoveWithDirection(DetectorMoveDirection dmd)
{
	AUTO_LOG_FUNCTION;
	int ndx = m_list.GetCurSel(); if (ndx < 0) return;
	CDetectorInfo* detInfo = reinterpret_cast<CDetectorInfo*>(m_list.GetItemData(ndx));
	if (nullptr == detInfo) return;
	CZoneInfo* zoneInfo = m_machine->GetZone(detInfo->get_zone_value());
	CMapInfo* mapInfo = m_machine->GetMapInfo(detInfo->get_map_id());
	if (zoneInfo == nullptr || mapInfo == nullptr) return;

	int x = detInfo->get_x();
	int y = detInfo->get_y();
	switch (dmd) {
		case CEditDetectorDlg::DMD_UP:
			y -= DEFAULT_STEP;
			break;
		case CEditDetectorDlg::DMD_DOWN:
			y += DEFAULT_STEP;
			break;
		case CEditDetectorDlg::DMD_LEFT:
			x -= DEFAULT_STEP;
			break;
		case CEditDetectorDlg::DMD_RIGHT:
			x += DEFAULT_STEP;
			break;
		default:
			return;
			break;
	}

	static const int MAX_OFFSET = -100;

	if (x < MAX_OFFSET)		x = MAX_OFFSET;
	if (y < MAX_OFFSET)		y = MAX_OFFSET;
	// TODO: x,y out of map range

	detInfo->set_x(x);
	detInfo->set_y(y);
	zoneInfo->InversionControl(ICZC_MOVE);
	zoneInfo->execute_update_detector_info_field(CZoneInfo::DIF_X, x);
	zoneInfo->execute_update_detector_info_field(CZoneInfo::DIF_Y, y);
}


BOOL CEditDetectorDlg::PreTranslateMessage(MSG* pMsg)
{
	if (pMsg->message == WM_KEYDOWN) {
		if (GetAsyncKeyState(VK_CONTROL) & 0x8000) { 
			BOOL ret = TRUE;
			switch(pMsg->wParam ){
				case VK_UP:
					OnBnClickedButtonMoveUp();
					break;
				case VK_DOWN:
					OnBnClickedButtonMoveDown();
					break;
				case VK_LEFT:
					OnBnClickedButtonMoveLeft();
					break;
				case VK_RIGHT:
					OnBnClickedButtonMoveRight();
					break;
				case 'A':
					OnBnClickedButtonRotateUnticlock();
					break;
				case 'S':
					OnBnClickedButtonRotateClock();
					break;
				case VK_ADD:
					OnBnClickedButtonDistanceFar();
					break;
				case VK_SUBTRACT:
					OnBnClickedButtonDistanceNear();
					break;
				default:
					ret = FALSE;
					break;
			}
			return ret;
		}
	}

	return CDialogEx::PreTranslateMessage(pMsg);
}
