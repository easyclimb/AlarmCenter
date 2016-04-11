// EditDetectorDlg.cpp : implementation file
//

#include "stdafx.h"
#include "AlarmCenter.h"
#include "EditCameraDlg.h"
#include "afxdialogex.h"
#include "AlarmMachine.h"
#include "MapInfo.h"
#include "CameraInfo.h"
#include "DetectorInfo.h"
#include "DetectorLib.h"
#include "BmpEx.h"
//#include "EditZoneDlg.h"
#include "CameraBindWizard.h"
//#include "EditMapDlg.h"
#include "UserInfo.h"
#include "AlarmMachineManager.h"
#include "VideoDeviceInfoEzviz.h"
#include "VideoUserInfo.h"

#include <vector>
//#include <algorithm>

using namespace core;

namespace detail {
	const int NDX_ALL = 0;
	const int DEFAULT_STEP = 5;
};

// CEditCameraDlg dialog

IMPLEMENT_DYNAMIC(CEditCameraDlg, CDialogEx)

CEditCameraDlg::CEditCameraDlg(CWnd* pParent /*=nullptr*/)
	: CDialogEx(CEditCameraDlg::IDD, pParent)
	, m_prevSelMapInfo(nullptr)
	, m_prevSelCameraInfo(nullptr)
{

}

CEditCameraDlg::~CEditCameraDlg()
{
}

void CEditCameraDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_LIST1, m_list);
	DDX_Control(pDX, IDC_COMBO_ANGLE, m_cmbAngle);
	DDX_Control(pDX, IDC_BUTTON_ROTATE_UNTICLOCK, m_btnRotateUnticlock);
	DDX_Control(pDX, IDC_BUTTON_ROTATE_CLOCK, m_btnRotateClock);
	DDX_Control(pDX, IDC_BUTTON_MOVE_UP, m_btnMoveUp);
	DDX_Control(pDX, IDC_BUTTON_MOVE_LEFT, m_btnMoveLeft);
	DDX_Control(pDX, IDC_BUTTON_MOVE_DOWN, m_btnMoveDown);
	DDX_Control(pDX, IDC_BUTTON_MOVE_RIGHT, m_btnMoveRight);
	DDX_Control(pDX, IDC_COMBO_SEE, m_cmbSee);
	DDX_Control(pDX, IDC_BUTTON_DEL_CEMERA, m_btnDeleteCamera);
}


BEGIN_MESSAGE_MAP(CEditCameraDlg, CDialogEx)
	ON_BN_CLICKED(IDOK, &CEditCameraDlg::OnBnClickedOk)
	ON_CBN_SELCHANGE(IDC_COMBO_SEE, &CEditCameraDlg::OnCbnSelchangeComboSee)
	ON_LBN_SELCHANGE(IDC_LIST1, &CEditCameraDlg::OnLbnSelchangeListCamera)
	ON_WM_CLOSE()
	ON_BN_CLICKED(IDC_BUTTON_ADD_CEMERA, &CEditCameraDlg::OnBnClickedButtonAddCamera)
	ON_BN_CLICKED(IDC_BUTTON_DEL_CEMERA, &CEditCameraDlg::OnBnClickedButtonDelCamera)
	ON_BN_CLICKED(IDC_BUTTON_MOVE_UP, &CEditCameraDlg::OnBnClickedButtonMoveUp)
	ON_BN_CLICKED(IDC_BUTTON_MOVE_DOWN, &CEditCameraDlg::OnBnClickedButtonMoveDown)
	ON_BN_CLICKED(IDC_BUTTON_MOVE_LEFT, &CEditCameraDlg::OnBnClickedButtonMoveLeft)
	ON_BN_CLICKED(IDC_BUTTON_MOVE_RIGHT, &CEditCameraDlg::OnBnClickedButtonMoveRight)
	ON_BN_CLICKED(IDC_BUTTON_ROTATE_UNTICLOCK, &CEditCameraDlg::OnBnClickedButtonRotateUnticlock)
	ON_BN_CLICKED(IDC_BUTTON_ROTATE_CLOCK, &CEditCameraDlg::OnBnClickedButtonRotateClock)
