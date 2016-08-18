#include <ra/process/test/CyclePolicyCheckerTest.h>
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
RA_LOG_SETUP(process, CyclePolicyCheckerTest);

CPPUNIT_TEST_SUITE_REGISTRATION(CyclePolicyCheckerTest);

CyclePolicyCheckerTest::CyclePolicyCheckerTest() { 
}

CyclePolicyCheckerTest::~CyclePolicyCheckerTest() { 
}

void CyclePolicyCheckerTest::setUp() { 
    RA_LOG(DEBUG, "setUp!");
}

void CyclePolicyCheckerTest::tearDown() { 
    RA_LOG(DEBUG, "tearDown!");
}

void CyclePolicyCheckerTest::testSimpleProcess() { 
    RA_LOG(DEBUG, "Begin Test!");
}

void CyclePolicyCheckerTest::testInit()
{
    CyclePolicyChecker checker;

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
    policyItem = TestUtil::generatePolicyItem("metric1", 15, 0, 0, PT_CYCLE);
    requestPackage.reset(new RequestPackage(config, policyItem));
    processPackage.reset(new ProcessPackage(requestPackage));
    CPPUNIT_ASSERT(checker.init(processPackage));
}

void CyclePolicyCheckerTest::testCheckHostValue()
{
    string metric = "metric";
    MetricDataItem hostData;
    hostData.host = 10000;
    hostData.value = 500;
    hostData.time = 200000;

    vector<NormalizedMetricDataPtr> metricDataVec;
    metricDataVec.push_back(NormalizedMetricDataPtr(new NormalizedMetricData));
    metricDataVec.push_back(NormalizedMetricDataPtr(new NormalizedMetricData));
    metricDataVec.push_back(NormalizedMetricDataPtr(new NormalizedMetricData));
    metricDataVec.push_back(NormalizedMetricDataPtr(new NormalizedMetricData));
    
    CyclePolicyChecker checker;
    checker._policyItem = tr1::dynamic_pointer_cast<CyclePolicyItem>(
        TestUtil::generatePolicyItem("s.*.m", 15, 0, 0, PT_CYCLE));
    checker._policyItem->setAscDiffValue(10.0);
    checker._policyItem->setDescDiffValue(10.0);

    //no history
    CPPUNIT_ASSERT(checker.checkHostVal(metric, hostData, metricDataVec));
    CPPUNIT_ASSERT_EQUAL((size_t)0, checker._abnormalInfo.size());
    
    //all history is not matched
    MetricDataItem h1;
    h1.host = 10000 + 1;
    h1.value = 500;
    h1.time = 200000;
    metricDataVec[1]->addMetricData("metric", h1);
    metricDataVec[2]->addMetricData("metric", h1);
    CPPUNIT_ASSERT(checker.checkHostVal(metric, hostData, metricDataVec));
    CPPUNIT_ASSERT_EQUAL((size_t)0, checker._abnormalInfo.size());

    //has missed and has abnormal
    MetricDataItem h2;
    h2.host = 10000;
    h2.value = 500 + 15;
    h2.time = 200000;
    metricDataVec[2]->addMetricData("metric", h2);
    CPPUNIT_ASSERT(!checker.checkHostVal(metric, hostData, metricDataVec));
    CPPUNIT_ASSERT_EQUAL((size_t)1, checker._abnormalInfo.size());
    CPPUNIT_ASSERT_EQUAL((size_t)1, checker._abnormalInfo["metric"].size());
    CPPUNIT_ASSERT_EQUAL(hostData.value, checker._abnormalInfo["metric"][10000].value);
    CPPUNIT_ASSERT_EQUAL((size_t)1, checker._abnormalInfo["metric"][10000].historyValVec.size());
    CPPUNIT_ASSERT_EQUAL(h2.value, checker._abnormalInfo["metric"][10000].historyValVec[0]);

    //one normal, one abnormal
    checker._abnormalInfo.clear();
    MetricDataItem h3;
    h3.host = 10000;
    h3.value = 500;
    metricDataVec[1]->addMetricData("metric", h3);
    CPPUNIT_ASSERT(checker.checkHostVal(metric, hostData, metricDataVec));
    CPPUNIT_ASSERT_EQUAL((size_t)0, checker._abnormalInfo.size());

    //one normal, two abnormal
    MetricDataItem h4;
    h4.host = 10000;
    h4.value = 500 + 25;
    metricDataVec[3]->addMetricData("metric", h4);
    CPPUNIT_ASSERT(!checker.checkHostVal(metric, hostData, metricDataVec));
    CPPUNIT_ASSERT_EQUAL((size_t)1, checker._abnormalInfo.size());
    CPPUNIT_ASSERT_EQUAL((size_t)1, checker._abnormalInfo["metric"].size());
    CPPUNIT_ASSERT_EQUAL(hostData.value, checker._abnormalInfo["metric"][10000].value);
    CPPUNIT_ASSERT_EQUAL((size_t)3, checker._abnormalInfo["metric"][10000].historyValVec.size());
    CPPUNIT_ASSERT_EQUAL(h3.value, checker._abnormalInfo["metric"][10000].historyValVec[0]);
    CPPUNIT_ASSERT_EQUAL(h2.value, checker._abnormalInfo["metric"][10000].historyValVec[1]);
    CPPUNIT_ASSERT_EQUAL(h4.value, checker._abnormalInfo["metric"][10000].historyValVec[2]);
}

