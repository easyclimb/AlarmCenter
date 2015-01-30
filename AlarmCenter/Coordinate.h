// Coordinate.h: interface for the CCoordinate class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_COORDINATE_H__05A1BDAC_1B22_41C8_A8B8_92CD0F9EDE01__INCLUDED_)
#define AFX_COORDINATE_H__05A1BDAC_1B22_41C8_A8B8_92CD0F9EDE01__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

namespace gui {
namespace control {

class CCoordinate  
{
public:
	static double AngleToRadian(double angle);
	static CPoint GetRotatedPoint(const CPoint& ptOrg, int distance, int angle);
private:
	CCoordinate();
	virtual ~CCoordinate();
	
};

NAMESPACE_END
NAMESPACE_END

#endif // !defined(AFX_COORDINATE_H__05A1BDAC_1B22_41C8_A8B8_92CD0F9EDE01__INCLUDED_)
