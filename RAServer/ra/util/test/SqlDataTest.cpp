#include <ra/util/test/SqlDataTest.h>
#include <ra/test/test.h>
#include <cppunit/TestAssert.h>
#include <sqlite3.h>
#include <ra/util/FileUtil.h>

using namespace std;

RA_BEGIN_NAMESPACE(util);
RA_LOG_SETUP(util, SqlDataTest);

CPPUNIT_TEST_SUITE_REGISTRATION(SqlDataTest);

SqlDataTest::SqlDataTest() {
    _filePath = "test.db";
}

SqlDataTest::~SqlDataTest() {
}

void SqlDataTest::setUp() { 
    sqlite3 *db = NULL;
    int rc;

    FileUtil::removeLocalFile(_filePath);
    rc = sqlite3_open(_filePath.c_str(), &db);
    if (rc)
    {
        sqlite3_close(db);
        CPPUNIT_ASSERT(false);
    }

    const char *sql = "create table TestTable(id integer primary key, data integer);" ;
    sqlite3_exec(db, sql, 0, 0, 0);

    sql = "insert into 'TestTable' values(1, 1);" ;
    sqlite3_exec(db, sql, 0, 0, 0);
    sql = "insert into 'TestTable' values(2, 3);" ;
    sqlite3_exec(db, sql, 0, 0, 0);
    sql = "insert into 'TestTable' values(3, 5);" ;
    sqlite3_exec(db, sql, 0, 0, 0);

    sqlite3_close(db);
}

void SqlDataTest::tearDown() { 
    FileUtil::removeLocalFile(_filePath);
}

void SqlDataTest::testSimpleProcess() { 
    SqlData data(_filePath);
    CPPUNIT_ASSERT_EQUAL(_filePath, data.getFilePath());
    CPPUNIT_ASSERT(data.load("TestTable"));
    CPPUNIT_ASSERT_EQUAL(3, data.getRow());
    CPPUNIT_ASSERT_EQUAL(2, data.getCol());
    string value;
    CPPUNIT_ASSERT(data.getValue(0, 0, value));
    CPPUNIT_ASSERT_EQUAL(string("1"), value);
    CPPUNIT_ASSERT(data.getValue(0, 1, value));
    CPPUNIT_ASSERT_EQUAL(string("1"), value);
    CPPUNIT_ASSERT(data.getValue(1, 0, value));
    CPPUNIT_ASSERT_EQUAL(string("2"), value);
    CPPUNIT_ASSERT(data.getValue(1, 1, value));
    CPPUNIT_ASSERT_EQUAL(string("3"), value);
    vector<string> row;
    CPPUNIT_ASSERT(data.getRow(2, row));
    CPPUNIT_ASSERT_EQUAL(size_t(2), row.size());
    CPPUNIT_ASSERT_EQUAL(string("3"), row[0]);
    CPPUNIT_ASSERT_EQUAL(string("5"), row[1]);
    data.close();

    string invalidPath = TEST_DATA_PATH "sqlite/invalid.db";
    data.setFilePath(invalidPath);
    CPPUNIT_ASSERT(!data.load("TestTable"));
    CPPUNIT_ASSERT_EQUAL(0, data.getRow());
    CPPUNIT_ASSERT_EQUAL(0, data.getCol());
    data.close();

    string textPath = TEST_DATA_PATH "sqlite/text.db";
    data.setFilePath(textPath);
    CPPUNIT_ASSERT(!data.load("TestTable"));
    CPPUNIT_ASSERT_EQUAL(0, data.getRow());
    CPPUNIT_ASSERT_EQUAL(0, data.getCol());
    data.close();
}

void SqlDataTest::testLoadFailed() { 
    SqlData data("invalid.db");
    CPPUNIT_ASSERT(!data.load("TestTable"));

    data.setFilePath(_filePath);
    CPPUNIT_ASSERT(!data.load("InvalidTable"));
    CPPUNIT_ASSERT_EQUAL(0, data.getRow());
    CPPUNIT_ASSERT_EQUAL(0, data.getCol());
    data.close();
    CPPUNIT_ASSERT(data.load("TestTable"));
    CPPUNIT_ASSERT(!data.load("TestTable"));
    data.close();
}

void SqlDataTest::testGetRow() {
    SqlData data(_filePath);
    CPPUNIT_ASSERT(data.load("TestTable"));
    vector<string> row;
    CPPUNIT_ASSERT(data.getRow(0, row));
    CPPUNIT_ASSERT_EQUAL(size_t(2), row.size());
    CPPUNIT_ASSERT_EQUAL(string("1"), row[0]);
    CPPUNIT_ASSERT_EQUAL(string("1"), row[1]);
    CPPUNIT_ASSERT(data.getRow(1, row));
    CPPUNIT_ASSERT_EQUAL(size_t(2), row.size());
    CPPUNIT_ASSERT_EQUAL(string("2"), row[0]);
    CPPUNIT_ASSERT_EQUAL(string("3"), row[1]);
    CPPUNIT_ASSERT(data.getRow(2, row));
    CPPUNIT_ASSERT_EQUAL(size_t(2), row.size());
    CPPUNIT_ASSERT_EQUAL(string("3"), row[0]);
    CPPUNIT_ASSERT_EQUAL(string("5"), row[1]);
    CPPUNIT_ASSERT(!data.getRow(3, row));
}

void SqlDataTest::testGetValue() {
    SqlData data(_filePath);
    CPPUNIT_ASSERT(data.load("TestTable"));
    string value;
    CPPUNIT_ASSERT(data.getValue(0, 0, value));
    CPPUNIT_ASSERT_EQUAL(string("1"), value);
    CPPUNIT_ASSERT(data.getValue(0, 1, value));
    CPPUNIT_ASSERT_EQUAL(string("1"), value);
    CPPUNIT_ASSERT(data.getValue(1, 0, value));
    CPPUNIT_ASSERT_EQUAL(string("2"), value);
    CPPUNIT_ASSERT(data.getValue(1, 1, value));
    CPPUNIT_ASSERT_EQUAL(string("3"), value);
    CPPUNIT_ASSERT(!data.getValue(3, 0, value));
    CPPUNIT_ASSERT(!data.getValue(0, 2, value));
}

RA_END_NAMESPACE(util);

