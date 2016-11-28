// AlarmHandleStep1Dlg.cpp : implementation file
//

#include "stdafx.h"
#include "AlarmCenter.h"
#include "AlarmHandleStep1Dlg.h"
#include "AlarmHandleStep2Dlg.h"
#include "AlarmHandleStep3Dlg.h"
#include "AlarmHandleStep4Dlg.h"
#include "afxdialogex.h"
#include "AlarmMachine.h"
#include "AlarmMachineManager.h"
#include "MapInfo.h"
#include "ZoneInfo.h"
#include "alarm_handle_mgr.h"

using namespace core;

namespace detail {
bool open_file_and_upload_to_data_attach(HWND hwnd, std::wstring& dest)
{
	std::wstring from;
	if (!jlib::get_file_open_dialog_result(from, hwnd)) {
		return false;
	}

	dest = get_data_path() + L"\\attach";
	CreateDirectory(dest.c_str(), nullptr);

	SHFILEOPSTRUCT sfos = {};
	sfos.hwnd = hwnd;
	CString pFrom = from.c_str();
	pFrom.Insert(pFrom.GetLength() + 1, L'\0');
	sfos.pFrom = pFrom;
	CString pTo = dest.c_str();
	pTo.Insert(pTo.GetLength() + 1, L'\0');
	sfos.pTo = dest.c_str();
	sfos.wFunc = FO_COPY;
	sfos.fFlags = FOF_SIMPLEPROGRESS;
	auto ret = SHFileOperation(&sfos);
	if (!sfos.fAnyOperationsAborted && ret == 0) {
		auto name = CFileOper::GetFileNameFromPathName(from.c_str());
		dest += L"\\";
		dest += name;
		return true;
	}
	return false;
}
}


class CAlarmHandleStep1Dlg::alarm_text_observer : public dp::observer<core::icmc_buffer_ptr>
{
public:
	virtual void on_update(const core::icmc_buffer_ptr& ptr) override {
		if (dlg) {
			dlg->add_alarm_text(ptr->_at);
		}
	}

	CAlarmHandleStep1Dlg* dlg = nullptr;
};

// CAlarmHandleStep1Dlg dialog

IMPLEMENT_DYNAMIC(CAlarmHandleStep1Dlg, CDialogEx)

CAlarmHandleStep1Dlg::CAlarmHandleStep1Dlg(CWnd* pParent /*=NULL*/)
	: CDialogEx(IDD_DIALOG_ALARM_HANDLE_1, pParent)
{

}

CAlarmHandleStep1Dlg::~CAlarmHandleStep1Dlg()
{
}


core::alarm_type CAlarmHandleStep1Dlg::get_alarm_type()
{
	if (m_radio_true_alarm.GetCheck()) {
		return alarm_type_true;
	} else if (m_radio_device_false_positive.GetCheck()) {
		return alarm_type_device_false_positive;
	} else if (m_radio_test_device.GetCheck()) {
		return alarm_type_test_device;
	} else if (m_radio_man_made_false_posotive.GetCheck()) {
		return alarm_type_man_made_false_positive;
	}

	return alarm_type_cannot_determine;
}

void CAlarmHandleStep1Dlg::add_alarm_text(const core::alarm_text_ptr & at)
{
	if (!machine_->get_is_submachine()) {
		if (at->_subzone == 0) {
			alarm_texts_.push_back(at);
		}
	} else {
		alarm_texts_.push_back(at);
	}
	
}

void CAlarmHandleStep1Dlg::prepair()
{
	//cur_handling_alarm_text_ = nullptr;
	alarm_texts_.clear();

	auto no_zone_map = machine_->GetUnbindZoneMap();
	if (no_zone_map) {
		no_zone_map->TraverseAlarmText(alarm_text_observer_);
	}

	core::map_info_list map_list;
	machine_->GetAllMapInfo(map_list);
	for (auto map : map_list) {
		map->TraverseAlarmText(alarm_text_observer_);
	}

	CString txt;
	txt.Format(L"%06d", machine_->get_ademco_id());
	m_aid.SetWindowTextW(txt);
	m_name.SetWindowTextW(machine_->get_machine_name());
	m_contact.SetWindowTextW(machine_->get_contact());
	m_addr.SetWindowTextW(machine_->get_address());
	m_phone.SetWindowTextW(machine_->get_phone());
	m_phone_bk.SetWindowTextW(machine_->get_phone_bk());
}

