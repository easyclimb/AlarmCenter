#include "stdafx.h"
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
using namespace ademco;

namespace gui {

IMPLEMENT_ADEMCO_EVENT_CALL_BACK(CButtonEx, OnAdemcoEvent)

static void __stdcall on_imagin_timer(imagin::CTimer* /*timer*/, void* udata)
{
	CButtonEx* btn = reinterpret_cast<CButtonEx*>(udata); ASSERT(btn);
	//TraverseAdmecoEventList(udata, OnAdemcoEvent);
	btn->OnImaginTimer();
}

static void __stdcall on_timer(/*imagin::CTimer* timer, */void* udata)
{
	CButtonEx* btn = reinterpret_cast<CButtonEx*>(udata); ASSERT(btn);
	btn->OnTimer();
}


//static void __stdcall on_timer(HWND hWnd, UINT uMsg, UINT_PTR idEvent, DWORD dwTime)
//{
//	
//}


static void __stdcall on_btnclick(ButtonClick bc, void* udata) {
	CButtonEx* btn = reinterpret_cast<CButtonEx*>(udata); ASSERT(btn);
	if (bc == BC_LEFT) {
		btn->OnBnClicked();
	} else if (bc == BC_RIGHT) {
		btn->OnRBnClicked();
	}
}

CButtonEx::CButtonEx(const wchar_t* text,
					 const RECT& rc,
					 CWnd* parent,
					 UINT id,
					 core::CAlarmMachine* machine)
	: _button(NULL)
	, _wndParent(parent)
	//, _data(data)
	//, _ademco_event(MS_OFFLINE)
	, _bSwitchColor(FALSE)
	, _timer(NULL)
	, _machine(machine)
	, _bAlarming(FALSE)
	, _clrText(RGB(255, 255, 255))
	, _clrFace(RGB(0, 0, 0))
{
	AUTO_LOG_FUNCTION;
	assert(machine);
	machine->RegisterObserver(this, OnAdemcoEvent);
	_button = new CMFCButtonEx();
	_button->Create(text, WS_CHILD | WS_VISIBLE | BS_ICON, rc, parent, id);
	ASSERT(IsWindow(_button->m_hWnd));
	UpdateButtonText();

	if (machine->get_online()) {
		_button->SetTextColor(RGB(0, 0, 0));
		_button->SetIcon(CAppResource::m_hIconNetOk);
		if (_machine->get_armed())
			_button->SetIcon(CAppResource::m_hIconArm);
		else 
			_button->SetIcon(CAppResource::m_hIconDisarm);
	} else {
		_button->SetTextColor(RGB(255, 0, 0));
		_button->SetIcon(CAppResource::m_hIconNetFailed);
	}
	
#pragma region set tooltip
	CString tooltip = L"", fmAlias, fmContact, fmAddress, fmPhone, fmPhoneBk, fmNull;
	CString sid, alias, contact, address, phone, phone_bk;
	fmAlias.LoadStringW(IDS_STRING_ALIAS);
	fmContact.LoadStringW(IDS_STRING_CONTACT);
	fmAddress.LoadStringW(IDS_STRING_ADDRESS);
	fmPhone.LoadStringW(IDS_STRING_PHONE);
	fmPhoneBk.LoadStringW(IDS_STRING_PHONE_BK);
	fmNull.LoadStringW(IDS_STRING_NULL);
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
	_timer = new imagin::CTimer(on_imagin_timer, this);
	_timer->Start(100);
	//_machine->TraverseAdmecoEventList(this, OnAdemcoEvent);

	_bAlarming = _machine->get_alarming();
	_clrFace = GetEventLevelColor(_machine->get_highestEventLevel());
	if (_bAlarming) {
		StartTimer();
	}
}

void CButtonEx::OnImaginTimer()
{
	AUTO_LOG_FUNCTION;
	if (_machine)
		_machine->TraverseAdmecoEventList(this, OnAdemcoEvent);
}


CButtonEx::~CButtonEx()
{
	if (_machine) {
		_machine->UnRegisterObserver(this);
		_machine = NULL;
	}
	_timer->Stop();
	delete _timer;

	_button->DestroyWindow();
	delete _button;
	
	clear_alarm_event_list();
}


bool CButtonEx::IsValidButton() const 
{ 
	return (_button && IsWindow(_button->m_hWnd)); 
}


void CButtonEx::StartTimer()
{
	if (IsValidButton()) {
		_button->SetTimerEx(this, on_timer);
	}
}


void CButtonEx::StopTimer()
{
	if (IsValidButton()) {
		_button->KillTimerEx();
	}
}


void CButtonEx::clear_alarm_event_list()
{
	std::list<AdemcoEvent*>::iterator iter = _alarmEventList.begin();
	while (iter != _alarmEventList.end()) {
		AdemcoEvent* ademcoEvent = *iter++;
		delete ademcoEvent;
	}
	_alarmEventList.clear();
}


void CButtonEx::ShowWindow(int nCmdShow)
{
	if (IsValidButton()) {
		// _button->SetTextColor(RGB(255, 0, 0));
		// _button->SetIcon(CAppResource::m_hIconNetFailed);
		_button->ShowWindow(nCmdShow);
	}
}


void CButtonEx::OnAdemcoEventResult(const AdemcoEvent* ademcoEvent)
{
	if (NULL == _machine)
		return;

	bool bsubmachine_status = ademcoEvent->_sub_zone != core::INDEX_ZONE;
	bool bmybusinese = bsubmachine_status == _machine->get_is_submachine();
	if (ademcoEvent && IsValidButton()) {
		switch (ademcoEvent->_event) {
			case ademco::EVENT_IM_GONNA_DIE:
				_machine = NULL;
				break;
			case ademco::EVENT_OFFLINE:
				if (bmybusinese) {
					_button->SetTextColor(RGB(255, 0, 0));
					_button->SetIcon(CAppResource::m_hIconNetFailed);
				}
				break;
			case ademco::EVENT_ONLINE:
				if (bmybusinese) {
					_button->SetTextColor(RGB(0, 0, 0));
					_button->SetIcon(CAppResource::m_hIconNetOk);
				}
				break;
			case EVENT_DISARM:
				if (bmybusinese) {
					_button->SetTextColor(RGB(0, 0, 0));
					_button->SetIcon(CAppResource::m_hIconDisarm);
				}
				break;
			case EVENT_ARM:
				if (bmybusinese) {
					_button->SetTextColor(RGB(0, 0, 0));
					_button->SetIcon(CAppResource::m_hIconArm);
				}
				break;
			case EVENT_CLEARMSG:
				if (/*bmybusinese && */_bAlarming) {
					_bAlarming = FALSE;
					bool online = _machine->get_online();
					_button->SetTextColor(online ? RGB(0, 0, 0) : RGB(255, 0, 0));
					_button->SetFaceColor(RGB(255, 255, 255));
					//_timer->Stop();
					StopTimer();
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
					//_button->SetIcon(CAlarmMachineContainerDlg::m_hIconNetFailed);
					//_timer->Stop();
					StopTimer();
					//_timer->Start(FLASH_GAP, true);
					StartTimer();
					//m_lock4AlarmEventList.Lock();
					//_alarmEventList.push_back(new AdemcoEvent(*ademcoEvent));
					//m_lock4AlarmEventList.UnLock();
				} 
				break;
		}
		_button->Invalidate();
	}
}


void CButtonEx::UpdateButtonText()
{
	CString alias = _machine->get_alias();
	if (alias.IsEmpty()) {
		if (_machine->get_is_submachine())
			alias.Format(L"%03d", _machine->get_submachine_zone());
		else
			alias.Format(L"%04d", _machine->get_ademco_id());
	}
	int count = _machine->get_submachine_count();
	if (count > 0) {
		CString txt, fm;
		fm.LoadStringW(IDS_STRING_HAS_SUB_MACHINE);
		txt.Format(L"[%s]%s", fm, alias);
		_button->SetWindowTextW(txt);
	} else {
		_button->SetWindowTextW(alias);
	}
}


void CButtonEx::OnTimer()
{
	if (IsValidButton()) {
		_button->SetFaceColor(_bSwitchColor ? _clrFace : RGB(255, 255, 255));
		//_button->SetTextColor(_bSwitchColor ? _clrText : RGB(0, 0, 0));
		_button->SetTextColor(RGB(0, 0, 0));
		//_button->SetColor(gui::control::CButtonST::BTNST_COLOR_BK_OUT, _bSwitchColor ? RGB(255, 0, 0) : RGB(255, 255, 255));
		//_button->SetColor(gui::control::CButtonST::BTNST_COLOR_FG_OUT, _bSwitchColor ? RGB(0, 0, 0) : RGB(255, 0, 0));
		_bSwitchColor = !_bSwitchColor;
		_button->Invalidate(0);
		//_timer->Start(FLASH_GAP, true);
		//CLog::WriteLog(L"OnTimer");
	}
}


void CButtonEx::OnBnClicked()
{
	if (_machine && _wndParent && IsWindow(_wndParent->GetSafeHwnd())) {
		_wndParent->PostMessage(WM_BNCLKEDEX, 0, reinterpret_cast<LPARAM>(_machine));
	}
}


void CButtonEx::OnRBnClicked()
{
	//if (_machine && _wndParent && IsWindow(_wndParent->GetSafeHwnd())) {
	//	_wndParent->PostMessage(WM_BNCLKEDEX, 1, reinterpret_cast<LPARAM>(_machine));
	//}
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

#define ENABLE_REMOTE_CONTROL

#ifndef ENABLE_REMOTE_CONTROL
	subMenu->DeleteMenu(1, MF_BYPOSITION);
	subMenu->DeleteMenu(1, MF_BYPOSITION);
	subMenu->DeleteMenu(1, MF_BYPOSITION);
	subMenu->DeleteMenu(1, MF_BYPOSITION);
	subMenu->DeleteMenu(1, MF_BYPOSITION);
#endif

	CRect rc;
	_button->GetWindowRect(rc);
	int ret = subMenu->TrackPopupMenu(TPM_LEFTALIGN | TPM_RIGHTBUTTON | TPM_RETURNCMD,
									  rc.left, rc.bottom, _button);

	core::CAlarmMachineManager* manager = core::CAlarmMachineManager::GetInstance();
	
	switch (ret) {
		case ID_DDD_32771: // open
			OnBnClicked();
			break;
		case ID_DDD_32772: // arm
			manager->RemoteControlAlarmMachine(_machine, ademco::EVENT_ARM, 0, 0, 
											   NULL, 0, _button);
			break;
		case ID_DDD_32773: {// disarm
			char xdata[64] = { 0 };
			int xdata_len = 0;
			if (!_machine->get_is_submachine()) {
				CInputDlg dlg(_button);
				if (dlg.DoModal() != IDOK)
					return ;
				if (dlg.m_edit.GetLength() != 6)
					return ;

				USES_CONVERSION;
				strcpy_s(xdata, W2A(dlg.m_edit));
				xdata_len = strlen(xdata);
			}
			manager->RemoteControlAlarmMachine(_machine, ademco::EVENT_DISARM, 0, 0, 
											   xdata, xdata_len, _button);
			break; 
		}
		case ID_DDD_32774: // emergency
			manager->RemoteControlAlarmMachine(_machine, ademco::EVENT_EMERGENCY, 0, 0, 
											   NULL, 0, _button);
			break;
		case ID_DDD_32775: // clear msg
			if (_machine) {
				_machine->clear_ademco_event_list();
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
