#pragma once
#include <string>

namespace web
{
	/*namespace baidu
	{
		static const wchar_t* HOSTNAME_BAIDU = L"www.baidu.com";

		static bool get_host_by_name(const wchar_t* name, wchar_t* out, size_t out_size);

		static bool get_my_ip(wchar_t* out, size_t out_size);
	};*/

	class CBaiduService
	{
	public:
		bool locate(std::wstring& addr, int& city_code, double& x, double& y);


		~CBaiduService();
		DECLARE_UNCOPYABLE(CBaiduService)
		DECLARE_SINGLETON(CBaiduService)
	};
};