void CAlarmHandleStep1Dlg::show_one()
{
	if (!alarm_texts_.empty()) {
		//auto iter = alarm_texts_.begin();
		//cur_handling_alarm_text_ = *iter;
		//alarm_texts_.erase(iter);



		//m_alarm_text.SetWindowTextW(cur_handling_alarm_text_->_txt);

		auto mgr = alarm_handle_mgr::get_instance();
		cur_handling_alarm_info_ = mgr->execute_add_alarm(machine_->get_ademco_id(),
														  machine_->get_is_submachine() ? machine_->get_submachine_zone() : 0,
														  machine_->get_is_submachine(),
														  /*(LPCTSTR)(cur_handling_alarm_text_->_txt),
														  std::chrono::system_clock::from_time_t(cur_handling_alarm_text_->_time),*/
														  0, 0, 0);

		cur_handling_alarm_info_ = mgr->execute_add_alarm_texts(cur_handling_alarm_info_->get_id(), alarm_texts_);

		machine_->set_alarm_id(cur_handling_alarm_info_->get_id());

		m_alarm_text.SetWindowTextW(cur_handling_alarm_info_->get_text().c_str());

	} else {
		OnOK();
	}
}

void CAlarmHandleStep1Dlg::handle_one()
{
	bool handled = false;
	bool need_security_guard = false;

	auto alarm_type = get_alarm_type();
	alarm_judgement_ptr judgement = nullptr;
	alarm_handle_ptr handle = nullptr;

	auto mgr = core::alarm_handle_mgr::get_instance();

	// resolve alarm reason
	auto reason = mgr->execute_add_alarm_reason(alarm_type, L"", L"");
	cur_handling_alarm_info_ = mgr->execute_update_alarm_reason(cur_handling_alarm_info_->get_id(), reason);
	cur_handling_alarm_info_ = mgr->execute_update_alarm_status(cur_handling_alarm_info_->get_id(), alarm_status::alarm_status_not_judged);

	switch (alarm_type) {
	case core::alarm_type_true:
	case core::alarm_type_device_false_positive:
	case core::alarm_type_man_made_false_positive:
	case core::alarm_type_test_device:
	{
		// resolve alarm judgment
		CAlarmHandleStep2Dlg dlg;
		if (IDOK != dlg.DoModal()) {
			break;
		}

		int judgement_id = dlg.prev_sel_alarm_judgement_;
		if (judgement_id == alarm_judgement_by_user_define) {
			judgement_id = dlg.prev_user_defined_;
		}

		judgement = mgr->execute_add_judgment(judgement_id, dlg.note_, dlg.video_, dlg.image_);
		cur_handling_alarm_info_ = mgr->execute_update_alarm_judgment(cur_handling_alarm_info_->get_id(), judgement);

		if (alarm_type == alarm_type_true) {
			cur_handling_alarm_info_ = mgr->execute_update_alarm_status(cur_handling_alarm_info_->get_id(), alarm_status::alarm_status_not_handled);
			need_security_guard = true;
		} else {
			cur_handling_alarm_info_ = mgr->execute_update_alarm_status(cur_handling_alarm_info_->get_id(), alarm_status::alarm_status_cleared);
			handled = true;
		}
	}
		break;
	
	case core::alarm_type_cannot_determine:
		need_security_guard = true;
		break;

	default:
		break;
	}

	if (need_security_guard) {
		// reolve alarm handle

		CAlarmHandleStep3Dlg dlg;
		if (IDOK == dlg.DoModal()) {
			handle = create_alarm_handle(dlg.cur_editting_guard_id_,
										 wstring_to_time_point(cur_handling_alarm_info_->get_assign_time()),
										 std::chrono::minutes(dlg.cur_editting_handle_time_),
										 dlg.cur_editting_note_);

			cur_handling_alarm_info_ = mgr->execute_update_alarm_handle(cur_handling_alarm_info_->get_id(), handle);
			cur_handling_alarm_info_ = mgr->execute_update_alarm_status(cur_handling_alarm_info_->get_id(), alarm_status::alarm_status_not_cleared);
			handled = true;
		} else {
			handled = false;
		}
	}

	if (handled) {
		//show_result();

		CAlarmHandleStep4Dlg dlg;
		dlg.cur_handling_alarm_info_ = cur_handling_alarm_info_;
		dlg.judgment_ = judgement;
		dlg.handle_ = handle;
		dlg.reason_ = reason;
		dlg.machine_ = machine_;
		if (IDOK != dlg.DoModal()) {
			handled = false;
		}

		if (handled && !alarm_texts_.empty()) {
			//show_one();
		}
	}
}


