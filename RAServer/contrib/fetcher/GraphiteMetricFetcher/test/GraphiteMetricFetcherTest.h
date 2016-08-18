#ifndef RA_GRAPHITEMETRICFETCHERTEST_H
#define RA_GRAPHITEMETRICFETCHERTEST_H

#include <ra/common/Common.h>
#include <ra/common/Log.h>
#include <cppunit/extensions/HelperMacros.h>

RA_BEGIN_NAMESPACE(fetcher);

class GraphiteMetricFetcherTest : public CppUnit::TestFixture {
    CPPUNIT_TEST_SUITE(GraphiteMetricFetcherTest);
    CPPUNIT_TEST(testSimpleProcess);
    CPPUNIT_TEST(testParseSubMetrics);
    CPPUNIT_TEST(testParseSubMetricData);
    CPPUNIT_TEST(testGetDataURL);
    CPPUNIT_TEST(testGetMetricsURL);
    CPPUNIT_TEST(testInit);
    CPPUNIT_TEST(testTraverse);
    CPPUNIT_TEST(testReadGraphiteData);
    CPPUNIT_TEST(testRetrieve);
    //CPPUNIT_TEST(testMain);
    CPPUNIT_TEST(testGetAllMetricsURL);
    CPPUNIT_TEST(testParseAllMetrics);
    CPPUNIT_TEST(testMakeTree);
    CPPUNIT_TEST_SUITE_END();
public:
    GraphiteMetricFetcherTest();
    ~GraphiteMetricFetcherTest();
public:
    void setUp();
    void tearDown();
    void testSimpleProcess();
    void testParseSubMetrics();
    void testParseSubMetricData();
    void testGetDataURL();
    void testGetMetricsURL();
    void testInit();
    void testTraverse();
    void testReadGraphiteData();
    void testRetrieve();
    void testMain();
    void testGetAllMetricsURL();
    void testParseAllMetrics();
    void testMakeTree();
private:
    RA_LOG_DECLARE();
};

RA_END_NAMESPACE(fetcher);

#endif //RA_GRAPHITEMETRICFETCHERTEST_H
