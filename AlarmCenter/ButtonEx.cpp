#include "stdafx.h"
#include "resource.h"
#include "ButtonEx.h"
#include "MFCButtonEx.h"
#include "BtnST.h"
#include "AlarmMachine.h"
#include "./imagin/Timer.h"
#include "AlarmMachineContainer.h"
#include "AlarmMachineManager.h"
#include "AppResource.h"
#include "ZoneInfo.h"
#include "InputDlg.h"
#include "BaiduMapViewerDlg.h"
#include "HistoryRecord.h"
#include "StaticBmp.h"
#include "StaticColorText.h"

using namespace ademco;


namespace gui {

namespace detail {
	const int cTimerIdFlush = 1;
	const int cTimerIdAdemco = 2;

	const COLORREF cColorRed = RGB(255, 0, 0);
	const COLORREF cColorBlack = RGB(0, 0, 0);
	const COLORREF cColorWhite = RGB(255, 255, 255);

	//IMPLEMENT_ADEMCO_EVENT_CALL_BACK(CButtonEx, OnAdemcoEvent);

	void __stdcall on_imagin_timer(imagin::CTimer* /*timer*/, void* udata)
	{
		CButtonEx* btn = reinterpret_cast<CButtonEx*>(udata); ASSERT(btn);
		btn->OnImaginTimer();
	}

	void __stdcall on_timer(/*imagin::CTimer* timer, */void* udata, UINT nTimerId)
	{
		CButtonEx* btn = reinterpret_cast<CButtonEx*>(udata); ASSERT(btn);
		btn->OnTimer(nTimerId);
	}