END_MESSAGE_MAP()


// CEditCameraDlg message handlers


void CEditCameraDlg::OnBnClickedOk()
{
	return;
}


BOOL CEditCameraDlg::OnInitDialog()
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
	acc = GetStringFromAppResource(IDS_STRING_ACCELERATOR);

	key = GetStringFromAppResource(IDS_STRING_A);
	txt.Format(L"%s%s", acc, key);
	m_btnRotateUnticlock.SetTooltipText(txt, TRUE);

	key = GetStringFromAppResource(IDS_STRING_S);
	txt.Format(L"%s%s", acc, key);
	m_btnRotateClock.SetTooltipText(txt, TRUE);

	key = GetStringFromAppResource(IDS_STRING_UP);
	txt.Format(L"%s%s", acc, key);
	m_btnMoveUp.SetTooltipText(txt, TRUE);

	key = GetStringFromAppResource(IDS_STRING_DOWN);
	txt.Format(L"%s%s", acc, key);
	m_btnMoveDown.SetTooltipText(txt, TRUE);

	key = GetStringFromAppResource(IDS_STRING_LEFT);
	txt.Format(L"%s%s", acc, key);
	m_btnMoveLeft.SetTooltipText(txt, TRUE);

	key = GetStringFromAppResource(IDS_STRING_RIGHT);
	txt.Format(L"%s%s", acc, key);
	m_btnMoveRight.SetTooltipText(txt, TRUE);

	CUserManager* userMgr = CUserManager::GetInstance();
	CUserInfoPtr user = userMgr->GetCurUserInfo();
	core::UserPriority user_priority = user->get_user_priority();
	switch (user_priority) {
		case core::UP_SUPER:
		case core::UP_ADMIN:
			m_btnDeleteCamera.EnableWindow(1);
			break;
		case core::UP_OPERATOR:
		default:
			m_btnDeleteCamera.EnableWindow(0);
			break;
	}

	return TRUE;  // return TRUE unless you set the focus to a control
	// EXCEPTION: OCX Property Pages should return FALSE
}


void CEditCameraDlg::DisableRightUi()
{
	AUTO_LOG_FUNCTION;
	if (m_prevSelCameraInfo) {
		m_prevSelCameraInfo->InversionControl(ICZC_KILL_FOCUS);
		m_prevSelCameraInfo = nullptr;
	}
	m_btnRotateUnticlock.EnableWindow(0);
	m_btnRotateClock.EnableWindow(0);
	m_btnMoveDown.EnableWindow(0);
	m_btnMoveLeft.EnableWindow(0);
	m_btnMoveRight.EnableWindow(0);
	m_btnMoveUp.EnableWindow(0);
}


void CEditCameraDlg::InitComboSeeAndDetList()
{
	int prevSel = 0;
	if (m_cmbSee.GetCount() > 0) {
		prevSel = m_cmbSee.GetCurSel();
	}
	m_cmbSee.ResetContent();
	m_list.ResetContent();
	m_cameraList.clear();

	CString sAll;
	sAll = GetStringFromAppResource(IDS_STRING_ALL_DET);
	VERIFY(detail::NDX_ALL == m_cmbSee.InsertString(detail::NDX_ALL, sAll));
	m_cmbSee.SetItemData(detail::NDX_ALL, detail::NDX_ALL);

	int ndx = detail::NDX_ALL + 1;
	CMapInfoList mapList;
	m_machine->GetAllMapInfo(mapList);
	for (auto mapInfo : mapList) {
		CDetectorBindInterfaceList list;
		mapInfo->GetAllInterfaceInfo(list);
		for (auto pInterface : list) {
			if (DIT_CAMERA_INFO == pInterface->GetInterfaceType()) {
				m_cameraList.push_back(std::dynamic_pointer_cast<CCameraInfo>(pInterface));
			}
		}
		ndx = m_cmbSee.InsertString(ndx, mapInfo->get_alias());
		m_cmbSee.SetItemData(ndx, mapInfo->get_id());
		ndx++;
	}
	m_cmbSee.SetCurSel(prevSel);
	OnCbnSelchangeComboSee();
}


