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


	/*namespace baidu
	{
		static const wchar_t* HOSTNAME_BAIDU = L"www.baidu.com";

		static bool get_host_by_name(const wchar_t* name, wchar_t* out, size_t out_size);

		static bool get_my_ip(wchar_t* out, size_t out_size);
	};*/

	class CBaiduService
	{
	public:
		bool locate(std::wstring& addr, int& city_code, BaiduCoordinate& coor);

		~CBaiduService();
		DECLARE_UNCOPYABLE(CBaiduService)
		DECLARE_SINGLETON(CBaiduService)
	};
};

