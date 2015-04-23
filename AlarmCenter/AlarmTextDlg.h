#pragma once
#include "resource.h"

static const int ALARM_TEXT_HEIGHT = 30;
// CAlarmTextDlg 对话框

class CAlarmTextDlg : public CDialogEx
{
	DECLARE_DYNAMIC(CAlarmTextDlg)

public:
	CAlarmTextDlg(CWnd* pParent = NULL);   // 标准构造函数
	virtual ~CAlarmTextDlg();

// 对话框数据
	enum { IDD = IDD_DIALOG_ALARM_TEXT };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 支持

	DECLARE_MESSAGE_MAP()
public:
	virtual BOOL OnInitDialog();
	afx_msg void OnBnClickedOk();
	afx_msg BOOL OnEraseBkgnd(CDC* pDC);
	afx_msg void OnPaint();
	afx_msg void OnTimer(UINT_PTR nIDEvent);
	afx_msg HBRUSH CtlColor(CDC* /*pDC*/, UINT /*nCtlColor*/);
private:
	int m_curPos;
	int m_extra;
	CString m_text;
	CFont m_font;
	BOOL m_bDrawText;
	CBitmap m_bitmap;
	CDC m_memDC;
	COLORREF m_clr;
	BOOL m_bAlreadyAddBlank;
public:

	inline void SetText(const CString& text)
	{
		m_text = text;
	}

	inline void Show()
	{
		ShowWindow(SW_SHOW);
		//CRect Rect;
		//GetWindowRect(&Rect);
		//GetParent()->ScreenToClient(&Rect);
		//GetParent()->InvalidateRect(&Rect);
		GetParent()->UpdateWindow();
		//KillTimer(1);
		//SetTimer(1, 500, NULL);
	}

	inline void Hide()
	{
		KillTimer(1);
		ShowWindow(SW_HIDE);
	}

	inline void SetColor(COLORREF color)
	{
		//KillTimer(1);
		if (color != m_clr) {
			m_clr = color;
			//CRect Rect;
			//GetWindowRect(&Rect);
			//GetParent()->ScreenToClient(&Rect);
			//GetParent()->InvalidateRect(&Rect);
			GetParent()->UpdateWindow();
			//UpdateWindow();
			//Invalidate();
		}
	}

	afx_msg void OnDestroy();
	void DrawText(CDC* pDC);
	afx_msg HBRUSH OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor);
};