void CEditCameraDlg::FormatText(const core::CCameraInfoPtr& camera, CString& txt)
{
	std::wstring tip = camera->FormatTooltip();
	std::wstring str;
	for (auto c : tip) {
		if (c != L'\r' && c != L'\n')
			str.push_back(c);
		else
			str.push_back(L'-');
	}

	txt = str.c_str();
}


void CEditCameraDlg::LoadCameras(std::list<CCameraInfoPtr>& cameraList)
{
	AUTO_LOG_FUNCTION;
	using namespace gui::control;
	if (cameraList.size() == 0)
		return;

	m_ImageList.Create(THUMBNAILWIDTH, THUMBNAILWIDTH, ILC_COLOR24, 0, 1);
	m_ImageListRotate.Create(THUMBNAILWIDTH, THUMBNAILWIDTH, ILC_COLOR24, 0, 1);
	m_ImageList.SetImageCount(cameraList.size());
	m_ImageListRotate.SetImageCount(cameraList.size());

	int ndx = 0;
	CDetectorLib* detLib = CDetectorLib::GetInstance();
	for (auto camera : cameraList) {
		const CDetectorLibDataPtr data = detLib->GetDetectorLibData(camera->GetDetectorInfo()->get_detector_lib_id());
		HBITMAP hBitmap = CBmpEx::GetHBitmapThumbnail(data->get_path(), THUMBNAILWIDTH, THUMBNAILWIDTH);
		if (hBitmap) {
			auto pImage = std::make_unique<CBitmap>();
			pImage->Attach(hBitmap);
			m_ImageList.Replace(ndx, pImage.get(), nullptr);
			if (data->get_type() == DT_DOUBLE) {
				HBITMAP hBitmapPair = CBmpEx::GetHBitmapThumbnail(data->get_path_pair(),
																  THUMBNAILWIDTH,
																  THUMBNAILWIDTH);
				auto pImagePair = std::make_unique<CBitmap>();
				pImagePair->Attach(hBitmapPair);
				m_ImageListRotate.Replace(ndx, pImagePair.get(), nullptr);
			} else {
				m_ImageListRotate.Replace(ndx, pImage.get(), nullptr);
			}
			ndx++;
		}
	}
	m_list.SetImageList(&m_ImageList, &m_ImageListRotate);
	
	ndx = 0;
	CString txt;
	for (auto camera : cameraList) {
		const CDetectorLibDataPtr data = detLib->GetDetectorLibData(camera->GetDetectorInfo()->get_detector_lib_id());
		FormatText(camera, txt);
		m_list.InsertString(ndx, txt, ndx, (data->get_type() == DT_DOUBLE) ? ndx : -1);
		m_list.SetItemData(ndx, camera->GetDetectorInfo()->get_id());
		ndx++;
	}
	
}


void CEditCameraDlg::OnCbnSelchangeComboSee()
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

	if (detail::NDX_ALL == ndx) {
		LoadCameras(m_cameraList);
	} else {
		auto mgr = core::CAlarmMachineManager::GetInstance();
		DWORD data = m_cmbSee.GetItemData(ndx);
		CMapInfoPtr mapInfo = mgr->GetMapInfoById(data);
		mapInfo->InversionControl(ICMC_SHOW);
		std::list<CDetectorBindInterfacePtr> interfaceList;
		mapInfo->GetAllInterfaceInfo(interfaceList);
		std::list<CCameraInfoPtr> cameraList;
		for (auto pInterface : interfaceList) {
			ASSERT(pInterface);
			if (DIT_CAMERA_INFO == pInterface->GetInterfaceType()) {
				cameraList.push_back(std::dynamic_pointer_cast<CCameraInfo>(pInterface));
			}
		}
		LoadCameras(cameraList);
	}

	if (prev_ndx == -1)
		prev_ndx = 0;
	m_list.SetCurSel(prev_ndx);
	OnLbnSelchangeListCamera();
}


