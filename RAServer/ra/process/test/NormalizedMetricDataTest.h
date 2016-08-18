#ifndef RA_NORMALIZEDMETRICDATATEST_H
#define RA_NORMALIZEDMETRICDATATEST_H

#include <ra/common/Common.h>
#include <ra/common/Log.h>
#include <ra/process/NormalizedMetricData.h>
#include <cppunit/extensions/HelperMacros.h>

RA_BEGIN_NAMESPACE(process);

class NormalizedMetricDataTest : public CppUnit::TestFixture {
    CPPUNIT_TEST_SUITE(NormalizedMetricDataTest);
    CPPUNIT_TEST(testSimpleProcess);
    CPPUNIT_TEST(testGetValue);
    CPPUNIT_TEST(testGetTime);
    CPPUNIT_TEST(testGenSampleTimeList);
    CPPUNIT_TEST(testSampleValues);
    CPPUNIT_TEST(testGenNormalizedTotalValue);
    CPPUNIT_TEST(testGetTotalValue);
    CPPUNIT_TEST_SUITE_END();
public:
    NormalizedMetricDataTest();
    ~NormalizedMetricDataTest();
public:
    void setUp();
    void tearDown();
    void testSimpleProcess();
    void testGetValue();
    void testGetTime();
    void testGenSampleTimeList();
    void testSampleValues();
    void testGenNormalizedTotalValue();
    void testGetTotalValue();

private:
    RA_LOG_DECLARE();
};

RA_END_NAMESPACE(process);

#endif //RA_NORMALIZEDMETRICDATATEST_H