void CyclePolicyCheckerTest::testCheckHostTotalValue()
{
    MetricDataItem hostData;
    ValueListPtr metric(new ValueList());
    TimeListPtr timeList(new TimeList());
    hostData.host = 0;
    hostData.value = 5000;
    hostData.time = 20 * MICROSECONDS;
    hostData.rawValue = metric;
    metric->push_back(5000.0);
    hostData.rawTime = timeList;
    timeList->push_back(20 * MICROSECONDS);

    vector<NormalizedMetricDataPtr> metricDataVec;
    metricDataVec.push_back(NormalizedMetricDataPtr(new NormalizedMetricData));
    metricDataVec.push_back(NormalizedMetricDataPtr(new NormalizedMetricData));
    metricDataVec.push_back(NormalizedMetricDataPtr(new NormalizedMetricData));
    metricDataVec.push_back(NormalizedMetricDataPtr(new NormalizedMetricData));
    
    CyclePolicyChecker checker;
    checker._policyItem = tr1::dynamic_pointer_cast<CyclePolicyItem>(
        TestUtil::generatePolicyItem("s.*.m", 15, 0, 0, PT_CYCLE));
    checker._policyItem->setAscDiffValue(100.0);
    checker._policyItem->setDescDiffValue(100.0);

    //no history
    CPPUNIT_ASSERT(checker.checkHostTotalVal("metric", hostData, metricDataVec));
    CPPUNIT_ASSERT_EQUAL((size_t)0, checker._abnormalInfo.size());
    
    //all history is not matched
    metricDataVec[1]->addMetricData("metric1", hostData);
    metricDataVec[1]->genHostTotalData();
    metricDataVec[2]->addMetricData("metric1", hostData);
    metricDataVec[2]->genHostTotalData();
    CPPUNIT_ASSERT(checker.checkHostTotalVal("metric", hostData, metricDataVec));
    CPPUNIT_ASSERT_EQUAL((size_t)0, checker._abnormalInfo.size());

    //has missed and has abnormal
    MetricDataItem h1;
    ValueListPtr metric1(new ValueList());
    TimeListPtr timeList1(new TimeList());
    h1.host = 10000;
    h1.value = 5000 - 200;
    h1.time = 20 * MICROSECONDS;
    h1.rawValue = metric1;
    metric1->push_back(5000.0 - 200.0);
    h1.rawTime = timeList1;
    timeList1->push_back(20 * MICROSECONDS);
    metricDataVec[2]->addMetricData("metric", h1);
    metricDataVec[2]->genHostTotalData();
    CPPUNIT_ASSERT(!checker.checkHostTotalVal("metric", hostData, metricDataVec));
    CPPUNIT_ASSERT_EQUAL((size_t)1, checker._abnormalInfo.size());
    CPPUNIT_ASSERT_EQUAL((size_t)1, checker._abnormalInfo["metric"].size());
    CPPUNIT_ASSERT_EQUAL(hostData.value, checker._abnormalInfo["metric"][0].value);
    CPPUNIT_ASSERT_EQUAL((size_t)1, checker._abnormalInfo["metric"][0].historyValVec.size());
    CPPUNIT_ASSERT_EQUAL(h1.value, checker._abnormalInfo["metric"][0].historyValVec[0]);

    //one normal, one abnormal
    checker._abnormalInfo.clear();
    MetricDataItem h2;
    h2.host = 10000;
    h2.value = hostData.value + 10;
    ValueListPtr metric2(new ValueList());
    TimeListPtr timeList2(new TimeList());
    h2.rawValue = metric2;
    metric2->push_back(h2.value);
    h2.rawTime = timeList2;
    timeList2->push_back(20 * MICROSECONDS);
    metricDataVec[1]->addMetricData("metric", h2);
    metricDataVec[1]->genHostTotalData();
    CPPUNIT_ASSERT(checker.checkHostTotalVal("metric", hostData, metricDataVec));
    CPPUNIT_ASSERT_EQUAL((size_t)0, checker._abnormalInfo.size());

    //one normal, two abnormal
    MetricDataItem h3;
    h3.host = 10000;
    h3.value = hostData.value + 500;
    ValueListPtr metric3(new ValueList());
    TimeListPtr timeList3(new TimeList());
    h3.rawValue = metric3;
    metric3->push_back(h3.value);
    h3.rawTime = timeList3;
    timeList3->push_back(20 * MICROSECONDS);
    metricDataVec[3]->addMetricData("metric", h3);
    metricDataVec[3]->genHostTotalData();
    CPPUNIT_ASSERT(!checker.checkHostTotalVal("metric", hostData, metricDataVec));
    CPPUNIT_ASSERT_EQUAL((size_t)1, checker._abnormalInfo.size());
    CPPUNIT_ASSERT_EQUAL((size_t)1, checker._abnormalInfo["metric"].size());
    CPPUNIT_ASSERT_EQUAL(hostData.value, checker._abnormalInfo["metric"][0].value);
    CPPUNIT_ASSERT_EQUAL((size_t)3, checker._abnormalInfo["metric"][0].historyValVec.size());
    CPPUNIT_ASSERT_EQUAL(h2.value, checker._abnormalInfo["metric"][0].historyValVec[0]);
    CPPUNIT_ASSERT_EQUAL(h1.value, checker._abnormalInfo["metric"][0].historyValVec[1]);
    CPPUNIT_ASSERT_EQUAL(h3.value, checker._abnormalInfo["metric"][0].historyValVec[2]);
}

