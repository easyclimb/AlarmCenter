#include "stdafx.h"
#include "CppUnitTest.h"

#include <afx.h>
#include "../AlarmCenter/ademco_func.h"
//#include "C:/dev/Global/global.h"



using namespace Microsoft::VisualStudio::CppUnitTestFramework;
//using namespace jlib;

//IMPLEMENT_CLASS_LOG_STATIC_MEMBER;

namespace UnitTest1
{		
	TEST_CLASS(UnitTest1)
	{
	public:
		
		TEST_METHOD(TestMethod1)
		{
			ademco::AdemcoPacket ap;
			size_t cbCmtd;
			Assert::IsTrue(ademco::RESULT_NOT_ENOUGH == ap.Parse("abc", 3, cbCmtd));
		}

	};
}