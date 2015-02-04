// test.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include <string.h>

typedef struct Test
{
	int a;
	int b;
	static const int c = 1;
	static const int d = 2;
	Test() { memset(this, 0, sizeof(Test)); }
}Test;

typedef struct Test2
{
	Test a;
	int b;
	Test2() { memset(this, 0, sizeof(Test2)); }
}Test2;

int _tmain(int argc, _TCHAR* argv[])
{
	Test test;
	int n = sizeof(test);
	Test2 test2;
	n = sizeof(test2);
	return 0;
}

