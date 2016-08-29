#pragma once

// CVideoPlayerCtrl

class CVideoPlayerCtrl : public CStatic
{
	DECLARE_DYNAMIC(CVideoPlayerCtrl)

public:
	CVideoPlayerCtrl();
	virtual ~CVideoPlayerCtrl();
	HWND GetRealHwnd() const;
	CRect GetRealRect() const;

	void SetFocused(BOOL b = TRUE) {
		if (focused_ != b) {
			focused_ = b;
			Invalidate();
		}
	}

	void set_index(int i) { ndx_ = i; }
	int get_index() const { return ndx_; }

private:
	int ndx_ = 0;
	BOOL m_bPlaying;
	BOOL focused_ = FALSE;
	//BOOL m_bMaximized;
	//CRect m_rcNormal;
	//CRect m_rcMonitor;
	CBrush brush_;
	CStatic* real_player = nullptr;

protected:
	DECLARE_MESSAGE_MAP()
public:
	//BOOL GetMaximized() const { return m_bMaximized; }
	//void SetMaximized(BOOL b);
	//void SetMonitorRect(const CRect& rc) { m_rcMonitor = rc; }
	
	afx_msg void OnRButtonUp(UINT nFlags, CPoint point);
	afx_msg void OnLButtonDblClk(UINT nFlags, CPoint point);
	afx_msg void OnLButtonDown(UINT nFlags, CPoint point);
	afx_msg void OnShowWindow(BOOL bShow, UINT nStatus);
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg void OnPaint();
	afx_msg void OnDestroy();
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg void OnMove(int x, int y);
};


