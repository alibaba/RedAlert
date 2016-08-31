#include <ra/process/test/ThresholdPolicyCheckerTest.h>
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

CPPUNIT_TEST_SUITE_REGISTRATION(ThresholdPolicyCheckerTest);

ThresholdPolicyCheckerTest::ThresholdPolicyCheckerTest() { 
}

ThresholdPolicyCheckerTest::~ThresholdPolicyCheckerTest() { 
}

void ThresholdPolicyCheckerTest::setUp() { 
}

void ThresholdPolicyCheckerTest::tearDown() { 
}

void ThresholdPolicyCheckerTest::testSimpleProcess() { 
}

void ThresholdPolicyCheckerTest::testInit()
{
    ThresholdPolicyChecker checker;

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
    policyItem = TestUtil::generatePolicyItem("metric1", 15, 0, 0, PT_THRESHOLD);
    requestPackage.reset(new RequestPackage(config, policyItem));
    processPackage.reset(new ProcessPackage(requestPackage));
    CPPUNIT_ASSERT(checker.init(processPackage));
}

void ThresholdPolicyCheckerTest::testCheckValue()
{
    ThresholdPolicyChecker checker;
    checker._policyItem = tr1::dynamic_pointer_cast<ThresholdPolicyItem>(
        TestUtil::generatePolicyItem("s.*.m", 15, 0, 0, PT_THRESHOLD));
    checker._policyItem->_upBound = 50;
    checker._policyItem->_downBound = 5;
    MetricDataItem dataItem;
    CPPUNIT_ASSERT(checker._abnormalInfo.empty());

    //too big
    dataItem.host = 120;
    dataItem.value = 120;
    CPPUNIT_ASSERT(!checker.checkValue("s.1.m", dataItem));
    CPPUNIT_ASSERT_EQUAL((size_t)1, checker._abnormalInfo.size());
    
    //too small
    dataItem.host = 3;
    dataItem.value = 3;
    CPPUNIT_ASSERT(!checker.checkValue("s.1.m", dataItem));
    CPPUNIT_ASSERT(!checker.checkValue("s.2.m", dataItem));
    CPPUNIT_ASSERT_EQUAL((size_t)2, checker._abnormalInfo.size());
    
    //normal
    dataItem.host = 25;
    dataItem.value = 25;
    CPPUNIT_ASSERT(checker.checkValue("s.3.m", dataItem));
    CPPUNIT_ASSERT_EQUAL((size_t)2, checker._abnormalInfo.size());
    
    CPPUNIT_ASSERT_EQUAL((size_t)2, checker._abnormalInfo["s.1.m"].size());
    CPPUNIT_ASSERT_EQUAL((size_t)1, checker._abnormalInfo["s.1.m"].count(120));
    CPPUNIT_ASSERT_EQUAL((size_t)1, checker._abnormalInfo["s.1.m"].count(3));
    
    CPPUNIT_ASSERT_EQUAL((size_t)1, checker._abnormalInfo["s.2.m"].size());
    CPPUNIT_ASSERT_EQUAL((size_t)1, checker._abnormalInfo["s.2.m"].count(3));
}

void ThresholdPolicyCheckerTest::testGetAlarmMsgContent()
{
    PolicyConfigItemBasePtr policyItem = TestUtil::generatePolicyItem(
            "shortcut.success_qps/app_for_sug_xx.success_qps", 15, 0, 0,
            config::PT_THRESHOLD);
    policyItem->setGroup("sc_online");
    ConfigWrapperPtr config(new ConfigWrapper);
    string sqlitePath = string(TEST_DATA_PATH) + "/config_wrapper/";
    CPPUNIT_ASSERT(config->loadConfig(sqlitePath, 3, "11.99.88.51:20010"));
    RequestPackagePtr requestPackage(new RequestPackage(config, policyItem));
    ProcessPackagePtr processPackage(new ProcessPackage(requestPackage));

    ThresholdPolicyChecker checker;
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
    //At 32bit systems sizeof(time_t)=4. 
    CPPUNIT_ASSERT(Util::formatTime("2029-01-01 12:00:00", currTime));
    CPPUNIT_ASSERT_EQUAL(string("Fetch no valid data:shortcut.success_qps/app_for_sug_xx.success_qps:0.0.0.0,10.99.1.0"), checker.getAlarmMsgContent(currTime));
    CPPUNIT_ASSERT(Util::formatTime("2000-01-01 12:00:00", currTime));
    CPPUNIT_ASSERT_EQUAL(string("Fetch no valid data:shortcut.success_qps/app_for_sug_xx.success_qps:0.0.0.0"), checker.getAlarmMsgContent(currTime));

    map<uint32_t, double> mp;
    mp[0] = 1;
    mp[10] = 3;
    checker._abnormalInfo["shortcut.qps"] = mp;
    CPPUNIT_ASSERT_EQUAL(string("shortcut.qps:[1.00]10.0.0.0[3.00];Fetch no valid"
                                " data:shortcut.success_qps/app_for_sug_xx.success_qps:0.0.0.0"),
                         checker.getAlarmMsgContent(currTime));

    checker._fetchNoValidDataMap.clear();
    checker._missedMetric.insert("metric1");
    CPPUNIT_ASSERT_EQUAL(string("shortcut.qps:[1.00]10.0.0.0[3.00];Fetch no valid data:[metric1]"),
                         checker.getAlarmMsgContent(currTime));
}

void ThresholdPolicyCheckerTest::testGetAlarmMsgPrefix()
{
    PolicyConfigItemBasePtr policyItem = TestUtil::generatePolicyItem(
            "shortcut.success_qps/app_for_sug_xx.success_qps", 15, 0, 0,
            config::PT_THRESHOLD);
    policyItem->setGroup("sc_online");
    ConfigWrapperPtr config(new ConfigWrapper);
    RequestPackagePtr requestPackage(new RequestPackage(config, policyItem));
    ProcessPackagePtr processPackage(new ProcessPackage(requestPackage));

    ThresholdPolicyChecker checker;
    CPPUNIT_ASSERT(checker.init(processPackage));
    CPPUNIT_ASSERT_EQUAL(string("[id:0][threshold][single][0.00,0.00]"),
                         checker.getAlarmMsgPrefix());
}

RA_END_NAMESPACE(process);

