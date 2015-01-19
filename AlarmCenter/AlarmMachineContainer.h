#pragma once

#include <list>
#include <afxbutton.h>
// CAlarmMachineContainerDlg dialog

namespace gui { class CScrollHelper; };
namespace core { 
	class CAlarmMachine; 
	//typedef std::list<CAlarmMachine*> CAlarmMachineList;
	//typedef std::list<CAlarmMachine*>::iterator CAlarmMachineListIter;
};

class CAlarmMachineContainerDlg : public CDialogEx
{
	DECLARE_DYNAMIC(CAlarmMachineContainerDlg)

	typedef struct ButtonEx
	{
		CMFCButton* _button;
		DWORD _data;

		ButtonEx(const wchar_t* text, 
				 const RECT& rc, 
				 CWnd* parent, 
				 UINT id,
				 DWORD data) : _button(NULL), _data(data)
		{
			_button = new CMFCButton();
			_button->Create(text, WS_CHILD | WS_VISIBLE | BS_ICON, rc, parent, id);
			ASSERT(IsWindow(_button->m_hWnd));
		}

		~ButtonEx()	{
			_button->DestroyWindow();
			delete _button;
		}
	}ButtonEx, *PButtonEx;

public:
	CAlarmMachineContainerDlg(CWnd* pParent = NULL);   // standard constructor
	virtual ~CAlarmMachineContainerDlg();

// Dialog Data
	enum { IDD = IDD_DIALOG_CONTAINER };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	DECLARE_MESSAGE_MAP()
private:
	gui::CScrollHelper* m_scrollHelper;
	//core::CAlarmMachineList m_machineList;
	std::list<PButtonEx> m_machineList;
public:
	afx_msg void OnHScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar);
	afx_msg void OnVScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar);
	afx_msg BOOL OnMouseWheel(UINT nFlags, short zDelta, CPoint pt);
	afx_msg void OnSize(UINT nType, int cx, int cy);
	BOOL InsertMachine(core::CAlarmMachine* machine);
	virtual BOOL OnInitDialog();
	afx_msg void OnDestroy();
};
