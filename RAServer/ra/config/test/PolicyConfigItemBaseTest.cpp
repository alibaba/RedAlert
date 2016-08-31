#include <ra/config/test/PolicyConfigItemBaseTest.h>
#include <ra/common/Json.h>
#include <ra/util/Util.h>
#include <ra/test/test.h>
#include <cppunit/TestAssert.h>

using namespace std;
RA_USE_NAMESPACE(util);
RA_USE_NAMESPACE(common);
RA_BEGIN_NAMESPACE(config);

CPPUNIT_TEST_SUITE_REGISTRATION(PolicyConfigItemBaseTest);

PolicyConfigItemBaseTest::PolicyConfigItemBaseTest() { 
}

PolicyConfigItemBaseTest::~PolicyConfigItemBaseTest() { 
}

void PolicyConfigItemBaseTest::setUp() { 
}

void PolicyConfigItemBaseTest::tearDown() { 
}

void PolicyConfigItemBaseTest::testSimpleProcess() { 
}

void PolicyConfigItemBaseTest::testPolicyFilterItemJsonize()
{
    // normal
    string jsonStr = "{"
        "\"metric\":\"m*m\","
        "\"host\":\"addr1,addr2,addr3*\""
        "}";
    PolicyFilterItem policyFilterItem;
    JsonPtr json = Json::load(jsonStr);
    CPPUNIT_ASSERT(json);
    CPPUNIT_ASSERT(fromJson(json, policyFilterItem));
    CPPUNIT_ASSERT_EQUAL(string("m*m"), policyFilterItem.metric);
    CPPUNIT_ASSERT_EQUAL((size_t)3, policyFilterItem.hostSet.size());
    CPPUNIT_ASSERT_EQUAL((size_t)1, policyFilterItem.hostSet.count("addr1"));
    CPPUNIT_ASSERT_EQUAL((size_t)1, policyFilterItem.hostSet.count("addr2"));
    CPPUNIT_ASSERT_EQUAL((size_t)1, policyFilterItem.hostSet.count("addr3*"));

    //host has *
    jsonStr = "{"
        "\"metric\":\"m*m\","
        "\"host\":\"addr1,*,addr3\""
        "}";
    json = Json::load(jsonStr);
    CPPUNIT_ASSERT(json);
    CPPUNIT_ASSERT(fromJson(json, policyFilterItem));
    CPPUNIT_ASSERT_EQUAL(string("m*m"), policyFilterItem.metric);
    CPPUNIT_ASSERT_EQUAL((size_t)1, policyFilterItem.hostSet.size());
    CPPUNIT_ASSERT_EQUAL((size_t)1, policyFilterItem.hostSet.count("*"));

    //default value
    jsonStr = "{}";
    json = Json::load(jsonStr);
    CPPUNIT_ASSERT(json);
    CPPUNIT_ASSERT(fromJson(json, policyFilterItem));
    CPPUNIT_ASSERT_EQUAL(string("*"), policyFilterItem.metric);
    CPPUNIT_ASSERT_EQUAL((size_t)1, policyFilterItem.hostSet.size());
    CPPUNIT_ASSERT_EQUAL((size_t)1, policyFilterItem.hostSet.count("*"));
}

void PolicyConfigItemBaseTest::testPolicyFilterItemIsMatched()
{
    PolicyFilterItem policyFilterItem;
    policyFilterItem.metric = "m1";
    policyFilterItem.hostSet.insert("1.1.1.1");
    CPPUNIT_ASSERT(!policyFilterItem.isMatched("m1"));
    uint32_t ip;
    CPPUNIT_ASSERT(Util::StringToIP("1.1.1.1", ip));
    CPPUNIT_ASSERT(policyFilterItem.isMatched("m1", ip));
    CPPUNIT_ASSERT(!policyFilterItem.isMatched("m2", ip));

    policyFilterItem.hostSet.clear();
    policyFilterItem.hostSet.insert("*");
    CPPUNIT_ASSERT(policyFilterItem.isMatched("m1"));
}