void CyclePolicyCheckerTest::testCheckMetricTotalVal()
{
    MetricDataItem hostData;
    hostData.host = 0;
    hostData.value = 5000;
    hostData.time = 20 * MICROSECONDS;
    ValueListPtr metric(new ValueList());
    TimeListPtr timeList(new TimeList());
    metric->push_back(hostData.value);
    hostData.rawTime = timeList;
    timeList->push_back(hostData.time);

    vector<NormalizedMetricDataPtr> metricDataVec;
    metricDataVec.push_back(NormalizedMetricDataPtr(new NormalizedMetricData));
    metricDataVec.push_back(NormalizedMetricDataPtr(new NormalizedMetricData));
    metricDataVec.push_back(NormalizedMetricDataPtr(new NormalizedMetricData));
    metricDataVec.push_back(NormalizedMetricDataPtr(new NormalizedMetricData));
    
    CyclePolicyChecker checker;
    checker._policyItem = tr1::dynamic_pointer_cast<CyclePolicyItem>(
        TestUtil::generatePolicyItem("s.*.m", 15, 0, 0, PT_CYCLE));
    checker._policyItem->setAscDiffValue(100.0);
    checker._policyItem->setDescDiffValue(100.0);

    //no history
    CPPUNIT_ASSERT(checker.checkMetricTotalVal(hostData, metricDataVec));
    CPPUNIT_ASSERT_EQUAL((size_t)0, checker._abnormalInfo.size());
    
    //has missed and has abnormal
    MetricDataItem h1;
    h1.host = 10000;
    h1.value = 5000 - 200;
    h1.time = hostData.time;
    ValueListPtr metric1(new ValueList());
    TimeListPtr timeList1(new TimeList());
    h1.rawValue = metric1;
    metric1->push_back(h1.value);
    h1.rawTime = timeList1;
    timeList1->push_back(h1.time);

    metricDataVec[2]->addMetricData("metric", h1);
    metricDataVec[2]->genMetricTotalVal();
    CPPUNIT_ASSERT(!checker.checkMetricTotalVal(hostData, metricDataVec));
    CPPUNIT_ASSERT_EQUAL((size_t)1, checker._abnormalInfo.size());
    CPPUNIT_ASSERT_EQUAL((size_t)1, checker._abnormalInfo["s.*.m"].size());
    CPPUNIT_ASSERT_EQUAL(hostData.value, checker._abnormalInfo["s.*.m"][0].value);
    CPPUNIT_ASSERT_EQUAL((size_t)1, checker._abnormalInfo["s.*.m"][0].historyValVec.size());
    CPPUNIT_ASSERT_EQUAL(h1.value, checker._abnormalInfo["s.*.m"][0].historyValVec[0]);

    //one normal, one abnormal
    checker._abnormalInfo.clear();
    MetricDataItem h2;
    h2.host = 10000;
    h2.value = hostData.value + 10;
    ValueListPtr metric2(new ValueList());
    TimeListPtr timeList2(new TimeList());
    h2.rawValue = metric2;
    metric2->push_back(h2.value);
    h2.rawTime = timeList2;
    timeList2->push_back(hostData.time);
    metricDataVec[1]->addMetricData("metric", h2);
    metricDataVec[1]->genMetricTotalVal();
    CPPUNIT_ASSERT(checker.checkMetricTotalVal(hostData, metricDataVec));
    CPPUNIT_ASSERT_EQUAL((size_t)0, checker._abnormalInfo.size());

    //one normal, two abnormal
    MetricDataItem h3;
    h3.host = 10000;
    h3.value = hostData.value + 500;
    ValueListPtr metric3(new ValueList());
    TimeListPtr timeList3(new TimeList());
    h3.rawValue = metric3;
    metric3->push_back(h3.value);
    h3.rawTime = timeList3;
    timeList3->push_back(hostData.time);
    metricDataVec[3]->addMetricData("metric", h3);
    metricDataVec[3]->genMetricTotalVal();
    CPPUNIT_ASSERT(!checker.checkMetricTotalVal(hostData, metricDataVec));
    CPPUNIT_ASSERT_EQUAL((size_t)1, checker._abnormalInfo.size());
    CPPUNIT_ASSERT_EQUAL((size_t)1, checker._abnormalInfo["s.*.m"].size());
    CPPUNIT_ASSERT_EQUAL(hostData.value, checker._abnormalInfo["s.*.m"][0].value);
    CPPUNIT_ASSERT_EQUAL((size_t)3, checker._abnormalInfo["s.*.m"][0].historyValVec.size());
    CPPUNIT_ASSERT_EQUAL(h2.value, checker._abnormalInfo["s.*.m"][0].historyValVec[0]);
    CPPUNIT_ASSERT_EQUAL(h1.value, checker._abnormalInfo["s.*.m"][0].historyValVec[1]);
    CPPUNIT_ASSERT_EQUAL(h3.value, checker._abnormalInfo["s.*.m"][0].historyValVec[2]);
}

