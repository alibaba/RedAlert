#include <ra/process/test/TrendPolicyCheckerTest.h>
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
RA_LOG_SETUP(process, TrendPolicyCheckerTest);

CPPUNIT_TEST_SUITE_REGISTRATION(TrendPolicyCheckerTest);

TrendPolicyCheckerTest::TrendPolicyCheckerTest() { 
}

TrendPolicyCheckerTest::~TrendPolicyCheckerTest() { 
}

void TrendPolicyCheckerTest::setUp() { 
    RA_LOG(DEBUG, "setUp!");
}

void TrendPolicyCheckerTest::tearDown() { 
    RA_LOG(DEBUG, "tearDown!");
}

void TrendPolicyCheckerTest::testSimpleProcess() { 
    RA_LOG(DEBUG, "Begin Test!");
}

void TrendPolicyCheckerTest::testInit()
{
    TrendPolicyChecker checker;

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
    policyItem = TestUtil::generatePolicyItem("metric1", 15, 0, 0, PT_TREND);
    requestPackage.reset(new RequestPackage(config, policyItem));
    processPackage.reset(new ProcessPackage(requestPackage));
    CPPUNIT_ASSERT(checker.init(processPackage));
}

void TrendPolicyCheckerTest::testCheckValue()
{
    TrendPolicyChecker checker;
    checker._policyItem = tr1::dynamic_pointer_cast<TrendPolicyItem>(
        TestUtil::generatePolicyItem("s.*.m", 15, 0, 0, PT_TREND));
    checker._policyItem->setAscDiffValue(10.0);
    checker._policyItem->setDescDiffValue(10.0);

    RequestPackagePtr requestPackage;
    ProcessPackagePtr processPackage(new ProcessPackage(requestPackage));
    TrendDataPtr trendData(new TrendData);
    processPackage->setTrendData(trendData);
    trendData->setFactors(0.5, 0.5);
    trendData->setAutoExpireTimeUs(1000);
    checker._processPackage = processPackage;

    //no trend history
    MetricDataItem dataItem;
    dataItem.host = 0;
    dataItem.value = 100;
    dataItem.time = 2000;
    CPPUNIT_ASSERT(checker._abnormalInfo.empty());
    CPPUNIT_ASSERT(checker.checkValue("m1", dataItem));
    
    // normal
    dataItem.host = 0;
    dataItem.value = 105;
    dataItem.time = 2010;
    CPPUNIT_ASSERT(checker.checkValue("m1", dataItem));
    CPPUNIT_ASSERT(checker._abnormalInfo.empty());

    //abnormal
    dataItem.host = 0;
    dataItem.value = 205;
    dataItem.time = 2100;
    CPPUNIT_ASSERT(!checker.checkValue("m1", dataItem));
    CPPUNIT_ASSERT_EQUAL((size_t)1, checker._abnormalInfo.size());
    CPPUNIT_ASSERT_EQUAL((size_t)1, checker._abnormalInfo["m1"].size());
    TrendPolicyChecker::AbnormalVal val =  checker._abnormalInfo["m1"][0];
    CPPUNIT_ASSERT_EQUAL(205.0, val.value);
    CPPUNIT_ASSERT_EQUAL(113.75, val.forecastValue);
    
    //abnormal, but has no trend data expired
    dataItem.host = 0;
    dataItem.value = 505;
    dataItem.time = 5100;
    CPPUNIT_ASSERT(checker.checkValue("m1", dataItem));
    CPPUNIT_ASSERT_EQUAL((size_t)1, checker._abnormalInfo.size());
}

void TrendPolicyCheckerTest::testGetAlarmMsgContent()
{
    PolicyConfigItemBasePtr policyItem = TestUtil::generatePolicyItem(
            "shortcut.success_qps/app_for_sug_xx.success_qps", 15, 0, 0,
            config::PT_TREND);
    policyItem->setGroup("sc_online");
    ConfigWrapperPtr config(new ConfigWrapper);
    string sqlitePath = string(TEST_DATA_PATH) + "/config_wrapper/";
    CPPUNIT_ASSERT(config->loadConfig(sqlitePath, 3, "11.99.88.51:20010"));
    RequestPackagePtr requestPackage(new RequestPackage(config, policyItem));
    ProcessPackagePtr processPackage(new ProcessPackage(requestPackage));

    TrendPolicyChecker checker;
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

    map<uint32_t, TrendPolicyChecker::AbnormalVal> mp;
    TrendPolicyChecker::AbnormalVal val;
    val.value = 1.0;
    val.forecastValue = 1.1;
    mp[0] = val;
    val.value = 2.0;
    val.forecastValue = 2.1;
    mp[10] = val;
    checker._abnormalInfo["shortcut.qps"] = mp;
    CPPUNIT_ASSERT_EQUAL(string("shortcut.qps:[1.00,Forecast:1.10]10.0.0.0"
                                "[2.00,Forecast:2.10];Fetch no valid"
                                " data:shortcut.success_qps/app_for_sug_xx.success_qps:0.0.0.0"),
                         checker.getAlarmMsgContent(currTime));
}

void TrendPolicyCheckerTest::testGetAlarmMsgPrefix()
{
    PolicyConfigItemBasePtr policyItem = TestUtil::generatePolicyItem(
            "shortcut.success_qps/app_for_sug_xx.success_qps", 15, 0, 0,
            config::PT_TREND);
    policyItem->setGroup("sc_online");
    ConfigWrapperPtr config(new ConfigWrapper);
    RequestPackagePtr requestPackage(new RequestPackage(config, policyItem));
    ProcessPackagePtr processPackage(new ProcessPackage(requestPackage));

    TrendPolicyChecker checker;
    CPPUNIT_ASSERT(checker.init(processPackage));
    CPPUNIT_ASSERT_EQUAL(string("[id:0][trend][single]"),
                         checker.getAlarmMsgPrefix());
}

RA_END_NAMESPACE(process);

