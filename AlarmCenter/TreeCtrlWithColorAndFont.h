#pragma once
#include <map>

// CTreeCtrlWithColorAndFont

class CTreeCtrlWithColorAndFont : public CTreeCtrl
{
	DECLARE_DYNAMIC(CTreeCtrlWithColorAndFont)
	typedef struct ColorAndFont {
		COLORREF color;
		LOGFONT font;
		ColorAndFont& operator=(const ColorAndFont& rhs) {
			color = rhs.color;
			memcpy(&font, &rhs.font, sizeof(LOGFONT));
			return *this;
		}
	}ColorAndFont;
	typedef std::shared_ptr<ColorAndFont> ColorAndFontPtr;
	std::map<HTREEITEM, ColorAndFontPtr> m_itemMap;
public:
	CTreeCtrlWithColorAndFont();
	virtual ~CTreeCtrlWithColorAndFont();

	void SetItemFont(HTREEITEM hItem, LOGFONT& font) {
		ColorAndFontPtr cf;
		auto iter = m_itemMap.find(hItem);
		if (iter == m_itemMap.end()) {
			cf = std::make_shared<ColorAndFont>();
			cf->color = COLORREF(-1);
		} else {
			cf = iter->second;
		}
		cf->font = font;
		m_itemMap[hItem] = cf;
	}

	void SetItemBold(HTREEITEM hItem, bool bBold) {
		SetItemState(hItem, bBold ? TVIS_BOLD : 0, TVIS_BOLD);
	}

	void SetItemColor(HTREEITEM hItem, COLORREF color) {
		ColorAndFontPtr cf;
		auto iter = m_itemMap.find(hItem);
		if (iter == m_itemMap.end()) {
			cf = std::make_shared<ColorAndFont>();
			cf->font.lfFaceName[0] = 0;
		} else {
			cf = iter->second;
		}
		cf->color = color;
		m_itemMap[hItem] = cf;
	}

	BOOL GetItemBold(HTREEITEM hItem)
	{
		return GetItemState(hItem, TVIS_BOLD) & TVIS_BOLD;
	}

protected:
	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnPaint();
};


