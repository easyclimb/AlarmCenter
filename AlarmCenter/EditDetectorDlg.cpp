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
#include "EditZoneDlg.h"
#include "DetectorBindWizrd.h"
#include "EditMapDlg.h"

#include <vector>
//#include <algorithm>

using namespace core;


static const int NDX_ALL = 0;
static const int NDX_BIND = 1;
static const int NDX_UNBIND = 2;

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

CEditDetectorDlg::CEditDetectorDlg(CWnd* pParent /*=NULL*/)
	: CDialogEx(CEditDetectorDlg::IDD, pParent)
	, m_prevSelMapInfo(NULL)
	, m_prevSelZoneInfo(NULL)
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

	return TRUE;  // return TRUE unless you set the focus to a control
	// EXCEPTION: OCX Property Pages should return FALSE
}


void CEditDetectorDlg::DisableRightUi()
{
	if (m_prevSelZoneInfo) {
		m_prevSelZoneInfo->InversionControl(ICZC_KILL_FOCUS);
		m_prevSelZoneInfo = NULL;
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
	CZoneInfoListIter zoneIter = zoneList.begin();
	while (zoneIter != zoneList.end()) {
		CZoneInfo* zoneInfo = *zoneIter++;
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
	CMapInfoListIter mapIter = mapList.begin();
	while (mapIter != mapList.end()) {
		CMapInfo* mapInfo = *mapIter++;
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
			szone.Format(L"%s%02d(%s)", fmZone, zoneInfo->get_zone_value(),
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
	std::list<CDetectorInfo*>::iterator iter = list.begin();
	while (iter != list.end()) {
		CDetectorInfo* detInfo = *iter++;
		const CDetectorLibData* data = detLib->GetDetectorLibData(detInfo->get_detector_lib_id());
		HBITMAP hBitmap = CBmpEx::GetHBitmapThumbnail(data->get_path(), THUMBNAILWIDTH, THUMBNAILWIDTH);
		if (hBitmap) {
			CBitmap *pImage = new CBitmap();
			pImage->Attach(hBitmap);
			m_ImageList.Replace(ndx, pImage, NULL);
			if (data->get_type() == DT_DOUBLE) {
				HBITMAP hBitmapPair = CBmpEx::GetHBitmapThumbnail(data->get_path_pair(),
																  THUMBNAILWIDTH,
																  THUMBNAILWIDTH);
				CBitmap *pImagePair = new CBitmap();
				pImagePair->Attach(hBitmapPair);
				m_ImageListRotate.Replace(ndx, pImagePair, NULL);
				delete pImagePair;	pImagePair = NULL;
			} else {
				m_ImageListRotate.Replace(ndx, pImage, NULL);
			}
			delete pImage; pImage = NULL;
			ndx++;
		}
	}
	m_list.SetImageList(&m_ImageList, &m_ImageListRotate);
	
	ndx = 0;
	iter = list.begin();
	CString txt;
	while (iter != list.end()) {
		CDetectorInfo* detInfo = *iter++;
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

	m_list.ResetContent();
	if (m_ImageList.GetSafeHandle() != NULL) {
		m_ImageList.DeleteImageList();
	}
	if (m_ImageListRotate.GetSafeHandle() != NULL) {
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
		CZoneInfoList zoneList;
		mapInfo->GetAllZoneInfo(zoneList);
		CZoneInfoListIter zoneIter = zoneList.begin();
		std::list<CDetectorInfo*> detList;
		while (zoneIter != zoneList.end()) {
			CZoneInfo* zoneInfo = *zoneIter++;
			CDetectorInfo* detInfo = zoneInfo->GetDetectorInfo();
			detList.push_back(detInfo);
		}
		LoadDetectors(detList);
	}

	m_list.SetCurSel(0);
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
	if (NULL == detInfo) {
		DisableRightUi();
		return;
	}
	CString snull;
	snull.LoadStringW(IDS_STRING_NULL);
	CZoneInfo* zoneInfo = m_machine->GetZone(detInfo->get_zone_value());
	CMapInfo* mapInfo = m_machine->GetMapInfo(detInfo->get_map_id());
	BOOL bBind2Zone = (NULL != zoneInfo);
	BOOL bBind2Map = (NULL != mapInfo);

	CString szone = snull;
	if (bBind2Zone) {
		if (m_prevSelZoneInfo) {
			m_prevSelZoneInfo->InversionControl(ICZC_KILL_FOCUS);
		}
		zoneInfo->InversionControl(ICZC_SET_FOCUS);
		m_prevSelZoneInfo = zoneInfo;
		if (m_machine->get_is_submachine()) 
			szone.Format(L"%02d", zoneInfo->get_zone_value());
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
	m_btnDistanceFar.EnableWindow(bBind2Zone && bBind2Map);
	m_btnDistanceNear.EnableWindow(bBind2Zone && bBind2Map);
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
	if (NULL == detInfo) return;
	CZoneInfo* zoneInfo = m_machine->GetZone(detInfo->get_zone_value());
	CMapInfo* mapInfo = m_machine->GetMapInfo(detInfo->get_map_id());
	BOOL bBind2Zone = (NULL != zoneInfo);
	BOOL bBind2Map = (NULL != mapInfo);
	if (bBind2Zone || !bBind2Map) return;
	CString txt;
	// 1.选择一个无探头的防区
	CRect rc;
	m_btnBindZone.GetWindowRect(rc);
	zoneInfo = ChooseNoDetZoneInfo(CPoint(rc.right, rc.top));
	if (NULL == zoneInfo)
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
		int ret = MessageBox(txt, NULL, MB_OKCANCEL | MB_ICONQUESTION);
		if (IDOK != ret) {
			LOG(L"user canceled bind zone\n"); return;
		}
	}
		
	// 2.更新数据库
	if (!zoneInfo->execute_set_detector_info(detInfo)) {
		ASSERT(0); LOG(L"update db failed.\n"); return;
	}

	// 3.更新info
	mapInfo->RemoveNoZoneDetectorInfo(detInfo);
	CMapInfo* oldMap = zoneInfo->GetMapInfo();
	if (oldMap == NULL) {
		mapInfo->AddZone(zoneInfo);
	} else if (oldMap != mapInfo) {
		oldMap->RemoveZone(zoneInfo);
		mapInfo->AddZone(zoneInfo);
	}
	zoneInfo->SetMapInfo(mapInfo);

	m_unbindList.remove(detInfo);
	m_bindList.push_back(detInfo);
	m_bindList.sort(MyCompareDetectorInfoFunc);

	// 4.显示探头
	mapInfo->SetActiveZoneInfo(zoneInfo);
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
	vZoneInfo.push_back(NULL); // 留空第0项

	CZoneInfoList list;
	m_machine->GetAllZoneInfo(list);
	CZoneInfoListIter iter = list.begin();
	while (iter != list.end()) {
		CZoneInfo* zoneInfo = *iter++;
		if (NULL == zoneInfo->GetDetectorInfo()) {
			if (NULL != zoneInfo->GetSubMachineInfo()) {
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
		MessageBox(q); return NULL;
	}

	DWORD ret = menu.TrackPopupMenu(TPM_LEFTALIGN | TPM_RIGHTBUTTON | TPM_RETURNCMD,
									pt.x, pt.y, this);
	if (ret == 0 || vZoneInfo.size() < ret)
		return NULL;
	return vZoneInfo[ret];
}


void CEditDetectorDlg::OnBnClickedButtonUnbindZone()
{
	AUTO_LOG_FUNCTION;
	int ndx = m_list.GetCurSel(); if (ndx < 0) return;
	CDetectorInfo* detInfo = reinterpret_cast<CDetectorInfo*>(m_list.GetItemData(ndx));
	if (NULL == detInfo) return;
	CZoneInfo* zoneInfo = m_machine->GetZone(detInfo->get_zone_value());
	CMapInfo* mapInfo = m_machine->GetMapInfo(detInfo->get_map_id());
	BOOL bBind2Zone = (NULL != zoneInfo);
	BOOL bBind2Map = (NULL != mapInfo);
	if (!bBind2Zone) return;
	CString txt;
	CDetectorLib* detLib = CDetectorLib::GetInstance();
	const CDetectorLibData* data = detLib->GetDetectorLibData(detInfo->get_detector_lib_id());

	if (bBind2Map) {	// 有地图
		// 1.删除detector
		mapInfo->SetActiveZoneInfo(zoneInfo);
		mapInfo->InversionControl(ICMC_DEL_DETECTOR);

		// 2.更新数据库
		if (!zoneInfo->execute_rem_detector_info()) {
			return;
		}

		// 3.更新info
		mapInfo->AddNoZoneDetectorInfo(detInfo);
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
	CEditZoneDlg dlg;
	dlg.m_machine = m_machine;
	dlg.DoModal();
}


void CEditDetectorDlg::OnBnClickedButtonEditMap()
{
	CEditMapDlg dlg;
	dlg.m_machine = m_machine;
	dlg.DoModal();
}


void CEditDetectorDlg::OnClose()
{
	if (m_prevSelMapInfo) {
		m_prevSelMapInfo->InversionControl(ICMC_MODE_NORMAL);
		m_prevSelMapInfo->InversionControl(ICMC_SHOW);
		m_prevSelMapInfo = NULL;
	}
	if (m_prevSelZoneInfo) {
		m_prevSelZoneInfo->InversionControl(ICZC_KILL_FOCUS);
		m_prevSelZoneInfo = NULL;
	}
	CDialogEx::OnClose();
}


void CEditDetectorDlg::OnBnClickedButtonBindMap()
{
	AUTO_LOG_FUNCTION;
	int ndx = m_list.GetCurSel(); if (ndx < 0) return;
	CDetectorInfo* detInfo = reinterpret_cast<CDetectorInfo*>(m_list.GetItemData(ndx));
	if (NULL == detInfo) return;
	CZoneInfo* zoneInfo = m_machine->GetZone(detInfo->get_zone_value());
	CMapInfo* mapInfo = m_machine->GetMapInfo(detInfo->get_map_id());
	BOOL bBind2Zone = (NULL != zoneInfo);
	BOOL bBind2Map = (NULL != mapInfo);
	if (!bBind2Zone || bBind2Map) return;

	// 1.选择一个地图
#pragma region choose a map
	CString txt, fmNull;
	fmNull.LoadStringW(IDS_STRING_NULL);
	
	CMenu menu;
	menu.CreatePopupMenu();
	std::vector<CMapInfo*> vMapInfo;
	vMapInfo.push_back(NULL); // 留空第0项

	CMapInfoList list;
	m_machine->GetAllMapInfo(list);
	CMapInfoListIter iter = list.begin();
	while (iter != list.end()) {
		CMapInfo* mapInfo = *iter++;
		txt = mapInfo->get_alias();
		if (txt.IsEmpty()) {
			txt = mapInfo->get_path();
		}
		menu.AppendMenuW(MF_STRING, vMapInfo.size(), txt);
		vMapInfo.push_back(mapInfo);
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
		ASSERT(0); LOG(L"update db failed.\n"); return;
	}

	// 3.更新info
	CMapInfo* oldMap = zoneInfo->GetMapInfo();
	if (oldMap == NULL) {
		mapInfo->AddZone(zoneInfo);
	} else if (oldMap != mapInfo) {
		oldMap->RemoveZone(zoneInfo);
		mapInfo->AddZone(zoneInfo);
	}
	zoneInfo->SetMapInfo(mapInfo);
	m_unbindList.remove(detInfo);
	m_bindList.push_back(detInfo);
	m_bindList.sort(MyCompareDetectorInfoFunc);

	// 4.显示探头
	mapInfo->SetActiveZoneInfo(zoneInfo);
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
	if (NULL == detInfo) return;
	CZoneInfo* zoneInfo = m_machine->GetZone(detInfo->get_zone_value());
	CMapInfo* mapInfo = m_machine->GetMapInfo(detInfo->get_map_id());
	BOOL bBind2Zone = (NULL != zoneInfo);
	BOOL bBind2Map = (NULL != mapInfo);
	if (!bBind2Map) return;
	CString txt;
	CDetectorLib* detLib = CDetectorLib::GetInstance();
	const CDetectorLibData* data = detLib->GetDetectorLibData(detInfo->get_detector_lib_id());

	if (bBind2Zone) {	// 有防区
		// 1.删除detector
		mapInfo->SetActiveZoneInfo(zoneInfo);
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
	if (IDOK != dlg.DoModal())
		return;

	int zoneValue = dlg.m_pageChooseZone.m_zoneValue;
	int detLibId = dlg.m_pageChooseDet.m_detLibID;
	int mapId = dlg.m_pageChooseMap.m_mapId;

	CDetectorLib* lib = CDetectorLib::GetInstance();
	CZoneInfo* zoneInfo = m_machine->GetZone(zoneValue);
	const CDetectorLibData* data = lib->GetDetectorLibData(detLibId);
	CMapInfo* mapInfo = m_machine->GetMapInfo(mapId);

	CString q;
	if (zoneInfo == NULL) {
		q.LoadStringW(IDS_STRING_NO_CHOOSE_ZONE);
		int ret = MessageBox(q, NULL, MB_YESNOCANCEL | MB_ICONQUESTION);
		if (ret == IDYES) 
			OnBnClickedButtonEditZone();
		return;
	}

	if (data == NULL) {
		q.LoadStringW(IDS_STRING_NO_CHOOSE_DET);
		MessageBox(q, NULL, MB_ICONERROR);
		return;
	}

	if (mapInfo == NULL) {
		q.LoadStringW(IDS_STRING_NO_CHOOSE_MAP);
		int ret = MessageBox(q, NULL, MB_YESNOCANCEL | MB_ICONQUESTION);
		if (ret == IDYES) 
			OnBnClickedButtonEditMap();
		return;
	}

	// 1.创建探头信息
	CDetectorInfo* detInfo = new CDetectorInfo();

	// 2.更新数据库
	if (!zoneInfo->execute_set_detector_info(detInfo)) {
		ASSERT(0); LOG(L"update db failed.\n"); return;
	}

	// 3.更新info
	mapInfo->RemoveNoZoneDetectorInfo(detInfo);
	CMapInfo* oldMap = zoneInfo->GetMapInfo();
	if (oldMap == NULL) {
		mapInfo->AddZone(zoneInfo);
	} else if (oldMap != mapInfo) {
		oldMap->RemoveZone(zoneInfo);
		mapInfo->AddZone(zoneInfo);
	}
	zoneInfo->SetMapInfo(mapInfo);

	m_unbindList.remove(detInfo);
	m_bindList.push_back(detInfo);
	m_bindList.sort(MyCompareDetectorInfoFunc);

	// 4.显示探头
	mapInfo->SetActiveZoneInfo(zoneInfo);
	mapInfo->InversionControl(ICMC_NEW_DETECTOR);

	// 5.更新显示
	m_list.DeleteString(ndx);
	FormatDetectorText(detInfo, txt);
	VERIFY(ndx == m_list.InsertString(ndx, txt, ndx, (data->get_type() == DT_DOUBLE) ? ndx : -1));
	m_list.SetItemData(ndx, reinterpret_cast<DWORD>(detInfo));
	m_list.SetCurSel(ndx);
	OnLbnSelchangeListDetector();
}



