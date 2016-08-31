#include <ra/config/test/PolicyConfigTest.h>
#include <ra/test/test.h>
#include <ra/util/Util.h>
#include <cppunit/TestAssert.h>
#include <ra/util/ConsistentHash.h>

using namespace std;
using namespace ra::config;
using namespace ra::util;
RA_USE_NAMESPACE(util);

RA_BEGIN_NAMESPACE(config);
RA_LOG_SETUP(config, PolicyConfigTest);

CPPUNIT_TEST_SUITE_REGISTRATION(PolicyConfigTest);


PolicyConfigTest::PolicyConfigTest() {
}

PolicyConfigTest::~PolicyConfigTest() {
}

void PolicyConfigTest::setUp() {
}

void PolicyConfigTest::tearDown() {
}

void PolicyConfigTest::testLoadConfig()
{
    PolicyConfig policyConfig;
    //path not exist
    string sqlitePath = string(TEST_DATA_PATH) + "/config_wrapper/not_exist_sqlite";
    bool ret = policyConfig.loadConfig(sqlitePath);
    CPPUNIT_ASSERT(!ret);

    //empty table
    sqlitePath = string(TEST_DATA_PATH) + "/config_wrapper/sqlite_miss_options";
    ret = policyConfig.loadConfig(sqlitePath);
    CPPUNIT_ASSERT(ret);
    CPPUNIT_ASSERT_EQUAL((size_t)0, policyConfig._policyItemVec.size());

    //load sucess
    sqlitePath = string(TEST_DATA_PATH) + "/config_wrapper/sqlite_normal";
    ret = policyConfig.loadConfig(sqlitePath);
    CPPUNIT_ASSERT(ret);
    CPPUNIT_ASSERT_EQUAL((size_t)8, policyConfig._policyItemVec.size());
    
    ThresholdPolicyItem* ptr = dynamic_cast<ThresholdPolicyItem*>(
        policyConfig._policyItemVec[0].get());
    assert(NULL != ptr);
    CPPUNIT_ASSERT_EQUAL(2U, ptr->getId());
    CPPUNIT_ASSERT_EQUAL(string("online_worker"), ptr->getGroup());
    CPPUNIT_ASSERT_EQUAL(string("metric_threshold"), ptr->getMetric());
    CPPUNIT_ASSERT_EQUAL(string("alimonitor"), ptr->getAlarmLevel());
    CPPUNIT_ASSERT_EQUAL(string("online_vip"), ptr->getAlarmGroup());
    CPPUNIT_ASSERT_EQUAL(CT_SINGLE, ptr->getCheckType());
    CPPUNIT_ASSERT_EQUAL(60, ptr->getFetchInterval());
    CPPUNIT_ASSERT_EQUAL(60, ptr->getMinAlarmInterval());
    CPPUNIT_ASSERT_EQUAL(PT_THRESHOLD, ptr->getTriggerType());
    CPPUNIT_ASSERT_EQUAL(1.0, ptr->getUpBound());
    CPPUNIT_ASSERT_EQUAL(0.0, ptr->getDownBound());


    CyclePolicyItem* ptr2 = dynamic_cast<CyclePolicyItem*>(
        policyConfig._policyItemVec[2].get());
    assert(NULL != ptr2);
    CPPUNIT_ASSERT_EQUAL(18U, ptr2->getId());
    CPPUNIT_ASSERT_EQUAL(string("online"), ptr2->getGroup());
    CPPUNIT_ASSERT_EQUAL(string("metric_cycle"), ptr2->getMetric());
    CPPUNIT_ASSERT_EQUAL(string("curlmail"), ptr2->getAlarmLevel());
    CPPUNIT_ASSERT_EQUAL(string("cycle_group"), ptr2->getAlarmGroup());
    CPPUNIT_ASSERT_EQUAL(CT_METRIC_TOTAL, ptr2->getCheckType());
    CPPUNIT_ASSERT_EQUAL(900, ptr2->getFetchInterval());
    CPPUNIT_ASSERT_EQUAL(1800, ptr2->getMinAlarmInterval());
    CPPUNIT_ASSERT_EQUAL(PT_CYCLE, ptr2->getTriggerType());
    CPPUNIT_ASSERT_EQUAL(0.1, ptr2->getAscDiffRatio());
    CPPUNIT_ASSERT_EQUAL(5.0, ptr2->getAscDiffValue());
    CPPUNIT_ASSERT_EQUAL(0.1, ptr2->getDescDiffRatio());
    CPPUNIT_ASSERT_EQUAL(5.0, ptr2->getDescDiffValue());
    CPPUNIT_ASSERT_EQUAL(604800, ptr2->getCycleTime());
    CPPUNIT_ASSERT_EQUAL(3, ptr2->getCycleCount());

    
    SingularityPolicyItem* ptr3 = dynamic_cast<SingularityPolicyItem*>(
        policyConfig._policyItemVec[3].get());
    assert(NULL != ptr3);
    CPPUNIT_ASSERT_EQUAL(41U, ptr3->getId());
    CPPUNIT_ASSERT_EQUAL(string("online_worker"), ptr3->getGroup());
    CPPUNIT_ASSERT_EQUAL(string("metric_singularity"), ptr3->getMetric());
    CPPUNIT_ASSERT_EQUAL(string("alimonitor"), ptr3->getAlarmLevel());
    CPPUNIT_ASSERT_EQUAL(string("shenma_worker_latency_mon"), ptr3->getAlarmGroup());
    CPPUNIT_ASSERT_EQUAL(CT_SINGLE, ptr3->getCheckType());
    CPPUNIT_ASSERT_EQUAL(60, ptr3->getFetchInterval());
    CPPUNIT_ASSERT_EQUAL(60, ptr3->getMinAlarmInterval());
    CPPUNIT_ASSERT_EQUAL(PT_SINGULARITY, ptr3->getTriggerType());
    CPPUNIT_ASSERT_EQUAL(0.0, ptr3->getAscDiffRatio());
    CPPUNIT_ASSERT_EQUAL(0.5, ptr3->getAscDiffValue());
    CPPUNIT_ASSERT_EQUAL(0.0, ptr3->getDescDiffRatio());
    CPPUNIT_ASSERT_EQUAL(0.5, ptr3->getDescDiffValue());


    TrendPolicyItem* ptr4 = dynamic_cast<TrendPolicyItem*>(
        policyConfig._policyItemVec[5].get());
    assert(NULL != ptr4);
    CPPUNIT_ASSERT_EQUAL(205U, ptr4->getId());
    CPPUNIT_ASSERT_EQUAL(string("online"), ptr4->getGroup());
    CPPUNIT_ASSERT_EQUAL(string("metric_trend"), ptr4->getMetric());
    CPPUNIT_ASSERT_EQUAL(string("alimonitor"), ptr4->getAlarmLevel());
    CPPUNIT_ASSERT_EQUAL(string("shenma_ra_pe_api"), ptr4->getAlarmGroup());
    CPPUNIT_ASSERT_EQUAL(CT_METRIC_TOTAL, ptr4->getCheckType());
    CPPUNIT_ASSERT_EQUAL(60, ptr4->getFetchInterval());
    CPPUNIT_ASSERT_EQUAL(600, ptr4->getMinAlarmInterval());
    CPPUNIT_ASSERT_EQUAL(PT_TREND, ptr4->getTriggerType());
    CPPUNIT_ASSERT_EQUAL(0.1, ptr4->getAscDiffRatio());
    CPPUNIT_ASSERT_EQUAL(10.0, ptr4->getAscDiffValue());
    CPPUNIT_ASSERT_EQUAL(0.1, ptr4->getDescDiffRatio());
    CPPUNIT_ASSERT_EQUAL(10.0, ptr4->getDescDiffValue());

    AvailabilityPolicyItem* ptr5 = dynamic_cast<AvailabilityPolicyItem*>(
        policyConfig._policyItemVec[7].get());
    assert(NULL != ptr5);
    CPPUNIT_ASSERT_EQUAL(99999U, ptr5->getId());
    CPPUNIT_ASSERT_EQUAL(string("availability_group"), ptr5->getGroup());
    CPPUNIT_ASSERT_EQUAL(string("metric_availability"), ptr5->getMetric());
    CPPUNIT_ASSERT_EQUAL(string("alimonitor"), ptr5->getAlarmLevel());
    CPPUNIT_ASSERT_EQUAL(string("dev"), ptr5->getAlarmGroup());
    CPPUNIT_ASSERT_EQUAL(CT_SINGLE, ptr5->getCheckType());
    CPPUNIT_ASSERT_EQUAL(60, ptr5->getFetchInterval());
    CPPUNIT_ASSERT_EQUAL(300, ptr5->getMinAlarmInterval());
    CPPUNIT_ASSERT_EQUAL(PT_AVAILABILITY, ptr5->getTriggerType());
    CPPUNIT_ASSERT_EQUAL(99, ptr5->getMinHostNum());
    CPPUNIT_ASSERT_EQUAL(101, ptr5->getMaxMissHostNum());
}

