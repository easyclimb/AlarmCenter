#pragma once

namespace gui {
namespace control {


// CStaticColorText

class CStaticColorText : public CStatic
{
	DECLARE_DYNAMIC(CStaticColorText)

public:
	CStaticColorText();
	virtual ~CStaticColorText();
	void SetTextColor(COLORREF clr) {
		if (clr != color_) {
			color_ = clr;
			Invalidate();
		}
	}

protected:
	DECLARE_MESSAGE_MAP()
	COLORREF color_ = RGB(255, 255, 255);
public:
	afx_msg HBRUSH OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor);
};


}
}