void CEditCameraDlg::OnLbnSelchangeListCamera()
{
	AUTO_LOG_FUNCTION;
	int ndx = m_list.GetCurSel(); if (ndx < 0) {
		DisableRightUi();
		return;
	}
	auto mgr = CAlarmMachineManager::GetInstance();
	CCameraInfoPtr cameraInfo = mgr->GetCameraInfo(m_list.GetItemData(ndx));
	if (nullptr == cameraInfo) {
		DisableRightUi();
		return;
	}

	CMapInfoPtr mapInfo = m_machine->GetMapInfo(cameraInfo->GetDetectorInfo()->get_map_id());
	if (m_prevSelMapInfo && m_prevSelMapInfo != mapInfo) {
		m_prevSelMapInfo->InversionControl(ICMC_MODE_NORMAL);
	}
	// trick to show mapview.
	mapInfo->InversionControl(ICMC_MODE_EDIT);
	mapInfo->InversionControl(ICMC_SHOW);
	m_prevSelMapInfo = mapInfo;

	if (m_prevSelCameraInfo) {
		m_prevSelCameraInfo->InversionControl(ICZC_KILL_FOCUS);
	}
	cameraInfo->InversionControl(ICZC_SET_FOCUS);
	m_prevSelCameraInfo = cameraInfo;

	

	m_btnRotateClock.EnableWindow();
	m_btnRotateUnticlock.EnableWindow();
	m_btnMoveUp.EnableWindow();
	m_btnMoveDown.EnableWindow();
	m_btnMoveLeft.EnableWindow();
	m_btnMoveRight.EnableWindow();
}


void CEditCameraDlg::OnClose()
{
	if (m_prevSelMapInfo) {
		m_prevSelMapInfo->InversionControl(ICMC_MODE_NORMAL);
		m_prevSelMapInfo->InversionControl(ICMC_SHOW);
		m_prevSelMapInfo = nullptr;
	}
	if (m_prevSelCameraInfo) {
		m_prevSelCameraInfo->InversionControl(ICZC_KILL_FOCUS);
		m_prevSelCameraInfo = nullptr;
	}
	CDialogEx::OnClose();
}