void PolicyConfigItemBaseTest::testIsFiltered()
{
    PolicyFilterItem shieldItem;
    PolicyConfigItemBase policyBase;
    shieldItem.metric = "m1";
    shieldItem.hostSet.insert("1.1.1.*");
    policyBase._policyFilterVec.push_back(shieldItem);
    shieldItem.metric = "n*";
    shieldItem.hostSet.clear();
    shieldItem.hostSet.insert("*");
    policyBase._policyFilterVec.push_back(shieldItem);

    uint32_t ip;
    CPPUNIT_ASSERT(Util::StringToIP("1.1.1.8", ip));
    CPPUNIT_ASSERT(policyBase.isFiltered("m1", ip));
    CPPUNIT_ASSERT(!policyBase.isFiltered("m1"));
    CPPUNIT_ASSERT(!policyBase.isFiltered("m2", ip));
    
    CPPUNIT_ASSERT(policyBase.isFiltered("nb", ip));
    CPPUNIT_ASSERT(policyBase.isFiltered("nc"));
}

void PolicyConfigItemBaseTest::testJsonize()
{
    string jsonStr = "{"
        "\"alarmLevel\":\"alimonitor\","
        "\"alarmGroup\":\"dev\","
        "\"checkType\":\"total\","
        "\"fetchInterval_s\":30,"
        "\"minAlarmInterval_s\":300,"
        "\"effectiveTime\":[{"
              "\"begin\":\"05:00\","
              "\"end\":\"09:30\""
         "}],"
        "\"policyFilter\":[{"
              "\"metric\":\"m1\","
              "\"host\":\"1.1.*.1,2.2.2.2\""
        "},"
        "{"
              "\"metric\":\"n2\","
              "\"host\":\"*,1.1.1.1\""
        "}]"
        
   "}";

    PolicyConfigItemBase config;
    JsonPtr json = Json::load(jsonStr);
    CPPUNIT_ASSERT(json);
    CPPUNIT_ASSERT(fromJson(json, config));
    CPPUNIT_ASSERT_EQUAL(string("alimonitor"), config.getAlarmLevel());
    CPPUNIT_ASSERT_EQUAL(string("dev"), config.getAlarmGroup());
    CPPUNIT_ASSERT_EQUAL(CT_METRIC_TOTAL, config.getCheckType());
    CPPUNIT_ASSERT_EQUAL(30, config.getFetchInterval());
    CPPUNIT_ASSERT_EQUAL(300, config.getMinAlarmInterval());
    CPPUNIT_ASSERT_EQUAL((size_t)1, config._timeRanges.size());
    CPPUNIT_ASSERT_EQUAL(5, config._timeRanges.begin()->_beginHour);
    CPPUNIT_ASSERT_EQUAL(0, config._timeRanges.begin()->_beginMin);
    CPPUNIT_ASSERT_EQUAL(9, config._timeRanges.begin()->_endHour);
    CPPUNIT_ASSERT_EQUAL(30, config._timeRanges.begin()->_endMin);
    CPPUNIT_ASSERT_EQUAL((size_t)2, config._policyFilterVec.size());

    PolicyFilterItem shieldItem = *config._policyFilterVec.begin();
    CPPUNIT_ASSERT_EQUAL(string("m1"), shieldItem.metric);
    CPPUNIT_ASSERT_EQUAL((size_t)2, shieldItem.hostSet.size());
    CPPUNIT_ASSERT_EQUAL((size_t)1, shieldItem.hostSet.count("1.1.*.1"));
    CPPUNIT_ASSERT_EQUAL((size_t)1, shieldItem.hostSet.count("2.2.2.2"));

    shieldItem = *(config._policyFilterVec.begin() + 1);
    CPPUNIT_ASSERT_EQUAL(string("n2"), shieldItem.metric);
    CPPUNIT_ASSERT_EQUAL((size_t)1, shieldItem.hostSet.size());
    CPPUNIT_ASSERT_EQUAL((size_t)1, shieldItem.hostSet.count("*"));

    CPPUNIT_ASSERT_EQUAL((size_t)1, config._retrieveTimeRangeVec.size());
    CPPUNIT_ASSERT_EQUAL(-30, config._retrieveTimeRangeVec[0].start);
    CPPUNIT_ASSERT_EQUAL(-0, config._retrieveTimeRangeVec[0].end);
}


RA_END_NAMESPACE(config);

