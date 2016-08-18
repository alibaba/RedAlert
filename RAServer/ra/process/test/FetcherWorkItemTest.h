#ifndef RA_FETCHERWORKITEMTEST_H
#define RA_FETCHERWORKITEMTEST_H

#include <ra/common/Common.h>
#include <ra/common/Log.h>
#include <ra/process/FetcherWorkItem.h>
#include <cppunit/extensions/HelperMacros.h>

RA_BEGIN_NAMESPACE(process);

class FetcherWorkItemTest : public CppUnit::TestFixture {
    CPPUNIT_TEST_SUITE(FetcherWorkItemTest);
    CPPUNIT_TEST(testSimpleProcess);
    CPPUNIT_TEST_SUITE_END();
public:
    FetcherWorkItemTest();
    ~FetcherWorkItemTest();
public:
    void setUp();
    void tearDown();
    void testSimpleProcess();
private:
    RA_LOG_DECLARE();
};

RA_END_NAMESPACE(process);

#endif //RA_FETCHERWORKITEMTEST_H
