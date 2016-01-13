// TestMysql.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include <string>
#include <iostream>

#define MYSQL_CPP_CONN_ROOT "C:\\dev_libs\\mysql\\mysql-connector-c++-1.1.6\\mysql-connector-c++-1.1.6"

#pragma comment(lib, MYSQL_CPP_CONN_ROOT "\\driver\\Debug\\mysqlcppconn.lib")
//#pragma comment(lib, "C:\\dev_libs\\mysql\\connector\\lib\\opt\\mysqlcppconn-static.lib")
//#include <mysql_connection.h>
//#include <mysql_driver.h>
//#include <cppconn/resultset.h>
//#include <cppconn/statement.h>
#include <iostream>
#include <sstream>
#include <memory>
#include <string>
#include <stdexcept>

#include "C:\\dev_libs\\mysql\\mysql-connector-c++-1.1.6\\mysql-connector-c++-1.1.6\\cppconn\\mysql_connector_cpp.h"


/* MySQL Connector/C++ specific headers */
//#include <cppconn/driver.h>
//#include <cppconn/connection.h>
//#include <cppconn/statement.h>
//#include <cppconn/prepared_statement.h>
//#include <cppconn/resultset.h>
//#include <cppconn/metadata.h>
//#include <cppconn/resultset_metadata.h>
//#include <cppconn/exception.h>
//#include <cppconn/warning.h>

#define DBHOST "tcp://192.168.168.168:3306"
#define USER "root"
#define PASSWORD "888101"
#define DATABASE "world"

#define NUMOFFSET 100
#define COLNAME 200

using namespace std;
using namespace sql;

static void retrieve_data_and_print(ResultSet *rs, int type, int colidx, string colname) {

	/* retrieve the row count in the result set */
	cout << "\nRetrieved " << rs->rowsCount() << " row(s)." << endl;

	cout << "\nCityName" << endl;
	cout << "--------" << endl;

	/* fetch the data : retrieve all the rows in the result set */
	while (rs->next()) {
		if (type == NUMOFFSET) {
			cout << rs->getString(colidx).c_str() << endl;
		} else if (type == COLNAME) {
			cout << rs->getString(colname).c_str() << endl;
		} // if-else
	} // while

	cout << endl;

} // retrieve_data_and_print()

static void retrieve_dbmetadata_and_print(Connection *dbcon) {

	if (dbcon->isClosed()) {
		throw runtime_error("DatabaseMetaData FAILURE - database connection closed");
	}

	cout << "\nDatabase Metadata" << endl;
	cout << "-----------------" << endl;

	cout << boolalpha;

	/* The following commented statement won't work with Connector/C++ 1.0.5 and later */
	//auto_ptr < DatabaseMetaData > dbcon_meta (dbcon -> getMetaData());

	DatabaseMetaData *dbcon_meta = dbcon->getMetaData();

	cout << "Database Product Name: " << dbcon_meta->getDatabaseProductName() << endl;
	//auto str = dbcon_meta->getDatabaseProductVersion().c_str();
	//cout << "Database Product Version: " << dbcon_meta->getDatabaseProductVersion() << endl;
	cout << "Database User Name: " << dbcon_meta->getUserName() << endl << endl;

	cout << "Driver name: " << dbcon_meta->getDriverName() << endl;
	cout << "Driver version: " << dbcon_meta->getDriverVersion() << endl << endl;

	cout << "Database in Read-Only Mode?: " << dbcon_meta->isReadOnly() << endl;
	cout << "Supports Transactions?: " << dbcon_meta->supportsTransactions() << endl;
	cout << "Supports DML Transactions only?: " << dbcon_meta->supportsDataManipulationTransactionsOnly() << endl;
	cout << "Supports Batch Updates?: " << dbcon_meta->supportsBatchUpdates() << endl;
	cout << "Supports Outer Joins?: " << dbcon_meta->supportsOuterJoins() << endl;
	cout << "Supports Multiple Transactions?: " << dbcon_meta->supportsMultipleTransactions() << endl;
	cout << "Supports Named Parameters?: " << dbcon_meta->supportsNamedParameters() << endl;
	cout << "Supports Statement Pooling?: " << dbcon_meta->supportsStatementPooling() << endl;
	cout << "Supports Stored Procedures?: " << dbcon_meta->supportsStoredProcedures() << endl;
	cout << "Supports Union?: " << dbcon_meta->supportsUnion() << endl << endl;

	cout << "Maximum Connections: " << dbcon_meta->getMaxConnections() << endl;
	cout << "Maximum Columns per Table: " << dbcon_meta->getMaxColumnsInTable() << endl;
	cout << "Maximum Columns per Index: " << dbcon_meta->getMaxColumnsInIndex() << endl;
	cout << "Maximum Row Size per Table: " << dbcon_meta->getMaxRowSize() << " bytes" << endl;

	cout << "\nDatabase schemas: " << endl;

	auto_ptr < ResultSet > rs(dbcon_meta->getSchemas());

	cout << "\nTotal number of schemas = " << rs->rowsCount() << endl;
	cout << endl;

	int row = 1;

	while (rs->next()) {
		cout << "\t" << row << ". " << rs->getString("TABLE_SCHEM") << endl;
		++row;
	} // while

	cout << endl << endl;

} // retrieve_dbmetadata_and_print()