void CAlarmHandleStep1Dlg::show_result()
{
	
}

void CAlarmHandleStep1Dlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_EDIT_AID, m_aid);
	DDX_Control(pDX, IDC_EDIT_NAME, m_name);
	DDX_Control(pDX, IDC_EDIT_CONTACT, m_contact);
	DDX_Control(pDX, IDC_EDIT_ADDR, m_addr);
	DDX_Control(pDX, IDC_EDIT_PHONE, m_phone);
	DDX_Control(pDX, IDC_EDIT_PHONE_BK, m_phone_bk);
	DDX_Control(pDX, IDC_EDIT_ALARM_INFO, m_alarm_text);
	DDX_Control(pDX, IDC_RADIO1, m_radio_true_alarm);
	DDX_Control(pDX, IDC_RADIO2, m_radio_device_false_positive);
	DDX_Control(pDX, IDC_RADIO3, m_radio_test_device);
	DDX_Control(pDX, IDC_RADIO4, m_radio_man_made_false_posotive);
	DDX_Control(pDX, IDC_RADIO5, m_radio_cannot_determine);
	DDX_Control(pDX, IDC_BUTTON_PRINT, m_btn_print);
}


BEGIN_MESSAGE_MAP(CAlarmHandleStep1Dlg, CDialogEx)
	ON_BN_CLICKED(IDOK, &CAlarmHandleStep1Dlg::OnBnClickedOk)
	ON_BN_CLICKED(IDC_BUTTON_PRINT, &CAlarmHandleStep1Dlg::OnBnClickedButtonPrint)
END_MESSAGE_MAP()


// CAlarmHandleStep1Dlg message handlers


