#pragma once

namespace core { class CMapInfo; };

namespace gui {

class CMapView : public CDialogEx
{
	DECLARE_DYNAMIC(CMapView)

public:
	CMapView(CWnd* pParent = NULL);   // standard constructor
	virtual ~CMapView();

// Dialog Data
	enum { IDD = IDD_DIALOG_MAPVIEW };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	afx_msg void OnPaint();
	afx_msg void OnDestroy();
	DECLARE_MESSAGE_MAP()
private:
	const core::CMapInfo* m_mapInfo;
	HBITMAP m_hBmpOrigin;
	int m_bmWidth;
	int m_bmHeight;
public:
	void SetMapInfo(const core::CMapInfo* mapInfo) { m_mapInfo = mapInfo; }
	virtual BOOL OnInitDialog();

protected:
	BOOL ImportBmp();
};

NAMESPACE_END
