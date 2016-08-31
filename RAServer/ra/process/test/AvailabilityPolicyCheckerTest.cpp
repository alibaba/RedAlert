#include <ra/process/test/AvailabilityPolicyCheckerTest.h>
#include <ra/test/test.h>
#include <cppunit/TestAssert.h>
#include <ra/test_util/TestUtil.h>
#include <ra/util/Util.h>
#include <ra/config/ConfigWrapper.h>

using namespace std;
RA_USE_NAMESPACE(test_util);
RA_USE_NAMESPACE(tree);
RA_USE_NAMESPACE(config);
RA_USE_NAMESPACE(util);

RA_BEGIN_NAMESPACE(process);

CPPUNIT_TEST_SUITE_REGISTRATION(AvailabilityPolicyCheckerTest);

AvailabilityPolicyCheckerTest::AvailabilityPolicyCheckerTest() { 
}

AvailabilityPolicyCheckerTest::~AvailabilityPolicyCheckerTest() { 
}

void AvailabilityPolicyCheckerTest::setUp() { 
}

void AvailabilityPolicyCheckerTest::tearDown() { 
}

void AvailabilityPolicyCheckerTest::testSimpleProcess() { 
}

void AvailabilityPolicyCheckerTest::testInit()
{
    AvailabilityPolicyChecker checker;

    //ProcessPackagePtr is null 
    ProcessPackagePtr processPackage;
    CPPUNIT_ASSERT(!checker.init(processPackage));

    //dynamic cast failed
    PolicyConfigItemBasePtr policyItem = TestUtil::generatePolicyItem("metric1", 15, 0, 0, PT_NONE);
    ConfigWrapperPtr config(new ConfigWrapper);
    RequestPackagePtr requestPackage(new RequestPackage(config, policyItem));
    processPackage.reset(new ProcessPackage(requestPackage));
    CPPUNIT_ASSERT(!checker.init(processPackage));

    //normal 
    policyItem = TestUtil::generatePolicyItem("metric1", 15, 0, 0, PT_AVAILABILITY);
    requestPackage.reset(new RequestPackage(config, policyItem));
    processPackage.reset(new ProcessPackage(requestPackage));
    CPPUNIT_ASSERT(checker.init(processPackage));
}

void AvailabilityPolicyCheckerTest::testCheckSingleHost()
{
    AvailabilityPolicyChecker checker;
    checker._policyItem = tr1::dynamic_pointer_cast<AvailabilityPolicyItem>(
        TestUtil::generatePolicyItem("s.*.m", 15, 0, 0, PT_AVAILABILITY));
    checker._policyItem->setMinHostNum(2);
    CPPUNIT_ASSERT(checker._hostCount < 0);
    CPPUNIT_ASSERT(!checker._hasNoMetric);

    PolicyConfigItemBasePtr policyItem = TestUtil::generatePolicyItem("metric1", 15, 0, 0);
    ConfigWrapperPtr config(new ConfigWrapper);
    RequestPackagePtr requestPackage(new RequestPackage(config, policyItem));
    ProcessPackagePtr processPackage(new ProcessPackage(requestPackage));
    checker._processPackage = processPackage;

    vector<NormalizedMetricDataPtr> metricDataVec;
    metricDataVec.push_back(NormalizedMetricDataPtr(new NormalizedMetricData));

    //has no metric
    checker.checkSingleHost(metricDataVec);
    CPPUNIT_ASSERT(checker._hostCount < 0);
    CPPUNIT_ASSERT(checker._hasNoMetric);

    //no host
    checker._hasNoMetric = false;
    StringSet metricSet;
    metricSet.insert("m1");
    requestPackage->swapMetricSet(metricSet);
    checker.checkSingleHost(metricDataVec);
    CPPUNIT_ASSERT_EQUAL(0, checker._hostCount);
    CPPUNIT_ASSERT(!checker._hasNoMetric);

    //one host
    checker._hostCount = -1;
    MetricDataItem h1;
    h1.host = 1;
    h1.value = 0.0;
    metricDataVec[0]->addMetricData("metric", h1);
    CPPUNIT_ASSERT_EQUAL((size_t)1, metricDataVec[0]->getHostCount());
    checker.checkSingleHost(metricDataVec);
    CPPUNIT_ASSERT_EQUAL(1, checker._hostCount);

    //two host
    checker._hostCount = -1;
    h1.host = 2;
    metricDataVec[0]->addMetricData("metric", h1);
    CPPUNIT_ASSERT_EQUAL((size_t)2, metricDataVec[0]->getHostCount());
    checker.checkSingleHost(metricDataVec);
    CPPUNIT_ASSERT(checker._hostCount < 0);
}

