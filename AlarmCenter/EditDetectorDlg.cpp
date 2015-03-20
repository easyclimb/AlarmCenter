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

using namespace core;


static const int NDX_ALL = 0;
static const int NDX_BIND = 1;
static const int NDX_UNBIND = 2;
// CEditDetectorDlg dialog

IMPLEMENT_DYNAMIC(CEditDetectorDlg, CDialogEx)

CEditDetectorDlg::CEditDetectorDlg(CWnd* pParent /*=NULL*/)
	: CDialogEx(CEditDetectorDlg::IDD, pParent)
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
	DDX_Control(pDX, IDC_STATIC_MAP, m_staticMap);
	DDX_Control(pDX, IDC_STATIC_ZONE, m_staticZone);
}


BEGIN_MESSAGE_MAP(CEditDetectorDlg, CDialogEx)
	ON_BN_CLICKED(IDOK, &CEditDetectorDlg::OnBnClickedOk)
	ON_CBN_SELCHANGE(IDC_COMBO_SEE, &CEditDetectorDlg::OnCbnSelchangeComboSee)
	ON_LBN_SELCHANGE(IDC_LIST1, &CEditDetectorDlg::OnLbnSelchangeListDetector)
END_MESSAGE_MAP()


// CEditDetectorDlg message handlers


void CEditDetectorDlg::OnBnClickedOk()
{
	return;
}


BOOL CEditDetectorDlg::OnInitDialog()
{
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
	m_cmbSee.SetCurSel(0);
	OnCbnSelchangeComboSee();
	return TRUE;  // return TRUE unless you set the focus to a control
	// EXCEPTION: OCX Property Pages should return FALSE
}


void CEditDetectorDlg::FormatDetectorText(const CDetectorInfo* const detectorInfo, 
										  CString& txt)
{
	AUTO_LOG_FUNCTION;
	ASSERT(detectorInfo);
	CString snull;
	snull.LoadStringW(IDS_STRING_NULL);
	CDetectorLib* detLib = CDetectorLib::GetInstance();
	const CDetectorLibData* data = detLib->GetDetectorLibData(detectorInfo->get_detector_lib_id());
	CZoneInfo* zoneInfo = m_machine->GetZone(detectorInfo->get_zone_value());
	CMapInfo* mapInfo = m_machine->GetMapInfo(detectorInfo->get_map_id());
	CString szone = snull;
	if (zoneInfo) {
		if (m_machine->get_is_submachine()) {
			szone.Format(L"%02d", zoneInfo->get_zone_value());
		} else {
			szone.Format(L"%03d", zoneInfo->get_zone_value());
		}
	}
	CString smap = snull;
	if (mapInfo) {
		smap = mapInfo->get_alias();
	}
	txt.Format(L"%s(%s)--%s", szone, data->get_detector_name(), smap);
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
	m_list.SetCurSel(0);
	OnLbnSelchangeListDetector();
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
}


void CEditDetectorDlg::OnLbnSelchangeListDetector()
{
	int ndx = m_list.GetCurSel(); if (ndx < 0) return;
	CDetectorInfo* detInfo = reinterpret_cast<CDetectorInfo*>(m_list.GetItemData(ndx));
	if (NULL == detInfo) return;

	CString snull;
	snull.LoadStringW(IDS_STRING_NULL);
	CZoneInfo* zoneInfo = m_machine->GetZone(detInfo->get_zone_value());
	CMapInfo* mapInfo = m_machine->GetMapInfo(detInfo->get_map_id());
	BOOL bBind2Zone = (NULL != zoneInfo);
	BOOL bBind2Map = (NULL != mapInfo);

	CString szone = snull;
	if (bBind2Zone) {
		if (m_machine->get_is_submachine()) 
			szone.Format(L"%02d", zoneInfo->get_zone_value());
		else 
			szone.Format(L"%03d", zoneInfo->get_zone_value());
	} 
	m_staticZone.SetWindowTextW(szone);
	m_btnBindZone.EnableWindow(!bBind2Zone);
	m_btnUnbindZone.EnableWindow(bBind2Zone);

	CString smap = snull;
	if (bBind2Map) {
		smap = mapInfo->get_alias();
		// trick to show mapview.
		mapInfo->AddNewAlarmText(NULL); 
	} 
	m_staticMap.SetWindowTextW(smap);
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
