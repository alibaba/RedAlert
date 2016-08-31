#ifndef RA_SQLDATATEST_H
#define RA_SQLDATATEST_H

#include <ra/common/CommonDefine.h>
#include <ra/common/Log.h>
#include <ra/util/SqlData.h>
#include <cppunit/extensions/HelperMacros.h>

RA_BEGIN_NAMESPACE(util);

class SqlDataTest : public CppUnit::TestFixture {
    CPPUNIT_TEST_SUITE(SqlDataTest);
    CPPUNIT_TEST(testSimpleProcess);
    CPPUNIT_TEST(testLoadFailed);
    CPPUNIT_TEST(testGetRow);
    CPPUNIT_TEST(testGetValue);
    CPPUNIT_TEST_SUITE_END();
public:
    SqlDataTest();
    ~SqlDataTest();
public:
    void setUp();
    void tearDown();
    void testSimpleProcess();
    void testLoadFailed();
    void testGetRow();
    void testGetValue();
private:
    std::string _filePath;
};

RA_END_NAMESPACE(util);

#endif //RA_SQLDATATEST_H