BOOL CAlarmHandleStep1Dlg::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	SetWindowText(TR(IDS_STRING_ALARM_HANDLE_STEP_1));

	SET_WINDOW_TEXT(IDC_STATIC_AID, IDS_STRING_IDC_STATIC_008);
	SET_WINDOW_TEXT(IDC_STATIC_NAME, IDS_STRING_IDC_STATIC_011);
	SET_WINDOW_TEXT(IDC_STATIC_CONTACT, IDS_STRING_IDC_STATIC_012);
	SET_WINDOW_TEXT(IDC_STATIC_ADDR, IDS_STRING_IDC_STATIC_013);
	SET_WINDOW_TEXT(IDC_STATIC_PHONE, IDS_STRING_IDC_STATIC_014);
	SET_WINDOW_TEXT(IDC_STATIC_PHONE_BK, IDS_STRING_IDC_STATIC_015);
	SET_WINDOW_TEXT(IDC_STATIC_GROUP_ALARM_INFO, IDS_STRING_HRLV_ALARM);

	SET_WINDOW_TEXT(IDC_RADIO1, IDS_STRING_ALARM_TYPE_TRUE);
	SET_WINDOW_TEXT(IDC_RADIO2, IDS_STRING_ALARM_TYPE_DEVICE_FALSE_POSITIVE);
	SET_WINDOW_TEXT(IDC_RADIO3, IDS_STRING_ALARM_TYPE_TEST_DEVICE);
	SET_WINDOW_TEXT(IDC_RADIO4, IDS_STRING_ALARM_TYPE_MAN_MADE_FALSE_POSITIVE);
	SET_WINDOW_TEXT(IDC_RADIO5, IDS_STRING_ALARM_TYPE_UNABLE_TO_DETERMINE);

	SET_WINDOW_TEXT(IDC_BUTTON_PRINT, IDS_STRING_PRINT);
	SET_WINDOW_TEXT(IDOK, IDS_OK);
	SET_WINDOW_TEXT(IDCANCEL, IDS_CANCEL);

	m_alarm_text.set_text_color(RGB(255, 0, 0));


	m_radio_cannot_determine.SetCheck(1);

	alarm_text_observer_ = std::make_shared<alarm_text_observer>();
	alarm_text_observer_->dlg = this;

	if (machine_) {
		prepair();
		show_one();
	}



	return TRUE;  // return TRUE unless you set the focus to a control
				  // EXCEPTION: OCX Property Pages should return FALSE
}


void CAlarmHandleStep1Dlg::OnBnClickedOk()
{
	handle_one();

	

	CDialogEx::OnOK();
}