void AvailabilityPolicyCheckerTest::testGetAlarmMsgContent()
{
    PolicyConfigItemBasePtr policyItem = TestUtil::generatePolicyItem(
            "shortcut.success_qps/app_for_sug_xx.success_qps", 15, 0, 0,
            config::PT_AVAILABILITY);
    policyItem->setGroup("sc_online");
    ConfigWrapperPtr config(new ConfigWrapper);
    string sqlitePath = string(TEST_DATA_PATH) + "/config_wrapper/";
    CPPUNIT_ASSERT(config->loadConfig(sqlitePath, 3, "11.99.88.51:20010"));
    RequestPackagePtr requestPackage(new RequestPackage(config, policyItem));
    ProcessPackagePtr processPackage(new ProcessPackage(requestPackage));

    AvailabilityPolicyChecker checker;
    CPPUNIT_ASSERT(checker.init(processPackage));
    checker._hasNoMetric = true;
    CPPUNIT_ASSERT_EQUAL(string("shortcut.success_qps/app_for_sug_xx.success_qps"
                                ":has no matched metric"),
                         checker.getAlarmMsgContent(0));

    checker._hasNoMetric = false;
    CPPUNIT_ASSERT_EQUAL(string(""), checker.getAlarmMsgContent(0));

    StringSet metrics;
    metrics.insert("shortcut.success_qps/app_for_sug_xx.success_qps");
    requestPackage->swapMetricSet(metrics);
    set<uint32_t> s1;
    s1.insert(0);
    uint32_t host;
    Util::StringToIP("10.99.1.0", host);
    s1.insert(host);
    checker._fetchNoValidDataMap["shortcut.success_qps/app_for_sug_xx.success_qps"] = s1;
    time_t currTime;
    CPPUNIT_ASSERT(Util::formatTime("2029-01-01 12:00:00", currTime));
    CPPUNIT_ASSERT_EQUAL(string("Fetch no valid data:shortcut.success_qps/app_for_sug_xx.success_qps:0.0.0.0,10.99.1.0"), checker.getAlarmMsgContent(currTime));
    CPPUNIT_ASSERT(Util::formatTime("2000-01-01 12:00:00", currTime));
    CPPUNIT_ASSERT_EQUAL(string("Fetch no valid data:shortcut.success_qps/app_for_sug_xx.success_qps:0.0.0.0"), checker.getAlarmMsgContent(currTime));

    checker._hostCount = 1;
    CPPUNIT_ASSERT_EQUAL(string("shortcut.success_qps/app_for_sug_xx.success_qps:[1];"
                                "Fetch no valid data:shortcut.success_qps/app_for_sug_xx.success_qps:0.0.0.0"),
                         checker.getAlarmMsgContent(currTime));
}

void AvailabilityPolicyCheckerTest::testGetAlarmMsgPrefix()
{
    PolicyConfigItemBasePtr policyItem = TestUtil::generatePolicyItem(
            "shortcut.success_qps/app_for_sug_xx.success_qps", 15, 0, 0,
            config::PT_AVAILABILITY);
    policyItem->setGroup("sc_online");
    ConfigWrapperPtr config(new ConfigWrapper);
    RequestPackagePtr requestPackage(new RequestPackage(config, policyItem));
    ProcessPackagePtr processPackage(new ProcessPackage(requestPackage));

    AvailabilityPolicyChecker checker;
    CPPUNIT_ASSERT(checker.init(processPackage));
    CPPUNIT_ASSERT_EQUAL(string("[id:0][availability][MinHostCount:0]"),
                         checker.getAlarmMsgPrefix());
}

RA_END_NAMESPACE(process);

