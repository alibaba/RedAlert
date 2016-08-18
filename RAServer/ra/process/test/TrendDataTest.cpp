#include <ra/process/test/TrendDataTest.h>
#include <ra/test/test.h>
#include <cppunit/TestAssert.h>
#include <ra/common/CommonDefine.h>

using namespace std;
RA_BEGIN_NAMESPACE(process);
RA_LOG_SETUP(process, TrendDataTest);

CPPUNIT_TEST_SUITE_REGISTRATION(TrendDataTest);

TrendDataTest::TrendDataTest() { 
}

TrendDataTest::~TrendDataTest() { 
}

void TrendDataTest::setUp() { 
    RA_LOG(DEBUG, "setUp!");
}

void TrendDataTest::tearDown() { 
    RA_LOG(DEBUG, "tearDown!");
}

void TrendDataTest::testSimpleProcess() { 
    RA_LOG(DEBUG, "Begin Test!");

}

void TrendDataTest::testGetForecastVal() 
{
    double smoothingFactor = 0.5;
    double trendFactor = 0.5;

    TrendData trendData;
    trendData.setFactors(smoothingFactor, trendFactor);
    //no key
    TrendKey k;
    k.host = 1000;
    k.metric = "m1";
    uint64_t curTime = 100;
    double forecastVal = 0;
    CPPUNIT_ASSERT(!trendData.getForecastVal(k, curTime, forecastVal));

    //init false
    trendData._data[k].reset();
    CPPUNIT_ASSERT(!trendData.getForecastVal(k, curTime, forecastVal));

    //lastInterval is 0
    trendData._data[k].init = true;
    CPPUNIT_ASSERT(!trendData.getForecastVal(k, curTime, forecastVal));

    //curTime < meta.lastTime
    trendData._data[k].lastInterval = 10;
    trendData._data[k].lastTime = 200;
    trendData._data[k].smoothedValue = 50.0;
    CPPUNIT_ASSERT(trendData.getForecastVal(k, curTime, forecastVal));
    CPPUNIT_ASSERT_EQUAL(50.0, forecastVal);

    //curTime - meta.lastTime > _autoExpireTimeUs
    CPPUNIT_ASSERT_EQUAL(MAX_TREND_HISTORY_MS, trendData._autoExpireTimeUs);
    trendData._autoExpireTimeUs = 50;
    trendData._data[k].lastTime = 0;
    CPPUNIT_ASSERT(!trendData.getForecastVal(k, curTime, forecastVal));
    CPPUNIT_ASSERT(!trendData._data[k].init);
    CPPUNIT_ASSERT_EQUAL((uint64_t)0, trendData._data[k].lastInterval);

    //normal
    trendData._autoExpireTimeUs = 10000;
    trendData._data[k].init = true;
    trendData._data[k].lastTime = 0;
    trendData._data[k].lastInterval = 2;
    trendData._data[k].smoothedValue = 1000;
    trendData._data[k].trendValue = 1;
    CPPUNIT_ASSERT(trendData.getForecastVal(k, curTime, forecastVal));
    CPPUNIT_ASSERT_EQUAL(1000.0 + 1 * ((100 - 0) / 2), forecastVal);
}

void TrendDataTest::testFeedTrend()
{
    TrendKey k;
    k.host = 1000;
    k.metric = "m1";
    uint64_t time = 100;
    double value = 100;
    
    double smoothingFactor = 0.5;
    double trendFactor = 0.5;
    TrendData trendData;
    trendData.setFactors(smoothingFactor, trendFactor);
    
    //first feed
    trendData.feedTrend(k, time, value);
    CPPUNIT_ASSERT(trendData._data.find(k) != trendData._data.end());
    CPPUNIT_ASSERT_EQUAL(true, trendData._data[k].init);
    CPPUNIT_ASSERT_EQUAL(value, trendData._data[k].smoothedValue);
    CPPUNIT_ASSERT_EQUAL(0.0, trendData._data[k].trendValue);
    CPPUNIT_ASSERT_EQUAL((uint64_t)100, trendData._data[k].lastTime);
    CPPUNIT_ASSERT_EQUAL((uint64_t)100, trendData._data[k].lastInterval);

    //second feed
    time = 104;
    value = 104;
    trendData.feedTrend(k, time, value);
    CPPUNIT_ASSERT(trendData._data.find(k) != trendData._data.end());
    CPPUNIT_ASSERT_EQUAL(true, trendData._data[k].init);
    CPPUNIT_ASSERT_EQUAL(102.0, trendData._data[k].smoothedValue);
    CPPUNIT_ASSERT_EQUAL(1.0, trendData._data[k].trendValue);
    CPPUNIT_ASSERT_EQUAL((uint64_t)104, trendData._data[k].lastTime);
    CPPUNIT_ASSERT_EQUAL((uint64_t)4, trendData._data[k].lastInterval);
}

void TrendDataTest::testClearExpiredData()
{
    double smoothingFactor = 0.5;
    double trendFactor = 0.5;
    TrendData trendData;
    trendData.setFactors(smoothingFactor, trendFactor);

    TrendKey k1;
    k1.host = 1000;
    k1.metric = "m1";
    trendData._data[k1].init = true;
    trendData._data[k1].lastTime = 100;

    TrendKey k2;
    k2.host = 2000;
    k2.metric = "m1";
    trendData._data[k2].init = true;
    trendData._data[k2].lastTime = 200;

    TrendKey k3;
    k3.host = 3000;
    k3.metric = "m1";
    trendData._data[k3].init = true;
    trendData._data[k3].lastTime = 300;

    CPPUNIT_ASSERT_EQUAL(MAX_TREND_HISTORY_MS, trendData._autoExpireTimeUs);
    trendData.clearExpiredData(1000);
    CPPUNIT_ASSERT_EQUAL((size_t)3, trendData._data.size());
    trendData.setAutoExpireTimeUs(MAX_TREND_HISTORY_MS + 100);
    CPPUNIT_ASSERT_EQUAL(MAX_TREND_HISTORY_MS, trendData._autoExpireTimeUs);
    trendData.setAutoExpireTimeUs(1000);
    CPPUNIT_ASSERT_EQUAL((uint64_t)1000, trendData._autoExpireTimeUs);
    //equal, not clear
    trendData.clearExpiredData(1000 + 100);
    CPPUNIT_ASSERT_EQUAL((size_t)3, trendData._data.size());
    
    //clear one
    trendData.clearExpiredData(1000 + 100 + 1);
    CPPUNIT_ASSERT_EQUAL((size_t)2, trendData._data.size());
    CPPUNIT_ASSERT_EQUAL((size_t)0, trendData._data.count(k1));
    CPPUNIT_ASSERT_EQUAL((size_t)1, trendData._data.count(k2));
    CPPUNIT_ASSERT_EQUAL((size_t)1, trendData._data.count(k3));

    //clear all
    trendData.clearExpiredData(1000 + 500);
    CPPUNIT_ASSERT_EQUAL((size_t)0, trendData._data.size());
}

RA_END_NAMESPACE(process);

