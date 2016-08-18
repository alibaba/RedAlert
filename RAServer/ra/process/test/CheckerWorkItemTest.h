#ifndef RA_CHECKERWORKITEMTEST_H
#define RA_CHECKERWORKITEMTEST_H

#include <ra/common/Common.h>
#include <ra/common/Log.h>
#include <ra/process/CheckerWorkItem.h>
#include <cppunit/extensions/HelperMacros.h>

RA_BEGIN_NAMESPACE(process);

class CheckerWorkItemTest : public CppUnit::TestFixture {
    CPPUNIT_TEST_SUITE(CheckerWorkItemTest);
    CPPUNIT_TEST(testSimpleProcess);
    CPPUNIT_TEST(testInit);
    CPPUNIT_TEST_SUITE_END();
public:
    CheckerWorkItemTest();
    ~CheckerWorkItemTest();
public:
    void setUp();
    void tearDown();
    void testSimpleProcess();
    void testInit();
    


private:
    RA_LOG_DECLARE();
};

RA_END_NAMESPACE(process);

#endif //RA_CHECKERWORKITEMTEST_H
