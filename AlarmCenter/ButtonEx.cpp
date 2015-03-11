#include "stdafx.h"
#include "ButtonEx.h"
#include "MFCButtonEx.h"
#include "BtnST.h"
#include "AlarmMachine.h"
#include "./imagin/Timer.h"
#include "AlarmMachineContainer.h"
#include "AlarmMachineManager.h"
#include "AppResource.h"
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
	, _bRed(FALSE)
	, _timer(NULL)
	, _machine(machine)
	, _bAlarming(FALSE)
{
	LOG_FUNCTION_AUTO;
	assert(machine);
	machine->RegisterObserver(this, OnAdemcoEvent);
	_button = new CMFCButtonEx();
	_button->Create(text, WS_CHILD | WS_VISIBLE | BS_ICON, rc, parent, id);
	ASSERT(IsWindow(_button->m_hWnd));

	if (machine->IsOnline()) {
		_button->SetTextColor(RGB(0, 0, 0));
		_button->SetIcon(CAppResource::m_hIconNetOk);

		if (_machine->IsArmed()) {
			_button->SetIcon(CAppResource::m_hIconArm);
		} else {
			_button->SetIcon(CAppResource::m_hIconDisarm);
		}

	} else {
		_button->SetTextColor(RGB(255, 0, 0));
		_button->SetIcon(CAppResource::m_hIconNetFailed);
	}

	
	
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
	//CToolTipCtrl ctrl = _button->GetToolTipCtrl();
	//ctrl.SendMessage(TTM_SETMAXTIPWIDTH, 0, 400);
	//if (ctrl.GetToolCount() == 0) {
	//	CRect rectBtn;
	//	_button->GetClientRect(rectBtn);
	//	ctrl.AddTool(_button, tooltip, rectBtn, 1);
	//} // if

	//// Set text for tooltip
	//ctrl.UpdateTipText(tooltip, _button, 1);
	////ctrl.Activate(0);
	//_button->EnableFullTextTooltip();

	_button->SetButtonClkCallback(on_btnclick, this);
	_timer = new imagin::CTimer(on_imagin_timer, this);
	_timer->Start(100);
	//_machine->TraverseAdmecoEventList(this, OnAdemcoEvent);
}

void CButtonEx::OnImaginTimer()
{
	LOG_FUNCTION_AUTO;
	if (_machine)
		_machine->TraverseAdmecoEventList(this, OnAdemcoEvent);
}


CButtonEx::~CButtonEx()
{
	_machine->UnRegisterObserver(this);

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
	//if (_ademco_event != ademco_event) {
	//	//if (IsStandardStatus(status)) {
	//	_ademco_event = ademco_event;
	//	//}

	if (ademcoEvent && IsValidButton()) {
		switch (ademcoEvent->_event) {
			case MS_OFFLINE:
				_button->SetTextColor(RGB(255, 0, 0));
				_button->SetIcon(CAppResource::m_hIconNetFailed);
				break;
			case MS_ONLINE:
				_button->SetTextColor(RGB(0, 0, 0));
				_button->SetIcon(CAppResource::m_hIconNetOk);
				break;
			case EVENT_DISARM:
				_button->SetTextColor(RGB(0, 0, 0));
				_button->SetIcon(CAppResource::m_hIconDisarm);
				break;
			case EVENT_ARM:
				_button->SetTextColor(RGB(0, 0, 0));
				_button->SetIcon(CAppResource::m_hIconArm);
				break;
			case EVENT_CLEARMSG:
				if (_bAlarming) {
					_bAlarming = FALSE;
					_button->SetTextColor(RGB(0, 0, 0));
					_button->SetFaceColor(RGB(255, 255, 255));
					//_timer->Stop();
					StopTimer();
				}
				break;
			default:	// means its alarming
				_bAlarming = TRUE;
				_button->SetTextColor(RGB(0, 0, 0));
				_button->SetFaceColor(RGB(255, 0, 0));
				//_button->SetIcon(CAlarmMachineContainerDlg::m_hIconNetFailed);
				//_timer->Stop();
				StopTimer();
				//_timer->Start(FLASH_GAP, true);
				StartTimer();
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
	if (IsValidButton()) {
		_button->SetFaceColor(_bRed ? RGB(255, 0, 0) : RGB(255, 255, 255));
		_button->SetTextColor(_bRed ? RGB(0, 0, 0) : RGB(255, 0, 0));
		//_button->SetColor(gui::control::CButtonST::BTNST_COLOR_BK_OUT, _bRed ? RGB(255, 0, 0) : RGB(255, 255, 255));
		//_button->SetColor(gui::control::CButtonST::BTNST_COLOR_FG_OUT, _bRed ? RGB(0, 0, 0) : RGB(255, 0, 0));
		_bRed = !_bRed;
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
			manager->RemoteControlAlarmMachine(_machine, ademco::EVENT_ARM, 0, 0, _button);
			break;
		case ID_DDD_32773: // disarm
			manager->RemoteControlAlarmMachine(_machine, ademco::EVENT_DISARM, 0, 0, _button);
			break;
		case ID_DDD_32774: // emergency
			manager->RemoteControlAlarmMachine(_machine, ademco::EVENT_EMERGENCY, 0, 0, _button);
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
