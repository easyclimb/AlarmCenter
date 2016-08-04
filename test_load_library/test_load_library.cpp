// test_load_library.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include <iostream>
#include <Windows.h>

int main()
{
	auto l = LoadLibrary(L"OpenNetStream.dll");
	if (l) {
		std::cout << "ok" << std::endl;
	} else {
		auto e = GetLastError();
		std::cout << "error:" << e << std::endl;
	}
    return 0;
}

