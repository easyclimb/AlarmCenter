#pragma once


// CColorEdit

class CColorEdit : public CEdit
{
	DECLARE_DYNAMIC(CColorEdit)

public:
	CColorEdit();
	virtual ~CColorEdit();

	void set_face_color(COLORREF clr);
	void set_text_color(COLORREF clr);

protected:
	DECLARE_MESSAGE_MAP()

private:
	COLORREF face_color_ = RGB(255, 255, 255);
	COLORREF text_color_ = RGB(0, 0, 0);
	CBrush face_brush_ = {};

public:
	afx_msg HBRUSH CtlColor(CDC* /*pDC*/, UINT /*nCtlColor*/);
	virtual void PreSubclassWindow();
};


