#pragma once

namespace core { class CMapInfo; }

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

	DECLARE_MESSAGE_MAP()
private:
	core::CMapInfo* m_mapInfo;

public:
	void SetMapInfo(core::CMapInfo* mapInfo) { m_mapInfo = mapInfo; }
};

NAMESPACE_END