static string pattern = "";
bool isMyMetric(const string& str)
{
    return Util::isPatternMatch(pattern, str);
}

void PolicyConfigTest::testFilterMetrics()
{
    PolicyConfig policyConfig;
    CPPUNIT_ASSERT_EQUAL((size_t)0, policyConfig._policyItemVec.size());

    TrendPolicyItemPtr trendPtr(new TrendPolicyItem());
    trendPtr->setMetric("abcdefg");
    policyConfig._policyItemVec.push_back(trendPtr);
    CPPUNIT_ASSERT_EQUAL((size_t)1, policyConfig._policyItemVec.size());

    ThresholdPolicyItemPtr thresholdPtr(new ThresholdPolicyItem());
    thresholdPtr->setMetric("123456*");
    policyConfig._policyItemVec.push_back(thresholdPtr);
    CPPUNIT_ASSERT_EQUAL((size_t)2, policyConfig._policyItemVec.size());

    AvailabilityPolicyItemPtr availabilityPtr(new AvailabilityPolicyItem());
    availabilityPtr->setMetric("1a2b3c");
    policyConfig._policyItemVec.push_back(availabilityPtr);
    CPPUNIT_ASSERT_EQUAL((size_t)3, policyConfig._policyItemVec.size());

    //testIsMonitored
    CPPUNIT_ASSERT(policyConfig.isMonitored("abcdefg"));
    CPPUNIT_ASSERT(policyConfig.isMonitored("123456789"));
    CPPUNIT_ASSERT(!policyConfig.isMonitored("654321"));

    //testFilterMetric
    pattern = "*";
    policyConfig.filterMetrics(isMyMetric);
    CPPUNIT_ASSERT_EQUAL((size_t)3, policyConfig._policyItemVec.size());

    pattern = "*b*";
    policyConfig.filterMetrics(isMyMetric);
    CPPUNIT_ASSERT_EQUAL((size_t)2, policyConfig._policyItemVec.size());

    pattern = "xxxx";
    policyConfig.filterMetrics(isMyMetric);
    CPPUNIT_ASSERT_EQUAL((size_t)0, policyConfig._policyItemVec.size());
}

RA_END_NAMESPACE(config);