void CEditCameraDlg::OnBnClickedButtonAddCamera()
{
	AUTO_LOG_FUNCTION;
	CCameraBindWizard dlg(IDS_STRING_WIZARD_ADD_CAMERA, this);
	dlg.m_pageChooseMap.m_machine = m_machine;
	dlg.m_pageChooseCamera.m_machine = m_machine;
	if (ID_WIZFINISH != dlg.DoModal())
		return;

	video::ezviz::CVideoDeviceInfoEzvizPtr devInfo = dlg.m_pageChooseCamera.m_curSelDev; assert(devInfo);
	
	CDetectorLib* lib = CDetectorLib::GetInstance();
	const CDetectorLibDataPtr data = lib->GetDetectorLibData(DI_CAMERA);
	CMapInfoPtr mapInfo = m_machine->GetMapInfo(dlg.m_pageChooseMap.m_mapId);

	CString q;
	if (data == nullptr) {
		q = GetStringFromAppResource(IDS_STRING_NO_CHOOSE_DET);
		MessageBox(q, nullptr, MB_ICONERROR);
		return;
	}
	if (devInfo == nullptr) {
		q = GetStringFromAppResource(IDS_STRING_NO_CHOOSE_CAMERA);
		MessageBox(q, nullptr, MB_ICONERROR);
		return;
	}
	if (mapInfo == nullptr) {
		q = GetStringFromAppResource(IDS_STRING_NO_CHOOSE_MAP);
		/*int ret = */MessageBox(q, nullptr, MB_ICONERROR);
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

	CDetectorInfoPtr detInfo = std::make_shared<CDetectorInfo>();
	detInfo->set_map_id(mapInfo->get_id());
	detInfo->set_x(x);
	detInfo->set_y(y);
	detInfo->set_distance(0);
	detInfo->set_angle(0);
	detInfo->set_detector_lib_id(DI_CAMERA);
	CString query;
	query.Format(L"insert into DetectorInfoOfCamera \
([ademco_id],[sub_machine_id],[map_id],[x],[y],[distance],[angle],[detector_lib_id],\
[device_info_id],[device_productor])\
values(%d,%d,%d,%d,%d,%d,%d,%d,%d,%d)",
		m_machine->get_ademco_id(), 
		m_machine->get_is_submachine() ? m_machine->get_submachine_zone() : -1,
		mapInfo->get_id(), 
		detInfo->get_x(),
		detInfo->get_y(),
		detInfo->get_distance(), 
		detInfo->get_angle(),
		detInfo->get_detector_lib_id(), 
		devInfo->get_id(), 
		devInfo->get_userInfo()->get_productorInfo().get_productor());

	CAlarmMachineManager* mgr = CAlarmMachineManager::GetInstance();
	int id = mgr->AddAutoIndexTableReturnID(query);
	if (-1 == id) {
		ASSERT(0); JLOG(L"insert DetectorInfoOfCamera info failed.\n"); return;
	}
	detInfo->set_id(id);
	
	CCameraInfoPtr cameraInfo = std::make_shared<CCameraInfo>();
	cameraInfo->set_ademco_id(m_machine->get_ademco_id());
	cameraInfo->set_sub_machine_id(m_machine->get_is_submachine() ? m_machine->get_submachine_zone() : -1);
	cameraInfo->set_device_info_id(devInfo->get_id());
	cameraInfo->set_productor(devInfo->get_userInfo()->get_productorInfo().get_productor());
	cameraInfo->SetDetectorInfo(detInfo);

	mgr->AddCameraInfo(cameraInfo);

	m_cameraList.push_back(cameraInfo);

	// 2.显示探头
	mapInfo->AddInterface(cameraInfo);
	mapInfo->SetActiveInterfaceInfo(cameraInfo);
	mapInfo->InversionControl(ICMC_NEW_DETECTOR);

	// 3.更新显示
	InitComboSeeAndDetList();
	int ndx = 0;
	for (int i = detail::NDX_ALL + 1; i < m_cmbSee.GetCount(); i++) {
		DWORD itemData = m_cmbSee.GetItemData(ndx);
		CMapInfoPtr tmp_mapInfo = mgr->GetMapInfoById(itemData);
		if (tmp_mapInfo && tmp_mapInfo == mapInfo) {
			ndx = i;
			break;
		}
	}
	m_cmbSee.SetCurSel(ndx);
	OnCbnSelchangeComboSee();
	for (ndx = 0; ndx < m_list.GetCount(); ndx++) {
		CCameraInfoPtr tmp_camInfo = mgr->GetCameraInfo(m_list.GetItemData(ndx));
		if (tmp_camInfo && tmp_camInfo == cameraInfo) {
			break;
		}
	}
	m_list.SetCurSel(ndx);
	OnLbnSelchangeListCamera();
}


void CEditCameraDlg::OnBnClickedButtonDelCamera()
{
	AUTO_LOG_FUNCTION;
	int ndx = m_list.GetCurSel(); if (ndx < 0) return;
	auto mgr = CAlarmMachineManager::GetInstance();
	CCameraInfoPtr cameraInfo = mgr->GetCameraInfo(m_list.GetItemData(ndx));
	if (nullptr == cameraInfo) return;
	CDetectorInfoPtr detInfo = cameraInfo->GetDetectorInfo();
	if (nullptr == detInfo) return;
	CMapInfoPtr mapInfo = m_machine->GetMapInfo(detInfo->get_map_id());

	mapInfo->SetActiveInterfaceInfo(cameraInfo);
	mapInfo->InversionControl(ICMC_DEL_DETECTOR);
	mapInfo->RemoveInterface(cameraInfo);

	m_cameraList.remove(cameraInfo);
	CAlarmMachineManager::GetInstance()->DeleteCameraInfo(cameraInfo);

	m_prevSelCameraInfo = nullptr;
	m_list.DeleteString(ndx);
	m_list.SetCurSel(ndx-1);
	OnLbnSelchangeListCamera();
}


void CEditCameraDlg::OnBnClickedButtonMoveUp()
{
	AUTO_LOG_FUNCTION;
	MoveWithDirection(CMD_UP);
}


void CEditCameraDlg::OnBnClickedButtonMoveDown()
{
	AUTO_LOG_FUNCTION;
	MoveWithDirection(CMD_DOWN);
}


void CEditCameraDlg::OnBnClickedButtonMoveLeft()
{
	AUTO_LOG_FUNCTION;
	MoveWithDirection(CMD_LEFT);
}


void CEditCameraDlg::OnBnClickedButtonMoveRight()
{
	AUTO_LOG_FUNCTION;
	MoveWithDirection(CMD_RIGHT);
}


void CEditCameraDlg::OnBnClickedButtonRotateUnticlock()
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


void CEditCameraDlg::OnBnClickedButtonRotateClock()
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


void CEditCameraDlg::RotateDetector(int step)
{
	AUTO_LOG_FUNCTION;
	int ndx = m_list.GetCurSel(); if (ndx < 0) return;
	auto mgr = CAlarmMachineManager::GetInstance();
	CCameraInfoPtr cameraInfo = mgr->GetCameraInfo(m_list.GetItemData(ndx));
	if (nullptr == cameraInfo) return;
	CDetectorInfoPtr detInfo = cameraInfo->GetDetectorInfo();
	if (nullptr == detInfo) return;
	CMapInfoPtr mapInfo = m_machine->GetMapInfo(detInfo->get_map_id());
	if (mapInfo == nullptr) return;

	int angle = detInfo->get_angle();
	angle = (angle + step + 360) % 360;
	detInfo->set_angle(angle);
	cameraInfo->InversionControl(ICZC_ROTATE);
	cameraInfo->execute_update_detector_info_field(CDetectorBindInterface::DIF_ANGLE, angle);
}


void CEditCameraDlg::MoveWithDirection(CameraMoveDirection cmd)
{
	AUTO_LOG_FUNCTION;
	int ndx = m_list.GetCurSel(); if (ndx < 0) return;
	auto mgr = CAlarmMachineManager::GetInstance();
	CCameraInfoPtr cameraInfo = mgr->GetCameraInfo(m_list.GetItemData(ndx));
	if (nullptr == cameraInfo) return;
	CDetectorInfoPtr detInfo = cameraInfo->GetDetectorInfo();
	if (nullptr == detInfo) return;
	CMapInfoPtr mapInfo = m_machine->GetMapInfo(detInfo->get_map_id());
	if (mapInfo == nullptr) return;

	int x = detInfo->get_x();
	int y = detInfo->get_y();
	switch (cmd) {
		case CEditCameraDlg::CMD_UP:
			y -= detail::DEFAULT_STEP;
			break;
		case CEditCameraDlg::CMD_DOWN:
			y += detail::DEFAULT_STEP;
			break;
		case CEditCameraDlg::CMD_LEFT:
			x -= detail::DEFAULT_STEP;
			break;
		case CEditCameraDlg::CMD_RIGHT:
			x += detail::DEFAULT_STEP;
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
	cameraInfo->InversionControl(ICZC_MOVE);
	cameraInfo->execute_update_detector_info_field(CDetectorBindInterface::DIF_X, x);
	cameraInfo->execute_update_detector_info_field(CDetectorBindInterface::DIF_Y, y);
}


BOOL CEditCameraDlg::PreTranslateMessage(MSG* pMsg)
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
