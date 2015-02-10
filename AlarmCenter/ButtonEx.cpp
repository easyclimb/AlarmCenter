#include "stdafx.h"
#include "ButtonEx.h"
#include "MFCButtonEx.h"
#include "BtnST.h"
#include "AlarmMachine.h"
#include "./imagin/Timer.h"
#include "AlarmMachineContainer.h"
#include "AlarmMachineManager.h"
using namespace ademco;

namespace gui {

IMPLEMENT_ADEMCO_EVENT_CALL_BACK(CButtonEx, OnAdemcoEvent)

static void __stdcall on_timer(imagin::CTimer* /*timer*/, void* udata)
{
	CButtonEx* btn = reinterpret_cast<CButtonEx*>(udata); ASSERT(btn);
	btn->OnTimer();
}

static void __stdcall on_btnclick(control::ButtonClick bc, void* udata) {
	CButtonEx* btn = reinterpret_cast<CButtonEx*>(udata); ASSERT(btn);
	if (bc == control::BC_LEFT) {
		btn->OnBnClicked();
	} else if (bc == control::BC_RIGHT) {
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
	, _bRed(FALSE)
	, _timer(NULL)
	, _machine(machine)
	, _bAlarming(FALSE)
{
	assert(machine);
	machine->RegisterObserver(this, OnAdemcoEvent);
	_button = new control::CMFCButtonEx();
	_button->Create(text, WS_CHILD | WS_VISIBLE | BS_ICON, rc, parent, id);
	ASSERT(IsWindow(_button->m_hWnd));
	
	CString tooltip = L"", fmAlias, fmContact, fmAddress, fmPhone, fmPhoneBk, fmNull;
	CString alias, contact, address, phone, phone_bk;
	fmAlias.LoadStringW(IDS_STRING_ALIAS);
	fmContact.LoadStringW(IDS_STRING_CONTACT);
	fmAddress.LoadStringW(IDS_STRING_ADDRESS);
	fmPhone.LoadStringW(IDS_STRING_PHONE);
	fmPhoneBk.LoadStringW(IDS_STRING_PHONE_BK);
	fmNull.LoadStringW(IDS_STRING_NULL);

	alias = _machine->get_alias();
	contact = _machine->get_contact();
	address = _machine->get_address();
	phone = _machine->get_phone();
	phone_bk = _machine->get_phone_bk();

	tooltip.Format(L"ID:%04d    %s:%s    %s:%s    %s:%s    %s:%s    %s:%s",
				   _machine->get_ademco_id(),
				   fmAlias, alias.IsEmpty() ? fmNull : alias,
				   fmContact, contact.IsEmpty() ? fmNull : contact,
				   fmAddress, address.IsEmpty() ? fmNull : address,
				   fmPhone, phone.IsEmpty() ? fmNull : phone,
				   fmPhoneBk, phone_bk.IsEmpty() ? fmNull : phone_bk);
	_button->SetTooltip(tooltip);
	_button->SetButtonClkCallback(on_btnclick, this);
	_timer = new imagin::CTimer(on_timer, this);
}


CButtonEx::~CButtonEx()
{
	_button->DestroyWindow();
	delete _button;
	delete _timer;
	clear_alarm_event_list();
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
	if (_button && IsWindow(_button->m_hWnd)) {
		_button->SetTextColor(RGB(255, 0, 0));
		_button->SetIcon(CAlarmMachineContainerDlg::m_hIconNetFailed);
		_button->ShowWindow(nCmdShow);
	}
}


void CButtonEx::OnAdemcoEventResult(const AdemcoEvent* ademcoEvent)
{
	//if (_ademco_event != ademco_event) {
	//	//if (IsStandardStatus(status)) {
	//	_ademco_event = ademco_event;
	//	//}

	if (ademcoEvent&& _button && IsWindow(_button->m_hWnd)) {
		switch (ademcoEvent->_event) {
			case MS_OFFLINE:
				_button->SetTextColor(RGB(255, 0, 0));
				_button->SetIcon(CAlarmMachineContainerDlg::m_hIconNetFailed);
				break;
			case MS_ONLINE:
				_button->SetTextColor(RGB(0, 0, 0));
				_button->SetIcon(CAlarmMachineContainerDlg::m_hIconNetOk);
				break;
			case EVENT_DISARM:
				_button->SetTextColor(RGB(0, 0, 0));
				_button->SetIcon(CAlarmMachineContainerDlg::m_hIconDisarm);
				break;
			case EVENT_ARM:
				_button->SetTextColor(RGB(0, 0, 0));
				_button->SetIcon(CAlarmMachineContainerDlg::m_hIconArm);
				break;
			case EVENT_CLEARMSG:
				if (_bAlarming) {
					_bAlarming = FALSE;
					_button->SetTextColor(RGB(0, 0, 0));
					_button->SetFaceColor(RGB(255, 255, 255));
					_timer->Stop();
				}
				break;
			default:	// means its alarming
				_bAlarming = TRUE;
				_button->SetTextColor(RGB(0, 0, 0));
				_button->SetFaceColor(RGB(255, 0, 0));
				//_button->SetIcon(CAlarmMachineContainerDlg::m_hIconNetFailed);
				_timer->Stop();
				_timer->Start(FLASH_GAP, true);
				//m_lock4AlarmEventList.Lock();
				//_alarmEventList.push_back(new AdemcoEvent(*ademcoEvent));
				//m_lock4AlarmEventList.UnLock();
				break;
		}
		_button->Invalidate();

		
	}
	//}
}


void CButtonEx::OnTimer()
{
	if (_button && IsWindow(_button->m_hWnd)) {
		_button->SetFaceColor(_bRed ? RGB(255, 0, 0) : RGB(255, 255, 255));
		_button->SetTextColor(_bRed ? RGB(0, 0, 0) : RGB(255, 0, 0));
		//_button->SetColor(gui::control::CButtonST::BTNST_COLOR_BK_OUT, _bRed ? RGB(255, 0, 0) : RGB(255, 255, 255));
		//_button->SetColor(gui::control::CButtonST::BTNST_COLOR_FG_OUT, _bRed ? RGB(0, 0, 0) : RGB(255, 0, 0));
		_bRed = !_bRed;
		_button->Invalidate();
		_timer->Start(FLASH_GAP, true);
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
			manager->RemoteControlAlarmMachine(_machine, ademco::EVENT_ARM, _button);
			break;
		case ID_DDD_32773: // disarm
			manager->RemoteControlAlarmMachine(_machine, ademco::EVENT_DISARM, _button);
			break;
		case ID_DDD_32774: // emergency
			manager->RemoteControlAlarmMachine(_machine, ademco::EVENT_EMERGENCY, _button);
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

NAMESPACE_END
