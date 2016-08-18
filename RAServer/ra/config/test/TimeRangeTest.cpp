#include <ra/config/test/TimeRangeTest.h>
#include <ra/common/Json.h>
#include <ra/test/test.h>
#include <cppunit/TestAssert.h>

using namespace std;

RA_BEGIN_NAMESPACE(config);
RA_USE_NAMESPACE(common);
RA_LOG_SETUP(config, TimeRangeTest);

CPPUNIT_TEST_SUITE_REGISTRATION(TimeRangeTest);

TimeRangeTest::TimeRangeTest() { 
}

TimeRangeTest::~TimeRangeTest() { 
}

void TimeRangeTest::setUp() { 
    RA_LOG(DEBUG, "setUp!");
}

void TimeRangeTest::tearDown() { 
    RA_LOG(DEBUG, "tearDown!");
}

void TimeRangeTest::testSimpleProcess() { 
    RA_LOG(DEBUG, "Begin Test!");
    TimeRange config;
    string jsonStr = "{"
            "\"begin\":\"21:40\","
            "\"end\":\"22:30\""
            "}";
    JsonPtr json = Json::load(jsonStr);
    CPPUNIT_ASSERT(json);
    CPPUNIT_ASSERT(fromJson(json, config));
    time_t testTime = 1384955037;//pm.9:43:57
    CPPUNIT_ASSERT(config.isInRange(testTime));
    testTime = 1385000052;//am.10:14
    CPPUNIT_ASSERT(!config.isInRange(testTime));
}

void TimeRangeTest::testFormatTime()
{
    TimeRange timeRange;
    int hour, min;
    CPPUNIT_ASSERT(timeRange.formatTime("20:51", hour, min));
    CPPUNIT_ASSERT_EQUAL(20, hour);
    CPPUNIT_ASSERT_EQUAL(51, min);
    CPPUNIT_ASSERT(!timeRange.formatTime("0041", hour, min));
}

void TimeRangeTest::testIsInRangeWithFullDay()
{
    TimeRange config;
    string jsonStr = "{"
            "\"begin\":\"21:40\","
            "\"end\":\"02:30\""
            "}";
    JsonPtr json = Json::load(jsonStr);
    CPPUNIT_ASSERT(json);
    CPPUNIT_ASSERT(fromJson(json, config));
    time_t testTime = 1384012800;//00:00
    CPPUNIT_ASSERT(config.isInRange(testTime));
    testTime = 1384099199;//23:59
    CPPUNIT_ASSERT(config.isInRange(testTime));
    testTime = 1384955037;//pm.9:43:57
    CPPUNIT_ASSERT(config.isInRange(testTime));
}

void TimeRangeTest::testIsInRangeWithOneMin()
{
    TimeRange config;
    string jsonStr = "{"
            "\"begin\":\"00:40\","
            "\"end\":\"00:41\""
            "}";
    JsonPtr json = Json::load(jsonStr);
    CPPUNIT_ASSERT(json);
    CPPUNIT_ASSERT(fromJson(json, config));
    time_t testTime = 1384015200;//00:40
    CPPUNIT_ASSERT(config.isInRange(testTime));
    testTime = 1384015319;//00:41
    CPPUNIT_ASSERT(config.isInRange(testTime));
    testTime = 1384015320;//00:42
    CPPUNIT_ASSERT(!config.isInRange(testTime));
    testTime = 1384015199;//00:39
    CPPUNIT_ASSERT(!config.isInRange(testTime));
}

RA_END_NAMESPACE(config);

