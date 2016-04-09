// StaticColorText.cpp : implementation file
//

#include "stdafx.h"
#include "AlarmCenter.h"
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
	ON_WM_CTLCOLOR()
END_MESSAGE_MAP()



// CStaticColorText message handlers




HBRUSH CStaticColorText::OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor)
{
	HBRUSH hbr = CStatic::OnCtlColor(pDC, pWnd, nCtlColor);
	pDC->SetTextColor(color_);
	return hbr;
}

}
}
