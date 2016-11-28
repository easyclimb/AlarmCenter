// ColorEdit.cpp : implementation file
//

#include "stdafx.h"
#include "AlarmCenter.h"
#include "ColorEdit.h"


// CColorEdit

IMPLEMENT_DYNAMIC(CColorEdit, CEdit)

CColorEdit::CColorEdit()
{

}

CColorEdit::~CColorEdit()
{
	face_brush_.DeleteObject();
}

void CColorEdit::set_face_color(COLORREF clr)
{
	face_color_ = clr;
	face_brush_.DeleteObject();
	face_brush_.CreateSolidBrush(face_color_);
	Invalidate();
}

void CColorEdit::set_text_color(COLORREF clr)
{
	text_color_ = clr;	
	Invalidate();
}


BEGIN_MESSAGE_MAP(CColorEdit, CEdit)
	ON_WM_CTLCOLOR_REFLECT()
END_MESSAGE_MAP()



// CColorEdit message handlers




HBRUSH CColorEdit::CtlColor(CDC* pDC, UINT /*nCtlColor*/)
{
	pDC->SetTextColor(text_color_);

	return face_brush_;
}


void CColorEdit::PreSubclassWindow()
{
	face_brush_.CreateSolidBrush(face_color_);

	CEdit::PreSubclassWindow();
}