void CyclePolicyCheckerTest::testGetAlarmMsgContent()
{
    PolicyConfigItemBasePtr policyItem = TestUtil::generatePolicyItem(
            "shortcut.success_qps/app_for_sug_xx.success_qps", 15, 0, 0,
            config::PT_CYCLE);
    policyItem->setGroup("sc_online");
    ConfigWrapperPtr config(new ConfigWrapper);
    string sqlitePath = string(TEST_DATA_PATH) + "/config_wrapper/";
    CPPUNIT_ASSERT(config->loadConfig(sqlitePath, 3, "11.99.88.51:20010"));
    RequestPackagePtr requestPackage(new RequestPackage(config, policyItem));
    ProcessPackagePtr processPackage(new ProcessPackage(requestPackage));

    CyclePolicyChecker checker;
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

    map<uint32_t, CyclePolicyChecker::AbnormalVal> mp;
    CyclePolicyChecker::AbnormalVal val;
    val.value = 1.0;
    val.historyValVec.push_back(1.1);
    val.historyValVec.push_back(1.2);
    mp[0] = val;
    val.value = 2.0;
    val.historyValVec.push_back(2.1);
    val.historyValVec.push_back(2.2);
    mp[10] = val;
    checker._abnormalInfo["shortcut.qps"] = mp;
    CPPUNIT_ASSERT_EQUAL(string("shortcut.qps:[1.00,History:1.1,1.2]10.0.0.0"
                                "[2.00,History:1.1,1.2,2.1,2.2];Fetch no valid"
                                " data:shortcut.success_qps/app_for_sug_xx.success_qps:0.0.0.0"),
                         checker.getAlarmMsgContent(currTime));
}

void CyclePolicyCheckerTest::testGetAlarmMsgPrefix()
{
    PolicyConfigItemBasePtr policyItem = TestUtil::generatePolicyItem(
            "shortcut.success_qps/app_for_sug_xx.success_qps", 15, 0, 0,
            config::PT_CYCLE);
    policyItem->setGroup("sc_online");
    ConfigWrapperPtr config(new ConfigWrapper);
    RequestPackagePtr requestPackage(new RequestPackage(config, policyItem));
    ProcessPackagePtr processPackage(new ProcessPackage(requestPackage));

    CyclePolicyChecker checker;
    CPPUNIT_ASSERT(checker.init(processPackage));
    CPPUNIT_ASSERT_EQUAL(string("[id:0][cycle][single]"),
                         checker.getAlarmMsgPrefix());
}

RA_END_NAMESPACE(process);

