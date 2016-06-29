/**
 * @file    SQLiteCpp.h
 * @ingroup SQLiteCpp
 * @brief   SQLiteC++ is a smart and simple C++ SQLite3 wrapper. This file is only "easy include" for other files.
 *
 * Include this main header file in your project to gain access to all functionality provided by the wrapper.
 *
 * Copyright (c) 2012-2016 Sebastien Rombauts (sebastien.rombauts@gmail.com)
 *
 * Distributed under the MIT License (MIT) (See accompanying file LICENSE.txt
 * or copy at http://opensource.org/licenses/MIT)
 */
/**
 * @defgroup SQLiteCpp SQLiteC++
 * @brief    SQLiteC++ is a smart and simple C++ SQLite3 wrapper. This file is only "easy include" for other files.
 */
#pragma once

#include "sqlite3.h"

// Include useful headers of SQLiteC++
#include "Assertion.h"
#include "Exception.h"
#include "Database.h"
#include "Statement.h"
#include "Column.h"
#include "Transaction.h"


/**
 * @brief Version numbers for SQLiteC++ are provided in the same way as sqlite3.h
 *
 * The [SQLITECPP_VERSION] C preprocessor macro in the SQLiteC++.h header
 * evaluates to a string literal that is the SQLite version in the
 * format "X.Y.Z" where X is the major version number
 * and Y is the minor version number and Z is the release number.
 *
 * The [SQLITECPP_VERSION_NUMBER] C preprocessor macro resolves to an integer
 * with the value (X*1000000 + Y*1000 + Z) where X, Y, and Z are the same
 * numbers used in [SQLITECPP_VERSION].
 */
#define SQLITECPP_VERSION           "1.3.1"
#define SQLITECPP_VERSION_NUMBER    1003001

namespace SQLite {

inline std::wstring double_quotes(const std::wstring& str) {
	auto s = str;
	auto pos = s.find_first_of(L'\'');
	while (pos != std::wstring::npos) {
		s.replace(pos, 1, L"''");
		pos += 2;
		pos = s.find_first_of(L'\'', pos);
	}
	return s;
}

inline std::wstring double_quotes(LPCTSTR str) {
	return double_quotes(std::wstring(str));
}

}
