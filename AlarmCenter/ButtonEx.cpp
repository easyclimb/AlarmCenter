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

using namespace ademco;

namespace gui {

namespace {
	const int cTimerIdFlush = 1;
	const int cTimerIdAdemco = 2;

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

CButtonEx::CButtonEx(const wchar_t* text,
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
	, _clrText(RGB(255, 255, 255))
	, _clrFace(RGB(0, 0, 0))
{
	AUTO_LOG_FUNCTION;
	assert(machine);
	m_observer = std::make_shared<ObserverType>(this);
	machine->register_observer(m_observer);
	//machine->register_observer(this, OnAdemcoEvent);
	_button = std::make_shared<CMFCButtonEx>();
	_button->Create(text, WS_CHILD | WS_VISIBLE | BS_ICON, rc, parent, id);
	ASSERT(IsWindow(_button->m_hWnd));
	UpdateButtonText();

	_button->SetFaceColor(RGB(255, 255, 255));
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
	m_lock4AlarmEventList.Lock();
	_alarmEventList.push_back(ademcoEvent);
	m_lock4AlarmEventList.UnLock();
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
		_button->SetWindowTextW(alias);
	}
	UpdateIconAndColor(_machine->get_online(), _machine->get_machine_status());
}


void CButtonEx::OnTimer(UINT nTimerId)
{
	if (IsValidButton()) {
		if (cTimerIdFlush == nTimerId){
			_button->SetFaceColor(_bSwitchColor ? _clrFace : RGB(255, 255, 255));
			_button->SetTextColor(RGB(0, 0, 0));
			_bSwitchColor = !_bSwitchColor;
			_button->Invalidate(0);
		} else if (cTimerIdAdemco == nTimerId) {
			if (m_lock4AlarmEventList.TryLock()){
				while (_alarmEventList.size() > 0){
					const ademco::AdemcoEventPtr& ademcoEvent = _alarmEventList.front();
					HandleAdemcoEvent(ademcoEvent);
					_alarmEventList.pop_front();
				}
				m_lock4AlarmEventList.UnLock();
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
			if (bmybusinese) {
				bool online = _machine->get_online();
				//bool arm = _machine->get_armed();
				UpdateIconAndColor(online, _machine->get_machine_status());
			}

			break;
		case EVENT_CLEARMSG:
			if (_bAlarming) {
				_bAlarming = FALSE;
				StopTimer();
				bool online = _machine->get_online();
				_button->SetTextColor(online ? RGB(0, 0, 0) : RGB(255, 0, 0));
				_button->SetFaceColor(RGB(255, 255, 255));
			}
			break;
		case EVENT_SUBMACHINECNT:
		case EVENT_MACHINE_ALIAS:
			if (bmybusinese) {
				UpdateButtonText();
			}
			break;
		case EVENT_I_AM_NET_MODULE:
			break;
		default:	// means its alarming
			if (bmybusinese || !_machine->get_is_submachine()) {
				_bAlarming = TRUE;
				_clrFace = GetEventLevelColor(_machine->get_highestEventLevel());
				_button->SetTextColor(RGB(0, 0, 0));
				_button->SetFaceColor(_clrFace);
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
	if (IsValidButton()) {
		_button->SetTextColor(!online ? RGB(255, 0, 0) : RGB(0, 0, 0));
		HICON hIcon = nullptr;
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
			if (_machine->get_machine_type() == core::MT_GPRS) {
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
		_button->SetIcon(hIcon);
	}
}


void CButtonEx::OnBnClicked()
{
	if (_machine && _wndParent && IsWindow(_wndParent->GetSafeHwnd())) {
		_wndParent->SendMessage(WM_BNCLKEDEX, 0, _machine->get_is_submachine() ? _machine->get_submachine_zone() : _machine->get_ademco_id());
	}
}


void CButtonEx::OnRBnClicked()
{
	USES_CONVERSION;
	CMenu menu, *subMenu;
	menu.LoadMenuW(IDR_MENU1);
	subMenu = menu.GetSubMenu(0);

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
			manager->RemoteControlAlarmMachine(_machine, ademco::EVENT_ARM, 
											   _machine->get_is_submachine() ? core::INDEX_SUB_MACHINE : core::INDEX_ZONE, 
											   _machine->get_is_submachine() ? _machine->get_submachine_zone() : 0,
											   nullptr, _button.get());
			break;
		case ID_DDD_32786: // halfarm
		{
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
											   xdata, _button.get());
		}
			break;
		case ID_DDD_32773: { // disarm
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
											   xdata, _button.get());
			break; 
		}
		case ID_DDD_32774: // emergency
			manager->RemoteControlAlarmMachine(_machine, ademco::EVENT_EMERGENCY, 
											   _machine->get_is_submachine() ? core::INDEX_SUB_MACHINE : core::INDEX_ZONE,
											   _machine->get_is_submachine() ? _machine->get_submachine_zone() : 0,
											   nullptr, _button.get());
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

}


void CButtonEx::MoveWindow(const CRect& rc, BOOL bRepaint)
{
	if (IsValidButton()) {
		_button->MoveWindow(rc, bRepaint);
	}
}

NAMESPACE_END
