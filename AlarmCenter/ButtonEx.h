#pragma once

namespace imagin { class CTimer; };

namespace gui {

namespace control {
	class CButtonST;
	class CMFCButtonEx;
};


class CButtonEx
{
	static const int FLASH_GAP = 1000;
	DECLARE_UNCOPYABLE(CButtonEx)
private:
	//control::CButtonST* _button;
	control::CMFCButtonEx* _button;
	CWnd* _wndParent;
	DWORD _data;
	core::MachineStatus _status;
	BOOL _bRed;
	imagin::CTimer* _timer;
	CButtonEx() {}
public:
	CButtonEx(const wchar_t* text,
			  const RECT& rc,
			  CWnd* parent,
			  UINT id,
			  DWORD data);

	~CButtonEx();
	void OnBnClicked();
	void OnRBnClicked();
	void ShowWindow(int nCmdShow);
	void OnStatusChange(core::MachineStatus status);
	void OnTimer();
	static bool IsStandardStatus(core::MachineStatus status);
protected:
	void UpdateStatus();
};

NAMESPACE_END

