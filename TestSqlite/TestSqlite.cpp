// TestSqlite.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include <stdlib.h>
#include <iostream>

#include "sqlitecpp/SQLiteCpp.h"

void test_raw_sqlite3()
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
}


void test_sqlite_cpp()
{
	try {
		using namespace std;
		using namespace SQLite;

		Database db("test.db3", SQLITE_OPEN_READWRITE | SQLITE_OPEN_CREATE);
		db.exec("drop table if exists test");
		db.exec("create table test (id integer primary key, value text)");

		int nb = db.exec("insert into test values(NULL, \"test\")");
		cout << "insert return " << nb << endl;

		nb = db.exec("insert into test values(NULL, \"second\")");
		cout << "insert return " << nb << endl;

		nb = db.exec("update test set value=\"second-update\" where id='2'");
		cout << "update return " << nb << endl;

		Statement query(db, "select * from test");
		while (query.executeStep()) {
			cout << "row (" << query.getColumn(0) << ", \"" << query.getColumn(1) << "\")" << endl;
		}

		


	} catch (std::exception& e) {
		std::cout << e.what() << std::endl;
	}

	remove("aa");
}


int main()
{
	//test_raw_sqlite3();
	test_sqlite_cpp();

	system("pause");
    return 0;
}

