#if !defined(AFX_DETECTOR_H__72A2BC56_8C30_4786_99E4_F28B3C9B96A9__INCLUDED_)
#define AFX_DETECTOR_H__72A2BC56_8C30_4786_99E4_F28B3C9B96A9__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// Detector.h : header file
//
#include <list>
/////////////////////////////////////////////////////////////////////////////
// CDetector window
#include "core.h"

//namespace gui {
class CDetector : public CButton, public std::enable_shared_from_this<CDetector>
{
// Construction
public:
	CDetector(core::CDetectorBindInterfacePtr pInterface,
			  core::CDetectorInfoPtr detectorInfo, 
			  BOOL bMainDetector = TRUE);
	BOOL CreateDetector(CWnd* parentWnd);
	//void FormatAlarmText(CString& alarmText, int ademco_event);
	static void __stdcall OnInversionControlZone(core::CDetectorPtr detector,
												 core::InversionControlZoneCommand iczc,
												 DWORD dwExtra);
// Operations
public:
	BOOL m_bNeedRecalcPts;
	CDetector *m_pPairDetector;
// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CDetector)
	public:
	virtual void DrawItem(LPDRAWITEMSTRUCT lpDrawItemStruct);
	virtual BOOL PreTranslateMessage(MSG* pMsg);
	protected:
	virtual void PreSubclassWindow();
	//}}AFX_VIRTUAL

// Implementation
public:
	int GetPtn() const;
	void SetTooltipText(LPCTSTR lpszText, BOOL bActivate = TRUE);
	void Alarm(BOOL bAlarm);
	void SetFocus(BOOL bFocus);
	void GetPts(CPoint*& pts);
	void Rotate(int angle);
	virtual ~CDetector();
	BOOL IsAlarming() const { return m_bAlarming; }
	core::CDetectorBindInterfacePtr GetInterfaceInfo() { return m_interface; }
	//int GetZoneID() const;
	// Generated message map functions
protected:
	void ReleasePts();
	void GenerateAntlinePts();
	void InitToolTip();
	void ShowToolTip();
	DECLARE_MESSAGE_MAP()
	static HRGN BitmapToRegion(HBITMAP hBmp, COLORREF cTransparentColor, COLORREF cTolerance = RGB(0,0,0));
	void OnRotate();
	void OnDistance();
	void OnMoveWithDirection();
	void SetAlarmingColor(COLORREF clr);
	void OnClick(); 
	void OnRClick();
private:
	CPoint m_pt;
	HRGN m_hRgn;//, m_hRgnRotated;
	HBITMAP m_hBitmap;
	HBITMAP m_hBitmapRotated;
	core::CDetectorBindInterfacePtr m_interface;
	core::CDetectorInfoPtr m_detectorInfo;
	core::CDetectorLibDataPtr m_detectorLibData;
	BOOL m_bFocused;
	BOOL m_bManualRotate;
	BOOL m_bAlarming;
	BOOL m_bCurColorRed;
	CToolTipCtrl m_ToolTip;
	CRITICAL_SECTION m_cs;
	HBRUSH m_hBrushFocused;
	HBRUSH m_hBrushAlarmed;
	CSize m_sizeBmp;
	BOOL m_bAntlineGenerated;
	CPoint* m_pts;
	BOOL m_bMainDetector;
	BOOL m_bMouseIn;
	BOOL m_bRbtnDown;
	std::list<void*> m_iczcList;
	CLock m_iczcLock;
public:
	void AddIczc(void* iczc){
		AUTO_LOG_FUNCTION;
		m_iczcLock.Lock();
		m_iczcList.push_back(iczc);
		m_iczcLock.UnLock();
	}
public:
	afx_msg void OnTimer(UINT nIDEvent);
	afx_msg void OnDestroy();
	afx_msg void OnMouseMove(UINT nFlags, CPoint point);
	afx_msg void OnMouseLeave();
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg void OnShowWindow(BOOL bShow, UINT nStatus);
	afx_msg void OnBnClicked();
	afx_msg void OnBnDoubleclicked();
	afx_msg LRESULT OnInversionControlResult(WPARAM wParam, LPARAM lParam);
	afx_msg void OnRButtonDown(UINT nFlags, CPoint point);
	afx_msg void OnRButtonUp(UINT nFlags, CPoint point);
};

//NAMESPACE_END

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_DETECTOR_H__72A2BC56_8C30_4786_99E4_F28B3C9B96A9__INCLUDED_)
