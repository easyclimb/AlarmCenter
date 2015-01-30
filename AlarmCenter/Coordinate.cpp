// Coordinate.cpp: implementation of the CCoordinate class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "Coordinate.h"
#include <math.h>

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

namespace gui {
namespace control {

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CCoordinate::CCoordinate()
{

}

CCoordinate::~CCoordinate()
{

}

double CCoordinate::AngleToRadian(double angle)
{
	return (angle * 3.141592653) / 180.0;
}

CPoint CCoordinate::GetRotatedPoint(const CPoint& ptOrg, int distance, int angle)
{
	CPoint ptRtd;
	double sina = sin(AngleToRadian(angle));
	double cosa = cos(AngleToRadian(angle)); 
	double xx = distance * cosa;
	double yy = distance * sina;
	ptRtd.x = ptOrg.x + static_cast<int>(xx);
	ptRtd.y = ptOrg.y + static_cast<int>(yy);
	return ptRtd;
}

NAMESPACE_END
NAMESPACE_END
