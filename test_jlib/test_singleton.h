#pragma once
#include <Windows.h>
//#include "C:/dev/Global/singleton.h"
#include "singleton.h"

class test : public dp::singleton<test>
{
public:

	

	~test() {
		OutputDebugStringA("test dtor\n");
	}

	void foo() {
		OutputDebugStringA("foo\n");
	}

protected:

	test() {
		OutputDebugStringA("test ctor\n");
	}
};

