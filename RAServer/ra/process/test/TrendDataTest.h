#ifndef RA_TRENDDATATEST_H
#define RA_TRENDDATATEST_H

#include <ra/common/Common.h>
#include <ra/common/Log.h>
#include <ra/process/TrendData.h>
#include <cppunit/extensions/HelperMacros.h>

RA_BEGIN_NAMESPACE(process);

class TrendDataTest : public CppUnit::TestFixture {
    CPPUNIT_TEST_SUITE(TrendDataTest);
    CPPUNIT_TEST(testSimpleProcess);
    CPPUNIT_TEST(testGetForecastVal);
    CPPUNIT_TEST(testFeedTrend);
    CPPUNIT_TEST(testClearExpiredData);
    CPPUNIT_TEST_SUITE_END();
public:
    TrendDataTest();
    ~TrendDataTest();
public:
    void setUp();
    void tearDown();
    void testSimpleProcess();
    void testGetForecastVal();
    void testFeedTrend();
    void testClearExpiredData();
private:
    RA_LOG_DECLARE();
};

RA_END_NAMESPACE(process);

#endif //RA_TRENDDATATEST_H
