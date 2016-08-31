#include <ra/config/test/CyclePolicyItemTest.h>
#include <ra/common/Json.h>
#include <ra/test/test.h>
#include <cppunit/TestAssert.h>
#include <vector>
#include <iostream>

using namespace std;

RA_BEGIN_NAMESPACE(config);
RA_USE_NAMESPACE(common);

CPPUNIT_TEST_SUITE_REGISTRATION(CyclePolicyItemTest);

CyclePolicyItemTest::CyclePolicyItemTest() { 
}

CyclePolicyItemTest::~CyclePolicyItemTest() { 
}

void CyclePolicyItemTest::setUp() { 
}

void CyclePolicyItemTest::tearDown() { 
}

void CyclePolicyItemTest::testSimpleProcess() { 
    std::string jsonStr = "{"
                          "\"group\":\"default\","
                          "\"metric\":\"test_2\","
                          "\"alarmLevel\":\"warning\","
                          "\"alarmGroup\":\"dev\","
                          "\"checkType\":\"total\","
                          "\"cycleCount\":2,"
                          "\"fetchInterval_s\":60,"
                          "\"minAlarmInterval_s\":300,"
                          "\"maxDiffRatio\":0.2,"
                          "\"minDiffValue\":10.01,"
                          "\"cycleTime_s\":99"
                          "}";

    CyclePolicyItem config;
    JsonPtr json = Json::load(jsonStr);
    CPPUNIT_ASSERT(json);
    CPPUNIT_ASSERT(fromJson(json, config));

    CPPUNIT_ASSERT_EQUAL(99, config.getCycleTime());
    CPPUNIT_ASSERT_EQUAL(2, config.getCycleCount());

    int32_t fetchInterval = config.getFetchInterval();
    CPPUNIT_ASSERT_EQUAL(60, fetchInterval);
    vector<RetrieveTimeRange> retrieveTimeRangeVec;
    config.getRetrieveTimeRange(retrieveTimeRangeVec);
    CPPUNIT_ASSERT_EQUAL((size_t)3, retrieveTimeRangeVec.size());
    CPPUNIT_ASSERT_EQUAL(-60, retrieveTimeRangeVec[0].start);
    CPPUNIT_ASSERT_EQUAL(0, retrieveTimeRangeVec[0].end);
    CPPUNIT_ASSERT_EQUAL(-99 -60, retrieveTimeRangeVec[1].start);
    CPPUNIT_ASSERT_EQUAL(-99, retrieveTimeRangeVec[1].end);
    CPPUNIT_ASSERT_EQUAL(-99 * 2 -60, retrieveTimeRangeVec[2].start);
    CPPUNIT_ASSERT_EQUAL(-99 * 2, retrieveTimeRangeVec[2].end);
}

void CyclePolicyItemTest::testDefaultCycleTime()
{
    std::string jsonStr = "{"
                          "\"group\":\"default\","
                          "\"metric\":\"test_2\","
                          "\"alarmLevel\":\"warning\","
                          "\"alarmGroup\":\"dev\","
                          "\"checkType\":\"total\","
                          "\"fetchInterval_s\":60,"
                          "\"minAlarmInterval_s\":300,"
                          "\"maxDiffRatio\":0.2,"
                          "\"minDiffValue\":10.01"
                          "}";

    CyclePolicyItem config;
    JsonPtr json = Json::load(jsonStr);
    CPPUNIT_ASSERT(json);
    CPPUNIT_ASSERT(fromJson(json, config));
    CPPUNIT_ASSERT_EQUAL(86400, config.getCycleTime());
    CPPUNIT_ASSERT_EQUAL(3, config.getCycleCount());

    int32_t fetchInterval = config.getFetchInterval();
    CPPUNIT_ASSERT_EQUAL(60, fetchInterval);
    vector<RetrieveTimeRange> retrieveTimeRangeVec;
    config.getRetrieveTimeRange(retrieveTimeRangeVec);
    CPPUNIT_ASSERT_EQUAL((size_t)4, retrieveTimeRangeVec.size());
    CPPUNIT_ASSERT_EQUAL(-60, retrieveTimeRangeVec[0].start);
    CPPUNIT_ASSERT_EQUAL(0, retrieveTimeRangeVec[0].end);
    CPPUNIT_ASSERT_EQUAL(-86400 -60, retrieveTimeRangeVec[1].start);
    CPPUNIT_ASSERT_EQUAL(-86400, retrieveTimeRangeVec[1].end);
    CPPUNIT_ASSERT_EQUAL(-86400 * 2 -60, retrieveTimeRangeVec[2].start);
    CPPUNIT_ASSERT_EQUAL(-86400 * 2, retrieveTimeRangeVec[2].end);
    CPPUNIT_ASSERT_EQUAL(-86400 * 3 -60, retrieveTimeRangeVec[3].start);
    CPPUNIT_ASSERT_EQUAL(-86400 * 3, retrieveTimeRangeVec[3].end);
}

void CyclePolicyItemTest::testDiffTooMuch(){
    std::string jsonStr = "{"
            "\"group\":\"default\","
            "\"metric\":\"test_2\","
            "\"alarmLevel\":\"warning\","
            "\"alarmGroup\":\"dev\","
            "\"checkType\":\"total\","
            "\"fetchInterval_s\":60,"
            "\"minAlarmInterval_s\":300,"
            "\"ascDiffRatio\":0.2,"
            "\"ascDiffValue\":5,"
            "\"descDiffRatio\":0.3,"
            "\"descDiffValue\":20"
            "}";

    CyclePolicyItem config;
    JsonPtr json = Json::load(jsonStr);
    CPPUNIT_ASSERT(json);
    CPPUNIT_ASSERT(fromJson(json, config));

    CPPUNIT_ASSERT(!config.isDiffTooMuch(60.0, vector<double>()));

    vector<double> historyValVec0;
    historyValVec0.push_back(50.0);
    CPPUNIT_ASSERT(config.isDiffTooMuch(60, historyValVec0));
    CPPUNIT_ASSERT(!config.isDiffTooMuch(59.5, historyValVec0));

    vector<double> historyValVec1;
    historyValVec1.push_back(100.0);
    CPPUNIT_ASSERT(config.isDiffTooMuch(69.5, historyValVec1));
    CPPUNIT_ASSERT(!config.isDiffTooMuch(70.5, historyValVec1));

    vector<double> historyValVec2;
    historyValVec2.push_back(50.0);
    historyValVec2.push_back(100.0);
    CPPUNIT_ASSERT(!config.isDiffTooMuch(52, historyValVec2));
    CPPUNIT_ASSERT(config.isDiffTooMuch(69.5, historyValVec2));

    vector<double> historyValVec3;
    historyValVec3.push_back(50.0);
    historyValVec3.push_back(100.0);
    historyValVec3.push_back(140.0);
    CPPUNIT_ASSERT(config.isDiffTooMuch(60, historyValVec3));
    CPPUNIT_ASSERT(!config.isDiffTooMuch(119.5, historyValVec3));
}


RA_END_NAMESPACE(config);

