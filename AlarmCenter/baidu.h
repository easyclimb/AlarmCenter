#pragma once
#include <string>

namespace web
{

typedef struct BaiduCoordinate
{
	double x;
	double y;
	BaiduCoordinate() : x(.0), y(.0) {}
	BaiduCoordinate(double x, double y) : x(x), y(y) {}
	BaiduCoordinate& operator=(const BaiduCoordinate& rhs) { x = rhs.x; y = rhs.y; return *this; }
	bool operator==(const BaiduCoordinate& rhs) { return x == rhs.x && y == rhs.y; }
}BaiduCoordinate;


};

