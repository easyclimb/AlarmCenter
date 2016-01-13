// TestSqlite.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include <stdlib.h>
#include "C:/dev_libs/sqlite/sqlite-amalgamation-3100000/sqlite/sqlite3.h"
#pragma comment(lib, "C:\\dev_libs\\sqlite\\sqlite-dll-win32-x86-3100000\\sqlite3.lib")

int main()
{
	sqlite3* db = nullptr;
	char* msg = nullptr;
	int rc = 0;

	rc = sqlite3_open("test.db", &db);
	if (rc) {
		fprintf(stderr, "Can't open database: %s\n", sqlite3_errmsg(db));
		exit(0);
	} else {
		fprintf(stdout, "Open database successfully!\n");
	}

	system("pause");
    return 0;
}

