#ifndef RA_FETCHERMANAGERTEST_H
#define RA_FETCHERMANAGERTEST_H

#include <ra/common/Common.h>
#include <ra/common/Log.h>
#include <ra/process/FetcherManager.h>
#include <cppunit/extensions/HelperMacros.h>

RA_BEGIN_NAMESPACE(process);

class FetcherManagerTest : public CppUnit::TestFixture {
    CPPUNIT_TEST_SUITE(FetcherManagerTest);
    CPPUNIT_TEST(testSimpleProcess);
    CPPUNIT_TEST(testStartStop);
    CPPUNIT_TEST(testGetMetricTree);
    CPPUNIT_TEST(testRetrieveMetrics);
    CPPUNIT_TEST_SUITE_END();
public:
    FetcherManagerTest();
    ~FetcherManagerTest();
public:
    void setUp();
    void tearDown();
    void testSimpleProcess();
    void testStartStop();
    void testGetMetricTree();
    void testRetrieveMetrics();
 
private:
    RA_LOG_DECLARE();
};

RA_END_NAMESPACE(process);

#endif //RA_FETCHERMANAGERTEST_H
