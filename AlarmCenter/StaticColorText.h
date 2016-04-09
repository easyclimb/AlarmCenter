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
		if (clr != text_color_) {
			text_color_ = clr;
			Invalidate();
		}
	}

	void SetFaceColor(COLORREF clr) {
		if (clr != face_color_) {
			face_color_ = clr;
			Invalidate();
		}
	}

protected:
	DECLARE_MESSAGE_MAP()
	COLORREF text_color_ = RGB(255, 255, 255);
	COLORREF face_color_ = RGB(0, 0, 0);
public:
	afx_msg HBRUSH CtlColor(CDC* pDC, UINT nCtlColor);
};


}
}

