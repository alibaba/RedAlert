#include <ra/process/test/SingularityPolicyCheckerTest.h>
#include <ra/test/test.h>
#include <cppunit/TestAssert.h>
#include <ra/test_util/TestUtil.h>
#include <ra/config/ConfigWrapper.h>
#include <ra/util/Util.h>

using namespace std;
RA_USE_NAMESPACE(test_util);
RA_USE_NAMESPACE(tree);
RA_USE_NAMESPACE(config);
RA_USE_NAMESPACE(util);

RA_BEGIN_NAMESPACE(process);
RA_LOG_SETUP(process, SingularityPolicyCheckerTest);

CPPUNIT_TEST_SUITE_REGISTRATION(SingularityPolicyCheckerTest);

SingularityPolicyCheckerTest::SingularityPolicyCheckerTest() { 
}

SingularityPolicyCheckerTest::~SingularityPolicyCheckerTest() { 
}

void SingularityPolicyCheckerTest::setUp() { 
}

void SingularityPolicyCheckerTest::tearDown() { 
}

void SingularityPolicyCheckerTest::testSimpleProcess() { 
}

void SingularityPolicyCheckerTest::testInit()
{
    SingularityPolicyChecker checker;

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
    policyItem = TestUtil::generatePolicyItem("metric1", 15, 0, 0, PT_SINGULARITY);
    requestPackage.reset(new RequestPackage(config, policyItem));
    processPackage.reset(new ProcessPackage(requestPackage));
    CPPUNIT_ASSERT(checker.init(processPackage));
}

void SingularityPolicyCheckerTest::testCheckHostVal()
{
    SingularityPolicyChecker checker;
    checker._policyItem = tr1::dynamic_pointer_cast<SingularityPolicyItem>(
        TestUtil::generatePolicyItem("s.*.m", 15, 0, 0, PT_SINGULARITY));
    checker._policyItem->setAscDiffValue(10.0);
    checker._policyItem->setDescDiffValue(10.0);
    
    vector<NormalizedMetricDataPtr> metricDataVec;
    metricDataVec.push_back(NormalizedMetricDataPtr(new NormalizedMetricData));
    MetricDataItem h1;
    h1.host = 1000;
    h1.value = 500;
    metricDataVec[0]->addMetricData("metric", h1);
    h1.host = 1000;
    h1.value = 300;
    metricDataVec[0]->addMetricData("metric1", h1);
    CPPUNIT_ASSERT_EQUAL(400.0, metricDataVec[0]->getAvgValue());
    CPPUNIT_ASSERT_EQUAL(INVALID_METRIC_DATA, checker._avgValue);
    
    //normal
    h1.value = 400 + 5;
    CPPUNIT_ASSERT(checker.checkHostVal("m1", h1, metricDataVec));
    CPPUNIT_ASSERT_EQUAL((size_t)0, checker._abnormalInfo.size());

    //abnormal
    h1.value = 400 + 20;
    CPPUNIT_ASSERT(!checker.checkHostVal("m2", h1, metricDataVec));
    CPPUNIT_ASSERT_EQUAL((size_t)1, checker._abnormalInfo.size());
    CPPUNIT_ASSERT_EQUAL((size_t)1, checker._abnormalInfo["m2"].size());
    CPPUNIT_ASSERT_EQUAL(400.0 + 20.0, checker._abnormalInfo["m2"][1000]);
}

void SingularityPolicyCheckerTest::testGetAlarmMsgContent()
{
    PolicyConfigItemBasePtr policyItem = TestUtil::generatePolicyItem(
            "shortcut.success_qps/app_for_sug_xx.success_qps", 15, 0, 0,
            config::PT_SINGULARITY);
    policyItem->setGroup("sc_online");
    ConfigWrapperPtr config(new ConfigWrapper);
    string sqlitePath = string(TEST_DATA_PATH) + "/config_wrapper/";
    CPPUNIT_ASSERT(config->loadConfig(sqlitePath, 3, "11.99.88.51:20010"));
    RequestPackagePtr requestPackage(new RequestPackage(config, policyItem));
    ProcessPackagePtr processPackage(new ProcessPackage(requestPackage));

    SingularityPolicyChecker checker;
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

    map<uint32_t, double> mp;
    mp[0] = 1;
    mp[10] = 3;
    checker._abnormalInfo["shortcut.qps"] = mp;
    checker._avgValue = 999.0;
    CPPUNIT_ASSERT_EQUAL(string("avg:[999.00]shortcut.qps:[1.00]10.0.0.0[3.00];Fetch no valid"
                                " data:shortcut.success_qps/app_for_sug_xx.success_qps:0.0.0.0"),
                         checker.getAlarmMsgContent(currTime));
}

void SingularityPolicyCheckerTest::testGetAlarmMsgPrefix()
{
    PolicyConfigItemBasePtr policyItem = TestUtil::generatePolicyItem(
            "shortcut.success_qps/app_for_sug_xx.success_qps", 15, 0, 0,
            config::PT_SINGULARITY);
    policyItem->setGroup("sc_online");
    ConfigWrapperPtr config(new ConfigWrapper);
    RequestPackagePtr requestPackage(new RequestPackage(config, policyItem));
    ProcessPackagePtr processPackage(new ProcessPackage(requestPackage));

    SingularityPolicyChecker checker;
    CPPUNIT_ASSERT(checker.init(processPackage));
    CPPUNIT_ASSERT_EQUAL(string("[id:0][singularity]"),
                         checker.getAlarmMsgPrefix());
}

RA_END_NAMESPACE(process);

