#ifndef RA_METRICFETCHERTEST_H
#define RA_METRICFETCHERTEST_H

#include <cppunit/extensions/HelperMacros.h>
#include <ra/common/CommonDefine.h>
#include <ra/common/Log.h>

RA_BEGIN_NAMESPACE(fetcher);

class MetricFetcherTest : public CppUnit::TestFixture {
    CPPUNIT_TEST_SUITE(MetricFetcherTest);
    CPPUNIT_TEST(testAllocTree);
    CPPUNIT_TEST(testCloneTree);
    CPPUNIT_TEST(testRetrieve);
    CPPUNIT_TEST(testTraverse);
    CPPUNIT_TEST_SUITE_END();

public:
    MetricFetcherTest();
    ~MetricFetcherTest();

public:
    void setUp();
    void tearDown();

    void testAllocTree();
    void testCloneTree();
    void testRetrieve();
    void testTraverse();

    bool printPath(const MetricPath& path);
};

RA_END_NAMESPACE(fetcher);

#endif //RA_METRICFETCHERTEST_H