	void __stdcall on_btnclick(ButtonClick bc, void* udata) {
		CButtonEx* btn = reinterpret_cast<CButtonEx*>(udata); ASSERT(btn);
		if (btn && btn->IsValidButton()) {
			if (bc == BC_LEFT) {
				btn->OnBnClicked();
			} else if (bc == BC_RIGHT) {
				btn->OnRBnClicked();
			}
		}
	}
};

using namespace detail;

CButtonEx::CButtonEx(const wchar_t* /*text*/,
					 const RECT& rc,
					 CWnd* parent,
					 UINT id,
					 core::CAlarmMachinePtr machine)
	: _button(nullptr)
	, _wndParent(parent)
	, _bSwitchColor(FALSE)
	, _timer(nullptr)
	, _machine(machine)
	, _bAlarming(FALSE)
	, _clrText(cColorWhite)
	, _clrFace(cColorBlack)
{
	AUTO_LOG_FUNCTION;
	assert(machine);
	m_observer = std::make_shared<ObserverType>(this);
	machine->register_observer(m_observer);
	//machine->register_observer(this, OnAdemcoEvent);
	_button = std::make_shared<CMFCButtonEx>();
	_button->Create(nullptr, WS_CHILD | WS_VISIBLE | WS_EX_TRANSPARENT, rc, parent, id);
	ASSERT(IsWindow(_button->m_hWnd));
	_button->SetFocus();

	static const int cIconWidth = 30;
	static const int cTextHeight = 18;
	static const int cBoundGap = 6;
	static const int cIconGap = 2;
	
	CRect rcButton, rcText, rcIcon;
	_button->GetClientRect(rcButton);
	rcText = rcIcon = rcButton;

	const int cMid = rcButton.top + cBoundGap + cTextHeight + cIconGap;
	rcText.top = rcButton.top + cBoundGap;
	rcText.bottom = rcText.top + cTextHeight + cIconGap;
	rcText.left += cBoundGap;
	rcText.right = rcButton.right - cBoundGap;
	color_text_ = std::shared_ptr<CColorText>(new CColorText(),
											  [](CColorText* p) {SAFEDELETEDLG(p); });
	color_text_->Create(nullptr, WS_CHILD | WS_VISIBLE | WS_EX_TRANSPARENT | SS_LEFT, rcText, _button.get());

	
	rcIcon.top = cMid + cIconGap;
	rcIcon.bottom = rcButton.bottom - cBoundGap;
	rcIcon.left = rcButton.left + cBoundGap;
	rcIcon.right = rcIcon.left + cIconWidth;
	iconOnOffLine_ = std::shared_ptr<CIconEx>(new CIconEx(), [](CIconEx* p) { SAFEDELETEDLG(p); });
	iconOnOffLine_->Create(nullptr, WS_CHILD | WS_VISIBLE | WS_EX_TRANSPARENT, rcIcon, _button.get());

	rcIcon.left = rcIcon.right + cIconGap;
	rcIcon.right = rcIcon.left + cIconWidth;
	iconStatus_ = std::shared_ptr<CIconEx>(new CIconEx(), [](CIconEx* p) { SAFEDELETEDLG(p); });
	iconStatus_->Create(nullptr, WS_CHILD | WS_VISIBLE | WS_EX_TRANSPARENT, rcIcon, _button.get());

	rcIcon.left = rcIcon.right + cIconGap;
	rcIcon.right = rcIcon.left + cIconWidth;
	iconExtra_ = std::shared_ptr<CIconEx>(new CIconEx(), [](CIconEx* p) { SAFEDELETEDLG(p); });
	iconExtra_->Create(nullptr, WS_CHILD | WS_VISIBLE | WS_EX_TRANSPARENT, rcIcon, _button.get());

	_button->SetFaceColor(cColorWhite);
	color_text_->SetFaceColor(cColorWhite);
	UpdateButtonText();
	UpdateIconAndColor(_machine->get_online(), _machine->get_machine_status());
	
#pragma region set tooltip
	CString tooltip = L"", fmAlias, fmContact, fmAddress, fmPhone, fmPhoneBk, fmNull;
	CString sid, alias, contact, address, phone, phone_bk;
	fmAlias = GetStringFromAppResource(IDS_STRING_ALIAS);
	fmContact = GetStringFromAppResource(IDS_STRING_CONTACT);
	fmAddress = GetStringFromAppResource(IDS_STRING_ADDRESS);
	fmPhone = GetStringFromAppResource(IDS_STRING_PHONE);
	fmPhoneBk = GetStringFromAppResource(IDS_STRING_PHONE_BK);
	fmNull = GetStringFromAppResource(IDS_STRING_NULL);
	if (_machine->get_is_submachine())
		sid.Format(L"ID:%03d", _machine->get_submachine_zone());
	else
		sid.Format(L"ID:%04d", _machine->get_ademco_id());
	alias = _machine->get_alias();
	contact = _machine->get_contact();
	address = _machine->get_address();
	phone = _machine->get_phone();
	phone_bk = _machine->get_phone_bk();
	tooltip.Format(L"%s    %s:%s    %s:%s    %s:%s    %s:%s    %s:%s",
				   sid,
				   fmAlias, alias.IsEmpty() ? fmNull : alias,
				   fmContact, contact.IsEmpty() ? fmNull : contact,
				   fmAddress, address.IsEmpty() ? fmNull : address,
				   fmPhone, phone.IsEmpty() ? fmNull : phone,
				   fmPhoneBk, phone_bk.IsEmpty() ? fmNull : phone_bk);
	_button->SetTooltip(tooltip);
#pragma endregion

	_button->SetButtonClkCallback(on_btnclick, this);
	_timer = std::make_shared<imagin::CTimer>(on_imagin_timer, this);
	_timer->Start(100);

	_bAlarming = _machine->get_alarming();
	_clrFace = GetEventLevelColor(_machine->get_highestEventLevel());
	if (_bAlarming) {
		StartTimer();
	}
	if (IsValidButton()) {
		_button->SetTimerEx(cTimerIdAdemco, this, on_timer);
	}
}


void CButtonEx::OnImaginTimer()
{
	AUTO_LOG_FUNCTION;
	if (_machine)
		_machine->TraverseAdmecoEventList(m_observer);
}


CButtonEx::~CButtonEx()
{
	m_observer = nullptr;
	if (_machine) {
		//_machine->UnRegisterObserver(this);
		_machine = nullptr;
	}
	_timer->Stop();

	_button->DestroyWindow();
	
	clear_alarm_event_list();
}


bool CButtonEx::IsValidButton() const 
{ 
	return (_button && IsWindow(_button->m_hWnd)); 
}


void CButtonEx::StartTimer()
{
	if (IsValidButton()) {
		_button->SetTimerEx(cTimerIdFlush, this, on_timer);
	}
}


void CButtonEx::StopTimer()
{
	if (IsValidButton()) {
		_button->KillTimerEx(cTimerIdFlush);
	}
}


void CButtonEx::clear_alarm_event_list()
{
	_alarmEventList.clear();
}


void CButtonEx::ShowWindow(int nCmdShow)
{
	if (IsValidButton()) {
		_button->ShowWindow(nCmdShow);
	}
}


void CButtonEx::OnAdemcoEventResult(const ademco::AdemcoEventPtr& ademcoEvent)
{
	if (nullptr == _machine)
		return;
	std::lock_guard<std::mutex> lock(m_lock4AlarmEventList);
	_alarmEventList.push_back(ademcoEvent);
}


void CButtonEx::UpdateButtonText()
{
	if (IsValidButton()) {
		CString alias = _machine->get_alias();
		if (alias.IsEmpty()) {
			if (_machine->get_is_submachine())
				alias.Format(L"%03d", _machine->get_submachine_zone());
			else
				alias.Format(L"%04d", _machine->get_ademco_id());
		}
		color_text_->SetWindowTextW(alias);
	}
	UpdateIconAndColor(_machine->get_online(), _machine->get_machine_status());
}


void CButtonEx::OnTimer(UINT nTimerId)
{
	if (IsValidButton()) {
		if (cTimerIdFlush == nTimerId){
			_button->SetFaceColor(_bSwitchColor ? _clrFace : cColorWhite);
			color_text_->SetFaceColor(_bSwitchColor ? _clrFace : cColorWhite);
			color_text_->SetTextColor(cColorBlack);
			_bSwitchColor = !_bSwitchColor;
			_button->Invalidate(0);
		} else if (cTimerIdAdemco == nTimerId) {
			if (m_lock4AlarmEventList.try_lock()){
				std::lock_guard<std::mutex> lock(m_lock4AlarmEventList, std::adopt_lock);
				while (_alarmEventList.size() > 0){
					const ademco::AdemcoEventPtr& ademcoEvent = _alarmEventList.front();
					HandleAdemcoEvent(ademcoEvent);
					_alarmEventList.pop_front();
				}
			}
		}
	}
}


void CButtonEx::HandleAdemcoEvent(const ademco::AdemcoEventPtr& ademcoEvent)
{
	bool bsubmachine_status = ademcoEvent->_sub_zone != core::INDEX_ZONE;
	bool bmybusinese = bsubmachine_status == _machine->get_is_submachine();

	if (ademcoEvent && IsValidButton()) {
		switch (ademcoEvent->_event) {
		case ademco::EVENT_IM_GONNA_DIE:
			_machine = nullptr;
			break;
		case ademco::EVENT_OFFLINE:
		case ademco::EVENT_ONLINE:
		case EVENT_DISARM:
		case EVENT_HALFARM:
		case EVENT_ARM:
		case EVENT_SIGNAL_STRENGTH_CHANGED:
			if (bmybusinese) {
				bool online = _machine->get_online();
				UpdateIconAndColor(online, _machine->get_machine_status());
			}

			break;
		case EVENT_CLEARMSG:
			if (_bAlarming) {
				_bAlarming = FALSE;
				StopTimer();
				bool online = _machine->get_online();
				color_text_->SetTextColor(online ? cColorBlack : cColorRed);
				_button->SetFaceColor(cColorWhite);
				color_text_->SetFaceColor(cColorWhite);
				_button->Invalidate();
			}
			break;
		case EVENT_SUBMACHINECNT:
		case EVENT_MACHINE_ALIAS:
			if (bmybusinese) {
				UpdateButtonText();
			}
			break;
		case EVENT_I_AM_NET_MODULE:
		case EVENT_I_AM_EXPRESSED_GPRS_2050_MACHINE:
			break;
		

		default:	// means its alarming
			if (bmybusinese || !_machine->get_is_submachine()) {
				_bAlarming = TRUE;
				_clrFace = GetEventLevelColor(_machine->get_highestEventLevel());
				color_text_->SetTextColor(cColorWhite);
				_button->SetFaceColor(_clrFace);
				color_text_->SetFaceColor(_clrFace);
				StopTimer();
				StartTimer();
			}
			break;
		}

		_button->Invalidate();
	}
}


void CButtonEx::UpdateIconAndColor(bool online, core::MachineStatus status)
{
	if (!IsValidButton())
		return; 

	CString exepath = GetModuleFilePath();
	//CString bmppath;

	color_text_->SetTextColor(!online ? cColorRed : cColorBlack);
	if (online) {
		//bmppath = exepath + L"\\Resource\\online.bmp";
		iconOnOffLine_->ShowBmp(exepath + L"\\Resource\\online.bmp");
	} else {
		if (_machine->get_machine_type() == core::MT_IMPRESSED_GPRS_MACHINE_2050) {
			iconOnOffLine_->ShowBmp(exepath + L"\\Resource\\phone.bmp");
		} else {
			iconOnOffLine_->ShowBmp(exepath + L"\\Resource\\offline.bmp");
		}
	}

	switch (status) {
	case core::MACHINE_ARM:
		iconStatus_->ShowBmp(exepath + L"\\Resource\\arm.bmp");
		break;
	case core::MACHINE_HALFARM:
		iconStatus_->ShowBmp(exepath + L"\\Resource\\halfarm.bmp");
		break;
	case core::MACHINE_DISARM:
	case core::MACHINE_STATUS_UNKNOWN:
	default:
		iconStatus_->ShowBmp(exepath + L"\\Resource\\disarm.bmp");
		break;
	}

	if (_machine->get_machine_type() == core::MT_IMPRESSED_GPRS_MACHINE_2050) {
		auto signal_strength = _machine->get_signal_strength();
		if (!online) {
			signal_strength = core::SIGNAL_STRENGTH_0;
		}
		switch (signal_strength)
		{
		case core::SIGNAL_STRENGTH_1:
			iconExtra_->ShowBmp(exepath + L"\\Resource\\signal1.bmp");
			break;
		case core::SIGNAL_STRENGTH_2:
			iconExtra_->ShowBmp(exepath + L"\\Resource\\signal2.bmp");
			break;
		case core::SIGNAL_STRENGTH_3:
			iconExtra_->ShowBmp(exepath + L"\\Resource\\signal3.bmp");
			break;
		case core::SIGNAL_STRENGTH_4:
			iconExtra_->ShowBmp(exepath + L"\\Resource\\signal4.bmp");
			break;
		case core::SIGNAL_STRENGTH_5:
			iconExtra_->ShowBmp(exepath + L"\\Resource\\signal5.bmp");
			break;
		case core::SIGNAL_STRENGTH_0:
		default:
			iconExtra_->ShowBmp(exepath + L"\\Resource\\signal0.bmp");
			break;
		}
		
	} else {
		if (_machine->get_submachine_count() > 0) {
			iconOnOffLine_->ShowBmp(exepath + L"\\Resource\\machine.bmp");
		}
	}

	/*HICON hIcon = nullptr;
	if (online) {
		if (_machine->get_submachine_count() > 0) {
			switch (status) {
			case core::MACHINE_ARM:
				hIcon = CAppResource::m_hIcon_Online_Arm_Hassubmachine;
				break;
			case core::MACHINE_HALFARM:
				hIcon = CAppResource::m_hIcon_Online_Halfarm_Hassubmachine;
				break;
			case core::MACHINE_DISARM:
			case core::MACHINE_STATUS_UNKNOWN:
			default:
				hIcon = CAppResource::m_hIcon_Online_Disarm_Hassubmachine;
				break;
			}
				
		} else {
			switch (status) {
			case core::MACHINE_ARM:
				hIcon = CAppResource::m_hIcon_Online_Arm;
				break;
			case core::MACHINE_HALFARM:
				hIcon = CAppResource::m_hIcon_Online_Halfarm;
				break;
			case core::MACHINE_DISARM:
			case core::MACHINE_STATUS_UNKNOWN:
			default:
				hIcon = CAppResource::m_hIcon_Online_Disarm;
				break;
			}
		}
	} else {
		if (_machine->get_machine_type() == core::MT_IMPRESSED_GPRS_MACHINE_2050) {
			switch (status) {
			case core::MACHINE_ARM: 
				hIcon = CAppResource::m_hIcon_Gsm_Arm;
				break;
			case core::MACHINE_HALFARM:
				hIcon = CAppResource::m_hIcon_Gsm_Halfarm;
				break;
			case core::MACHINE_DISARM:
			case core::MACHINE_STATUS_UNKNOWN:
				hIcon = CAppResource::m_hIcon_Gsm_Disarm;
			default:
				break;
			}
		} else {
			if (_machine->get_submachine_count() > 0) {
				switch (status) {
				case core::MACHINE_ARM:
					hIcon = CAppResource::m_hIcon_Offline_Arm_Hassubmachine;
					break;
				case core::MACHINE_HALFARM:
					hIcon = CAppResource::m_hIcon_Offline_Halfarm_Hassubmachine;
					break;
				case core::MACHINE_DISARM:
				case core::MACHINE_STATUS_UNKNOWN:
				default:
					hIcon = CAppResource::m_hIcon_Offline_Disarm_Hassubmachine;
					break;
				}
			} else {
				switch (status) {
				case core::MACHINE_ARM:
					hIcon = CAppResource::m_hIcon_Offline_Arm;
					break;
				case core::MACHINE_HALFARM:
					hIcon = CAppResource::m_hIcon_Offline_Halfarm;
					break;
				case core::MACHINE_DISARM:
				case core::MACHINE_STATUS_UNKNOWN:
				default:
					hIcon = CAppResource::m_hIcon_Offline_Disarm;
					break;
				}
			}
		}
	}
	_button->SetIcon(hIcon);*/
	_button->Invalidate();
}


void CButtonEx::OnBnClicked()
{
	_button->SetFocus();
	_button->Invalidate();
	if (_machine && _wndParent && IsWindow(_wndParent->GetSafeHwnd())) {
		_wndParent->SendMessage(WM_BNCLKEDEX, 0, _machine->get_is_submachine() ? _machine->get_submachine_zone() : _machine->get_ademco_id());
	}
	_button->Invalidate();
}


void CButtonEx::OnRBnClicked()
{
	USES_CONVERSION;
	_button->SetFocus();
	_button->Invalidate();
	CMenu menu, *subMenu;
	menu.LoadMenuW(IDR_MENU1);
	subMenu = menu.GetSubMenu(0);
	if (subMenu == nullptr) return;

	/*if (!_machine->IsOnline()) {
		subMenu->EnableMenuItem(2, MF_BYPOSITION | MF_DISABLED | MF_GRAYED);
		subMenu->EnableMenuItem(3, MF_BYPOSITION | MF_DISABLED | MF_GRAYED);
		subMenu->EnableMenuItem(4, MF_BYPOSITION | MF_DISABLED | MF_GRAYED);
	}

	if (!_bAlarming) {
		subMenu->EnableMenuItem(6, MF_BYPOSITION | MF_DISABLED | MF_GRAYED);
	}*/

//#ifdef _DEBUG
#define ENABLE_REMOTE_CONTROL
//#endif

#ifndef ENABLE_REMOTE_CONTROL
	subMenu->DeleteMenu(1, MF_BYPOSITION);
	subMenu->DeleteMenu(1, MF_BYPOSITION);
	subMenu->DeleteMenu(1, MF_BYPOSITION);
	subMenu->DeleteMenu(1, MF_BYPOSITION);
	subMenu->DeleteMenu(1, MF_BYPOSITION);
#endif

	if (_machine->get_is_submachine() || core::MT_NETMOD != _machine->get_machine_type()) {
		subMenu->DeleteMenu(3, MF_BYPOSITION);
	}

	CRect rc;
	_button->GetWindowRect(rc);
	int ret = subMenu->TrackPopupMenu(TPM_LEFTALIGN | TPM_RIGHTBUTTON | TPM_RETURNCMD,
									  rc.left, rc.bottom, _button.get());

	core::CAlarmMachineManager* manager = core::CAlarmMachineManager::GetInstance();
	
	switch (ret) {
		case ID_DDD_32771: // open
			OnBnClicked();
			break;
		case ID_DDD_32772: // arm
		{
			bool sms_mode = _machine->get_sms_mode();
			CString txt;
			txt.Format(L"%s(%06d,%s) ", GetStringFromAppResource(IDS_STRING_MACHINE), _machine->get_ademco_id(), _machine->get_alias());
			txt += GetStringFromAppResource(IDS_STRING_ENTER_SMS_MODE);
			if (sms_mode) {
				_button->MessageBox(txt);
				return;
			} else if (_machine->get_is_submachine()) {
				auto parent_machine = manager->GetMachine(_machine->get_ademco_id());
				if (parent_machine) {
					sms_mode = parent_machine->get_sms_mode();
					if (sms_mode) {
						txt.Format(L"%s(%06d,%s) ", GetStringFromAppResource(IDS_STRING_MACHINE), _machine->get_ademco_id(), parent_machine->get_alias());
						txt += GetStringFromAppResource(IDS_STRING_ENTER_SMS_MODE);
						_button->MessageBox(txt);
						return;
					}
				}
			}
		}

			manager->RemoteControlAlarmMachine(_machine, ademco::EVENT_ARM,
												_machine->get_is_submachine() ? core::INDEX_SUB_MACHINE : core::INDEX_ZONE,
												_machine->get_is_submachine() ? _machine->get_submachine_zone() : 0,
												nullptr, nullptr, ES_UNKNOWN, _button.get());
			
		
		break;
		case ID_DDD_32786: // halfarm
		{
			{
				bool sms_mode = _machine->get_sms_mode();
				CString txt;
				txt.Format(L"%s(%06d,%s) ", GetStringFromAppResource(IDS_STRING_MACHINE), _machine->get_ademco_id(), _machine->get_alias());
				txt += GetStringFromAppResource(IDS_STRING_ENTER_SMS_MODE);
				if (sms_mode) {
					_button->MessageBox(txt);
					return;
				} else if (_machine->get_is_submachine()) {
					auto parent_machine = manager->GetMachine(_machine->get_ademco_id());
					if (parent_machine) {
						sms_mode = parent_machine->get_sms_mode();
						if (sms_mode) {
							txt.Format(L"%s(%06d,%s) ", GetStringFromAppResource(IDS_STRING_MACHINE), _machine->get_ademco_id(), parent_machine->get_alias());
							txt += GetStringFromAppResource(IDS_STRING_ENTER_SMS_MODE);
							_button->MessageBox(txt);
							return;
						}
					}
				}
			}
			auto xdata = std::make_shared<ademco::char_array>();
			if (_machine->get_machine_status() == core::MACHINE_ARM) {
				if (!_machine->get_is_submachine()) {
					CInputDlg dlg(_button.get());
					if (dlg.DoModal() != IDOK)
						return;
					if (dlg.m_edit.GetLength() != 6)
						return;
					const char* a = W2A(dlg.m_edit);
					for (int i = 0; i < 6; i++) {
						xdata->push_back(a[i]);
					}
				}
			}
			manager->RemoteControlAlarmMachine(_machine, ademco::EVENT_HALFARM,
											   _machine->get_is_submachine() ? core::INDEX_SUB_MACHINE : core::INDEX_ZONE,
											   _machine->get_is_submachine() ? _machine->get_submachine_zone() : 0,
											   xdata, nullptr, ES_UNKNOWN, _button.get());
		}
			break;
		case ID_DDD_32773: { // disarm
			{
				bool sms_mode = _machine->get_sms_mode();
				CString txt;
				txt.Format(L"%s(%06d,%s) ", GetStringFromAppResource(IDS_STRING_MACHINE), _machine->get_ademco_id(), _machine->get_alias());
				txt += GetStringFromAppResource(IDS_STRING_ENTER_SMS_MODE);
				if (sms_mode) {
					_button->MessageBox(txt);
					return;
				} else if (_machine->get_is_submachine()) {
					auto parent_machine = manager->GetMachine(_machine->get_ademco_id());
					if (parent_machine) {
						sms_mode = parent_machine->get_sms_mode();
						if (sms_mode) {
							txt.Format(L"%s(%06d,%s) ", GetStringFromAppResource(IDS_STRING_MACHINE), _machine->get_ademco_id(), parent_machine->get_alias());
							txt += GetStringFromAppResource(IDS_STRING_ENTER_SMS_MODE);
							_button->MessageBox(txt);
							return;
						}
					}
				}
			}
			auto xdata = std::make_shared<ademco::char_array>();
			if (!_machine->get_is_submachine()) {
				CInputDlg dlg(_button.get());
				if (dlg.DoModal() != IDOK)
					return ;
				if (dlg.m_edit.GetLength() != 6)
					return ;
				const char* a = W2A(dlg.m_edit);
				for (int i = 0; i < 6; i++) {
					xdata->push_back(a[i]);
				}
			}
			manager->RemoteControlAlarmMachine(_machine, ademco::EVENT_DISARM, 
											   _machine->get_is_submachine() ? core::INDEX_SUB_MACHINE : core::INDEX_ZONE,
											   _machine->get_is_submachine() ? _machine->get_submachine_zone() : 0,
											   xdata, nullptr, ES_UNKNOWN, _button.get());
			break; 
		}
		case ID_DDD_32774: // emergency
		{
			bool sms_mode = _machine->get_sms_mode();
			CString txt;
			txt.Format(L"%s(%06d,%s) ", GetStringFromAppResource(IDS_STRING_MACHINE), _machine->get_ademco_id(), _machine->get_alias());
			txt += GetStringFromAppResource(IDS_STRING_ENTER_SMS_MODE);
			if (sms_mode) {
				_button->MessageBox(txt);
				return;
			} else if (_machine->get_is_submachine()) {
				auto parent_machine = manager->GetMachine(_machine->get_ademco_id());
				if (parent_machine) {
					sms_mode = parent_machine->get_sms_mode();
					if (sms_mode) {
						txt.Format(L"%s(%06d,%s) ", GetStringFromAppResource(IDS_STRING_MACHINE), _machine->get_ademco_id(), parent_machine->get_alias());
						txt += GetStringFromAppResource(IDS_STRING_ENTER_SMS_MODE);
						_button->MessageBox(txt);
						return;
					}
				}
			}
		}
			manager->RemoteControlAlarmMachine(_machine, ademco::EVENT_EMERGENCY, 
											   _machine->get_is_submachine() ? core::INDEX_SUB_MACHINE : core::INDEX_ZONE,
											   _machine->get_is_submachine() ? _machine->get_submachine_zone() : 0,
											   nullptr, nullptr, ES_UNKNOWN, _button.get());
			break;
		case ID_DDD_32775: // clear msg
			if (_machine) {
				_machine->clear_ademco_event_list();
			}
			break;
		case ID_DDD_32785: // show baidu map
			if (g_baiduMapDlg) {
				g_baiduMapDlg->ShowMap(_machine);
			}
			break;
		default:
			break;

	}
	_button->Invalidate();
}


void CButtonEx::MoveWindow(const CRect& rc, BOOL bRepaint)
{
	if (IsValidButton()) {
		_button->MoveWindow(rc, bRepaint);
	}
}

NAMESPACE_END
