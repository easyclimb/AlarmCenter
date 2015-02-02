#if !defined(AFX_DETECTOR_H__72A2BC56_8C30_4786_99E4_F28B3C9B96A9__INCLUDED_)
#define AFX_DETECTOR_H__72A2BC56_8C30_4786_99E4_F28B3C9B96A9__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// Detector.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CDetector window
namespace core { class CZoneInfo; class CDetectorInfo; class CDetectorLibData; };

//namespace gui {
class CDetector : public CButton
{
	static const int ALARM_FLICK_GAP = 1500;
	static const UINT m_TimerIDRepaint = 1;
	static const UINT m_TimerIDAlarm = 2;
// Construction
public:
	CDetector(core::CZoneInfo* zoneInfo, CWnd* parentWnd, BOOL bMainDetector = TRUE);
	BOOL CreateDetector();
// Attributes
public:

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

	// Generated message map functions
protected:
	void ReleasePts();
	void GenerateAntlinePts();
	void InitToolTip();
	//{{AFX_MSG(CDetector)
	afx_msg void OnLButtonDown(UINT nFlags, CPoint point);
	afx_msg void OnTimer(UINT nIDEvent);
	afx_msg void OnDestroy();
	afx_msg void OnMouseMove(UINT nFlags, CPoint point);
	afx_msg void OnMouseLeave();
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg void OnShowWindow(BOOL bShow, UINT nStatus);
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
	static HRGN BitmapToRegion(HBITMAP hBmp, COLORREF cTransparentColor, COLORREF cTolerance = RGB(0,0,0));
private:
	CPoint m_pt;
	HRGN m_hRgn;//, m_hRgnRotated;
	HBITMAP m_hBitmap;
	HBITMAP m_hBitmapRotated;
	core::CZoneInfo* m_zoneInfo;
	core::CDetectorInfo* m_detectorInfo;
	core::CDetectorLibData* m_detectorLibData;
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
	CWnd* m_parentWnd;
	BOOL m_bMainDetector;
	BOOL m_bMouseIn;
};

//NAMESPACE_END

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_DETECTOR_H__72A2BC56_8C30_4786_99E4_F28B3C9B96A9__INCLUDED_)
