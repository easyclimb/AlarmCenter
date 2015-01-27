#include "stdafx.h"
#include "ButtonEx.h"
#include "MFCButtonEx.h"
#include "AlarmMachine.h"
#include "./imagin/Timer.h"
#include "AlarmMachineContainer.h"


namespace gui {


static void __stdcall on_timer(Imagin::CTimer* /*timer*/, void* udata)
{
	CButtonEx* btn = reinterpret_cast<CButtonEx*>(udata); ASSERT(btn);
	btn->OnTimer();
}

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
					 DWORD data)
	: _button(NULL)
	, _wndParent(parent)
	, _data(data)
	, _status(core::MS_OFFLINE)
	, _bRed(FALSE),
	_timer(NULL)
{
	_button = new CMFCButtonEx();
	_button->Create(text, WS_CHILD | WS_VISIBLE | BS_ICON, rc, parent, id);
	ASSERT(IsWindow(_button->m_hWnd));
	_button->SetButtonClkCallback(on_btnclick, this);
	_timer = new Imagin::CTimer(on_timer, this);
}


CButtonEx::~CButtonEx()
{
	_button->DestroyWindow();
	delete _button;
	delete _timer;
}


void CButtonEx::ShowWindow(int nCmdShow)
{
	if (_button && IsWindow(_button->m_hWnd)) {
		UpdateStatus();
		_button->ShowWindow(nCmdShow);
	}
}


bool CButtonEx::IsStandardStatus(core::MachineStatus status)
{
	return status == core::MS_OFFLINE
		|| status == core::MS_ONLINE
		|| status == core::MS_ARM
		|| status == core::MS_DISARM
		|| status == core::MS_HALFARM;
}


void CButtonEx::OnStatusChange(core::MachineStatus status)
{
	if (_status != status) {
		//if (IsStandardStatus(status)) {
			_status = status;
		//}

		if (_button && IsWindow(_button->m_hWnd)) {
			UpdateStatus();
		}
	}
}


void CButtonEx::OnTimer()
{
	if (_button && IsWindow(_button->m_hWnd)) {
		_button->SetFaceColor(_bRed ? RGB(255, 0, 0) : RGB(255, 255, 255));
		_button->SetTextColor(_bRed ? RGB(0, 0, 0) : RGB(255, 0, 0));
		_bRed = !_bRed;
		_button->Invalidate();
		_timer->Start(FLASH_GAP, true);
		CLog::WriteLog(L"OnTimer");
	}
}


void CButtonEx::UpdateStatus()
{
	switch (_status) {
		case core::MS_OFFLINE:
			_button->SetTextColor(RGB(255, 0, 0));
			_button->SetIcon(CAlarmMachineContainerDlg::m_hIconNetFailed);
			break;
		case core::MS_ONLINE:
			_button->SetTextColor(RGB(0, 0, 0));
			_button->SetIcon(CAlarmMachineContainerDlg::m_hIconNetOk);
			break;
		case core::MS_DISARM:
			_button->SetTextColor(RGB(0, 0, 0));
			_button->SetIcon(CAlarmMachineContainerDlg::m_hIconDisarm);
			break;
		case core::MS_ARM:
			_button->SetTextColor(RGB(0, 0, 0));
			_button->SetIcon(CAlarmMachineContainerDlg::m_hIconArm);
			break;
		default:	// means its alarming
			_button->SetTextColor(RGB(0, 0, 0));
			_button->SetFaceColor(RGB(255, 0, 0));
			//_button->SetIcon(CAlarmMachineContainerDlg::m_hIconNetFailed);
			_timer->Stop();
			_timer->Start(FLASH_GAP, true);
			break;
	}
	_button->Invalidate();
}


void CButtonEx::OnBnClicked()
{
	if (_wndParent && IsWindow(_wndParent->GetSafeHwnd())) {
		_wndParent->PostMessage(WM_BNCLKEDEX, 0, _data);
	}
}


void CButtonEx::OnRBnClicked()
{
	if (_wndParent && IsWindow(_wndParent->GetSafeHwnd())) {
		_wndParent->PostMessage(WM_BNCLKEDEX, 1, _data);
	}
}

NAMESPACE_END