static void retrieve_rsmetadata_and_print(ResultSet *rs) {

	if (rs->rowsCount() == 0) {
		throw runtime_error("ResultSetMetaData FAILURE - no records in the result set");
	}

	cout << "ResultSet Metadata" << endl;
	cout << "------------------" << endl;

	/* The following commented statement won't work with Connector/C++ 1.0.5 and later */
	//auto_ptr < ResultSetMetaData > res_meta ( rs -> getMetaData() );

	ResultSetMetaData *res_meta = rs->getMetaData();

	int numcols = res_meta->getColumnCount();
	cout << "\nNumber of columns in the result set = " << numcols << endl << endl;

	cout.width(20);
	cout << "Column Name/Label";
	cout.width(20);
	cout << "Column Type";
	cout.width(20);
	cout << "Column Size" << endl;

	for (int i = 0; i < numcols; ++i) {
		cout.width(20);
		cout << res_meta->getColumnLabel(i + 1);
		cout.width(20);
		cout << res_meta->getColumnTypeName(i + 1);
		cout.width(20);
		cout << res_meta->getColumnDisplaySize(i + 1) << endl << endl;
	}

	cout << "\nColumn \"" << res_meta->getColumnLabel(1);
	cout << "\" belongs to the Table: \"" << res_meta->getTableName(1);
	cout << "\" which belongs to the Schema: \"" << res_meta->getSchemaName(1) << "\"" << endl << endl;

} // retrieve_rsmetadata_and_print()


int giri_example(int argc, const char *argv[]) {

	Driver *driver;
	Connection *con;
	Statement *stmt;
	ResultSet *res;
	PreparedStatement *prep_stmt;
	Savepoint *savept;

	int updatecount = 0;

	/* initiate url, user, password and database variables */
	SQLString url(argc >= 2 ? argv[1] : DBHOST);
	SQLString user(argc >= 3 ? argv[2] : USER);
	SQLString password(argc >= 4 ? argv[3] : PASSWORD);
	SQLString database(argc >= 5 ? argv[4] : DATABASE);

	try {
		driver = get_driver_instance();

		/* create a database connection using the Driver */
		con = driver->connect(url, user, password);

		/* alternate syntax using auto_ptr to create the db connection */
		//auto_ptr <Connection> con (driver -> connect(url, user, password));

		/* turn off the autocommit */
		con->setAutoCommit(0);

		cout << "\nDatabase connection\'s autocommit mode = " << con->getAutoCommit() << endl;

		/* select appropriate database schema */
		con->setSchema(database);

		/* retrieve and display the database metadata */
		//retrieve_dbmetadata_and_print(con);

		/* create a statement object */
		stmt = con->createStatement();

		cout << "Executing the Query: \"SELECT * FROM City\" .." << endl;

		/* run a query which returns exactly one result set */
		res = stmt->executeQuery("SELECT name FROM City");

		cout << "Retrieving the result set .." << endl;

		/* retrieve the data from the result set and display on stdout */
		retrieve_data_and_print(res, NUMOFFSET, 1, string("CityName"));

		/* retrieve and display the result set metadata */
		//retrieve_rsmetadata_and_print(res);

		cout << "Demonstrating Prepared Statements .. " << endl << endl;

		/* insert couple of rows of data into City table using Prepared Statements */
		prep_stmt = con->prepareStatement("INSERT INTO City (CityName) VALUES (?)");

		cout << "\tInserting \"London, UK\" into the table, City .." << endl;

		prep_stmt->setString(1, "London, UK");
		updatecount = prep_stmt->executeUpdate();

		cout << "\tCreating a save point \"SAVEPT1\" .." << endl;
		savept = con->setSavepoint("SAVEPT1");

		cout << "\tInserting \"Paris, France\" into the table, City .." << endl;

		prep_stmt->setString(1, "Paris, France");
		updatecount = prep_stmt->executeUpdate();

		cout << "\tRolling back until the last save point \"SAVEPT1\" .." << endl;
		con->rollback(savept);
		con->releaseSavepoint(savept);

		cout << "\tCommitting outstanding updates to the database .." << endl;
		con->commit();

		cout << "\nQuerying the City table again .." << endl;

		/* re-use result set object */
		res = NULL;
		res = stmt->executeQuery("SELECT * FROM City");

		/* retrieve the data from the result set and display on stdout */
		retrieve_data_and_print(res, COLNAME, 1, string("CityName"));

		cout << "Cleaning up the resources .." << endl;

		/* Clean up */
		delete res;
		delete stmt;
		delete prep_stmt;
		con->close();
		delete con;

	} catch (SQLException &e) {
		cout << "ERROR: SQLException in " << __FILE__;
		cout << " (" << __func__ << ") on line " << __LINE__ << endl;
		cout << "ERROR: " << e.what();
		cout << " (MySQL error code: " << e.getErrorCode();
		cout << ", SQLState: " << e.getSQLState() << ")" << endl;

		if (e.getErrorCode() == 1047) {
			/*
			Error: 1047 SQLSTATE: 08S01 (ER_UNKNOWN_COM_ERROR)
			Message: Unknown command
			*/
			cout << "\nYour server does not seem to support Prepared Statements at all. ";
			cout << "Perhaps MYSQL < 4.1?" << endl;
		}

		return EXIT_FAILURE;
	} catch (std::runtime_error &e) {

		cout << "ERROR: runtime_error in " << __FILE__;
		cout << " (" << __func__ << ") on line " << __LINE__ << endl;
		cout << "ERROR: " << e.what() << endl;

		return EXIT_FAILURE;
	}

	return EXIT_SUCCESS;
} // main()
using namespace std;
using namespace sql;

