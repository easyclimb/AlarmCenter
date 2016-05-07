#pragma once
#include "video.h"

// CVideoPlayerCtrl

class CVideoPlayerCtrl : public CStatic
{
	DECLARE_DYNAMIC(CVideoPlayerCtrl)

public:
	CVideoPlayerCtrl();
	virtual ~CVideoPlayerCtrl();
private:
	video::CVideoDeviceInfoPtr m_device;
	BOOL m_bPlaying;
	BOOL m_bMaximized;
	//CRect m_rcNormal;
	//CRect m_rcMonitor;
protected:
	DECLARE_MESSAGE_MAP()
public:
	BOOL GetMaximized() const { return m_bMaximized; }
	void SetMaximized(BOOL b);
	//void SetMonitorRect(const CRect& rc) { m_rcMonitor = rc; }
	int ndx_ = 0;
	afx_msg void OnRButtonUp(UINT nFlags, CPoint point);
	afx_msg void OnLButtonDblClk(UINT nFlags, CPoint point);
	afx_msg void OnLButtonDown(UINT nFlags, CPoint point);
};


