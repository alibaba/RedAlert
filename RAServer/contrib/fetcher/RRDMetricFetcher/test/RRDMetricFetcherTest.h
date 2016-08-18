#ifndef RA_RRDMETRICFETCHERTEST_H
#define RA_RRDMETRICFETCHERTEST_H

#include <ra/common/Common.h>
#include <ra/common/Log.h>
#include <cppunit/extensions/HelperMacros.h>

RA_BEGIN_NAMESPACE(fetcher);

class RRDMetricFetcherTest : public CppUnit::TestFixture {
    CPPUNIT_TEST_SUITE(RRDMetricFetcherTest);
    CPPUNIT_TEST(testGetMetricName);
    CPPUNIT_TEST(testCompareAndAddChild);
    CPPUNIT_TEST(testTraverse);
    CPPUNIT_TEST(testRetrieve);
    CPPUNIT_TEST(testReadRRDData);
    CPPUNIT_TEST(testGetMetricValue);
    CPPUNIT_TEST_SUITE_END();
public:
    RRDMetricFetcherTest();
    ~RRDMetricFetcherTest();
public:
    void setUp();
    void tearDown();
    void testGetMetricName();
    void testCompareAndAddChild();
    void testTraverse();
    void testRetrieve();
    void testReadRRDData();
    void testGetMetricValue();
private:
    void checkChildrenLabel(const MetricNodePtr &parent, const std::vector<std::string> & labels);
private:
    RA_LOG_DECLARE();
};

RA_END_NAMESPACE(fetcher);

#endif //RA_RRDMETRICFETCHERTEST_H