void mysql_example() {
	cout << endl;
	cout << "Running 'SELECT 'Hello World!' >> AS _message'..." << endl;

	try {
		sql::Driver *driver;
		sql::Connection *con;
		sql::Statement *stmt;
		sql::ResultSet *res;

		/* Create a connection */
		driver = get_driver_instance();
		con = driver->connect("tcp://192.168.168.168:3306", "root", "888101");
		/* Connect to the MySQL test database */
		con->setSchema("test");

		stmt = con->createStatement();
		res = stmt->executeQuery("SELECT 'Hello World!' AS _message");
		while (res->next()) {
			cout << "\t... MySQL replies: ";
			/* Access column data by alias or column name */
			cout << res->getString("_message").c_str() << endl;
			cout << "\t... MySQL says it again: ";
			/* Access column fata by numeric offset, 1 is the first column */
			cout << res->getString(1).c_str() << endl;
		}
		delete res;
		delete stmt;
		delete con;

	} catch (sql::SQLException &e) {
		cout << "# ERR: SQLException in " << __FILE__;
		cout << "(" << __FUNCTION__ << ") on line >> " << __LINE__ << endl;
		cout << "# ERR: " << e.what();
		cout << " (MySQL error code: " << e.getErrorCode();
		cout << ", SQLState: " << e.getSQLState() << " )" << endl;
	}

	cout << endl;
}

int mysql_example2()
{
	cout << endl;
	cout << "Let's have MySQL count from 10 to 1..." << endl;

	try {
		sql::Driver *driver;
		sql::Connection *con;
		sql::Statement *stmt;
		sql::ResultSet *res;
		sql::PreparedStatement *pstmt;

		/* Create a connection */
		driver = get_driver_instance();
		con = driver->connect("tcp://192.168.168.168:3306", "root", "888101");
		/* Connect to the MySQL test database */
		con->setSchema("test");

		stmt = con->createStatement();
		stmt->execute("DROP TABLE IF EXISTS test");
		stmt->execute("CREATE TABLE test(id INT)");
		delete stmt;

		/* '?' is the supported placeholder syntax */
		pstmt = con->prepareStatement("INSERT INTO test(id) VALUES (?)");
		for (int i = 1; i <= 10; i++) {
			pstmt->setInt(1, i);
			pstmt->executeUpdate();
		}
		delete pstmt;

		/* Select in ascending order */
		pstmt = con->prepareStatement("SELECT id FROM test ORDER BY id ASC");
		res = pstmt->executeQuery();

		/* Fetch in reverse = descending order! */
		res->afterLast();
		while (res->previous())
			cout << "\t... MySQL counts: " << res->getInt("id") << endl;
		delete res;

		delete pstmt;
		delete con;

	} catch (sql::SQLException &e) {
		cout << "# ERR: SQLException in " << __FILE__;
		cout << "(" << __FUNCTION__ << ") on line " << __LINE__ << endl;
		cout << "# ERR: " << e.what();
		cout << " (MySQL error code: " << e.getErrorCode();
		cout << ", SQLState: " << e.getSQLState() << " )" << endl;
	}

	cout << endl;

	return EXIT_SUCCESS;
}


int main(int argc, const char* argv[])
{
	mysql_example();
	mysql_example2();
	giri_example(argc, argv);
	return EXIT_SUCCESS;
}