void CAlarmHandleStep1Dlg::OnBnClickedButtonPrint()
{
	//auto window = m_hWnd;

	//// get screen rectangle
	//RECT windowRect;
	//GetWindowRect(&windowRect);

	//// bitmap dimensions
	//int bitmap_dx = windowRect.right - windowRect.left;
	//int bitmap_dy = windowRect.bottom - windowRect.top;

	//if (false) {
	//	CRect rc;
	//	m_btn_print.GetWindowRect(rc);
	//	bitmap_dy -= rc.Height() + 25;
	//}

	//// create file
	//std::wstringstream ss;
	//ss << get_exe_path() << L"\\tmp.bmp";
	//auto filename = ss.str();
	//std::ofstream file(filename, std::ios::binary);
	//if (!file) return;

	//// save bitmap file headers
	//BITMAPFILEHEADER fileHeader;
	//BITMAPINFOHEADER infoHeader;

	//fileHeader.bfType = 0x4d42;
	//fileHeader.bfSize = 0;
	//fileHeader.bfReserved1 = 0;
	//fileHeader.bfReserved2 = 0;
	//fileHeader.bfOffBits = sizeof(BITMAPFILEHEADER) + sizeof(BITMAPINFOHEADER);

	//infoHeader.biSize = sizeof(infoHeader);
	//infoHeader.biWidth = bitmap_dx;
	//infoHeader.biHeight = bitmap_dy;
	//infoHeader.biPlanes = 1;
	//infoHeader.biBitCount = 24;
	//infoHeader.biCompression = BI_RGB;
	//infoHeader.biSizeImage = 0;
	//infoHeader.biXPelsPerMeter = 0;
	//infoHeader.biYPelsPerMeter = 0;
	//infoHeader.biClrUsed = 0;
	//infoHeader.biClrImportant = 0;

	//file.write((char*)&fileHeader, sizeof(fileHeader));
	//file.write((char*)&infoHeader, sizeof(infoHeader));

	//// dibsection information
	//BITMAPINFO info;
	//info.bmiHeader = infoHeader;

	//// ------------------
	//// THE IMPORTANT CODE
	//// ------------------
	//// create a dibsection and blit the window contents to the bitmap
	//HDC winDC = ::GetWindowDC(window);
	//HDC memDC = CreateCompatibleDC(winDC);
	//BYTE* memory = 0;
	//HBITMAP bitmap = CreateDIBSection(winDC, &info, DIB_RGB_COLORS, (void**)&memory, 0, 0);
	//SelectObject(memDC, bitmap);
	//BitBlt(memDC, 0, 0, bitmap_dx, bitmap_dy, winDC, 0, 0, SRCCOPY);
	//DeleteDC(memDC);
	//::ReleaseDC(window, winDC);

	//// save dibsection data
	//int bytes = (((24 * bitmap_dx + 31) & (~31)) / 8)*bitmap_dy;
	//file.write((const char*)memory, bytes);

	//// HA HA, forgot paste in the DeleteObject lol, happy now ;)?
	//DeleteObject(bitmap);

	
	/*ss.str(L""); ss.clear();
	ss << L"print " << filename;
	std::system(utf8::w2a(ss.str()).c_str());*/



	CPrintDialog dlg(FALSE);
	//dlg.GetDefaults();
	int ret = dlg.DoModal();
	if (ret != IDOK) {
		return;
	}

	//DEVMODE devmode = { 0 };
	//devmode.dmOrientation = DMORIENT_PORTRAIT;

	//PRINTDLG   pd;
	//pd.lStructSize = sizeof(PRINTDLG);
	//pd.Flags = PD_RETURNDC;
	//pd.hDC = nullptr;
	//pd.hwndOwner = nullptr;
	//pd.hInstance = nullptr;
	//pd.nMaxPage = 2;
	//pd.nMinPage = 1;
	//pd.nFromPage = 1;
	//pd.nToPage = 1;
	//pd.nCopies = 1;
	//pd.hDevMode = nullptr;
	//pd.hDevNames = nullptr;

	///////////////////////////////////////////////////////////
	////显示打印对话框，由用户来设定纸张大小等.
	//if (!PrintDlg(&pd))   return;
	//ASSERT(pd.hDC != nullptr);/*断言获取的句柄不为空.*/



	//auto pDevMode = dlg.GetDevMode();
	//GlobalLock(pDevMode);
	//pDevMode->dmOrientation = DMORIENT_PORTRAIT;
	//GlobalUnlock(pDevMode);


	auto winDC = GetDC();
	CRect rc;
	GetClientRect(rc);

	if (0) {
		CRect rc_btn;
		m_btn_print.GetWindowRect(rc_btn);
		rc.bottom += rc_btn.Height() + 25;
	}

	// is a default printer set up?
	HDC hdcPrinter = dlg.GetPrinterDC();
	//HDC hdcPrinter = pd.hDC;
	if (hdcPrinter == NULL) {
		MessageBox(_T("Buy a printer!"));
	} else {
		// create a CDC and attach it to the default printer
		CDC dcPrinter;
		dcPrinter.Attach(hdcPrinter);

		// call StartDoc() to begin printing
		DOCINFO docinfo;
		memset(&docinfo, 0, sizeof(docinfo));
		docinfo.cbSize = sizeof(docinfo);
		docinfo.lpszDocName = _T("CDC::StartDoc() Code Fragment");

		// if it fails, complain and exit gracefully
		if (dcPrinter.StartDoc(&docinfo) < 0) {
			MessageBox(_T("Printer wouldn't initalize"));
		} else {
			// start a page
			if (dcPrinter.StartPage() < 0) {
				//MessageBox(_T("Could not start page"));
				dcPrinter.AbortDoc();
			} else {
				int   nHorRes = dcPrinter.GetDeviceCaps(HORZRES);
				int   nVerRes = dcPrinter.GetDeviceCaps(VERTRES);
				int   nXMargin = 20;//页边的空白   
				int   nYMargin = 5;

				dcPrinter.StretchBlt(nXMargin, nYMargin, nHorRes - 2 * nXMargin, nVerRes - 2 * nYMargin, winDC, 
									 0, 0, rc.Width(), rc.Height(), SRCCOPY);

				dcPrinter.EndPage();
				dcPrinter.EndDoc();
			}
		}
	}

	//winDC->DeleteDC();
}
