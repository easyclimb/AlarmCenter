#pragma once

namespace Imagin { class CTimer; };

namespace gui {
class CButtonEx
{
	static const int FLASH_GAP = 1000;
	DECLARE_UNCOPYABLE(CButtonEx)
private:
	CMFCButton* _button;
	DWORD _data;
	core::MachineStatus _status;
	BOOL _bRed;
	Imagin::CTimer* _timer;
	CButtonEx() {}
public:
	CButtonEx(const wchar_t* text,
			  const RECT& rc,
			  CWnd* parent,
			  UINT id,
			  DWORD data);

	~CButtonEx();

	void ShowWindow(int nCmdShow);
	void SetStatus(core::MachineStatus status);
	void OnTimer();
	static bool IsStandardStatus(core::MachineStatus status);
protected:
	void UpdateStatus();
};

NAMESPACE_END

