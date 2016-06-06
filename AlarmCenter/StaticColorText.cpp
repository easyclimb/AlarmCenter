// StaticColorText.cpp : implementation file
//

#include "stdafx.h"
//#include "AlarmCenter.h"
#include "StaticColorText.h"

namespace gui {
namespace control {

// CStaticColorText

IMPLEMENT_DYNAMIC(CStaticColorText, CStatic)

CStaticColorText::CStaticColorText()
{

}

CStaticColorText::~CStaticColorText()
{
}


BEGIN_MESSAGE_MAP(CStaticColorText, CStatic)
	ON_WM_CTLCOLOR_REFLECT()
END_MESSAGE_MAP()



// CStaticColorText message handlers




HBRUSH CStaticColorText::CtlColor(CDC* pDC, UINT /*nCtlColor*/)
{
	pDC->SetTextColor(text_color_);
	pDC->SetBkColor(face_color_);
	return (HBRUSH)GetStockObject(NULL_BRUSH);
}

}
}
