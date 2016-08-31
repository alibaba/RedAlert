#include <ra/process/test/PolicyCheckerTest.h>
#include <ra/test/test.h>
#include <cppunit/TestAssert.h>
#include <ra/test_util/TestUtil.h>
#include <ra/test_util/FakePolicyChecker.h>
#include <ra/util/Util.h>
#include <ra/config/ConfigWrapper.h>
#include <ra/fetcher/MetricNode.h>
#include <ra/fetcher/MetricDataNode.h>
#include <ra/fetcher/MetricTreeUtil.h>

using namespace std;
RA_USE_NAMESPACE(fetcher);
RA_USE_NAMESPACE(tree);
RA_USE_NAMESPACE(util);
RA_USE_NAMESPACE(test_util);
RA_USE_NAMESPACE(config);
RA_USE_NAMESPACE(alarm);

RA_BEGIN_NAMESPACE(process);

CPPUNIT_TEST_SUITE_REGISTRATION(PolicyCheckerTest);

PolicyCheckerTest::PolicyCheckerTest() { 
}

PolicyCheckerTest::~PolicyCheckerTest() { 
}

void PolicyCheckerTest::setUp() { 
}

void PolicyCheckerTest::tearDown() { 
}

std::string PolicyCheckerTest::makeMetricPath(const std::string& serviceName,
                                      const std::string& nodePath,
                                      const std::string& metricName)
{
    return serviceName + "." + nodePath + "." + metricName;
}

void PolicyCheckerTest::testSimpleProcess() { 
}

MetricNodePtr PolicyCheckerTest::generateMetricData(
    const string& serviceName, const string& nodePath, uint32_t host, const string& metric,
    const MetricData::TimeVec& timeVec, const MetricData::ValueVec& valVec) const
{
    MetricNodePtr root(new MetricNode(METRIC_ROOT_NODE_LABEL));
    MetricNodePtr node1(new MetricNode(serviceName));
    MetricNodePtr node2(new MetricNode(nodePath));
    MetricNodePtr node3(new MetricNode(metric));
    MetricDataPtr data(new MetricData());
    MetricNodePtr node4(new MetricDataNode(data));
    root->addChild(node1);
    node1->addChild(node2);
    node2->addChild(node3);
    node3->addChild(node4);
    data->hostname = Util::IPToString(host);
    data->times = MetricData::TimeVecPtr(new MetricData::TimeVec(timeVec));
    data->values = MetricData::ValueVecPtr(new MetricData::ValueVec(valVec));
    return root;
}

MetricPath PolicyCheckerTest::generateMetricPath(
    const string& serviceName, const string& nodePath, uint32_t host, const string& metric,
    const MetricData::TimeVec& timeVec, const MetricData::ValueVec& valVec) const {
    fetcher::MetricPath path(5);
    path[0] = MetricNodePtr(new MetricNode(METRIC_ROOT_NODE_LABEL));
    path[1] = MetricNodePtr(new MetricNode(serviceName));
    path[2] = MetricNodePtr(new MetricNode(nodePath));
    path[3] = MetricNodePtr(new MetricNode(metric));
    MetricDataPtr data(new MetricData());
    path[4] = MetricNodePtr(new MetricDataNode(data));
    data->hostname = Util::IPToString(host);
    data->times = MetricData::TimeVecPtr(new MetricData::TimeVec(timeVec));
    data->values = MetricData::ValueVecPtr(new MetricData::ValueVec(valVec));
    return path;
}

void PolicyCheckerTest::testGenOneNormalizedData()
{
    MetricNodePtr root;
    MetricPath path;
    NormalizedMetricDataPtr normalizedMetricData(new NormalizedMetricData);

    PolicyConfigItemBasePtr policyItem = TestUtil::generatePolicyItem("metric1", 15, 0, 0);
    ConfigWrapperPtr config(new ConfigWrapper);
    RequestPackagePtr requestPackage(new RequestPackage(config, policyItem));
    ProcessPackagePtr processPackage(new ProcessPackage(requestPackage));
    FakePolicyChecker policyChecker;
    CPPUNIT_ASSERT(policyChecker.init(processPackage));

    //empty response
    policyChecker.genNormalizedData(root, normalizedMetricData);
    CPPUNIT_ASSERT_EQUAL((size_t)0, normalizedMetricData->getMetricCount());

    vector<uint64_t> timeVec;
    vector<double> valVec;

    // metric no data
    path = generateMetricPath("s1", "n1", 10000, "m1", timeVec, valVec);
    CPPUNIT_ASSERT(MetricTreeUtil::insertPath(root, path));
    
    // all data is invalid
    timeVec.push_back(10);
    valVec.push_back(INVALID_METRIC_DATA);
    path = generateMetricPath("s2", "n1", 20000, "m1", timeVec, valVec);
    CPPUNIT_ASSERT(MetricTreeUtil::insertPath(root, path));

    //normal
    timeVec.clear();
    timeVec.push_back(10);
    timeVec.push_back(11);
    timeVec.push_back(30);
    valVec.assign(3, 100);
    path = generateMetricPath("s3", "n1", 10000, "m1", timeVec, valVec);
    CPPUNIT_ASSERT(MetricTreeUtil::insertPath(root, path));

    // same data(same metric, same host)
    CPPUNIT_ASSERT(MetricTreeUtil::insertPath(root, path));

    //diff host
    path = generateMetricPath("s3", "n1", 20000, "m1", timeVec, valVec);
    CPPUNIT_ASSERT(MetricTreeUtil::insertPath(root, path));

    //host is filtered
    PolicyFilterItem filterItem;
    filterItem.metric = makeMetricPath("s3", "n1", "m1");
    filterItem.hostSet.insert("10.99.1.1");
    policyItem->_policyFilterVec.push_back(filterItem);
    uint32_t hostIp = 0;
    Util::StringToIP("10.99.1.1", hostIp);
    path = generateMetricPath("s3", "n1", hostIp, "m1", timeVec, valVec);
    CPPUNIT_ASSERT(MetricTreeUtil::insertPath(root, path));

    //diff metric
    path = generateMetricPath("s3", "n1", 10000, "m2", timeVec, valVec);
    CPPUNIT_ASSERT(MetricTreeUtil::insertPath(root, path));

    //no metric
    path = generateMetricPath("s4", "n1", 10000, "m1", timeVec, valVec);
    path.pop_back();
    CPPUNIT_ASSERT(MetricTreeUtil::insertPath(root, path));

    LOG(INFO) << "metric tree:\n" << root->toDebugString();
    policyChecker.genNormalizedData(root, normalizedMetricData);
    CPPUNIT_ASSERT_EQUAL((size_t)2, normalizedMetricData->getMetricCount());
    
    set<MetricDataItem> itemSet;
    string metric = makeMetricPath("s3", "n1", "m1");
    itemSet = normalizedMetricData->getMetricData(metric);
    CPPUNIT_ASSERT(!itemSet.empty());
    CPPUNIT_ASSERT_EQUAL((size_t)2, itemSet.size());
    MetricDataItem hostItem;
    hostItem.host = 10000;
    set<MetricDataItem>::const_iterator it = itemSet.find(hostItem);
    CPPUNIT_ASSERT(it != itemSet.end());
    CPPUNIT_ASSERT_EQUAL((uint64_t)17, it->time);
    CPPUNIT_ASSERT_EQUAL(100.0, it->value);

    hostItem.host = 20000;
    it = itemSet.find(hostItem);
    CPPUNIT_ASSERT(it != itemSet.end());
    CPPUNIT_ASSERT_EQUAL((uint64_t)17, it->time);
    CPPUNIT_ASSERT_EQUAL(100.0, it->value);
    
    metric = makeMetricPath("s3", "n1", "m2");
    itemSet = normalizedMetricData->getMetricData(metric);
    CPPUNIT_ASSERT(!itemSet.empty());
    CPPUNIT_ASSERT_EQUAL((size_t)1, itemSet.size());
    hostItem.host = 10000;
    it = itemSet.find(hostItem);
    CPPUNIT_ASSERT(it != itemSet.end());
    CPPUNIT_ASSERT_EQUAL((uint64_t)17, it->time);
    CPPUNIT_ASSERT_EQUAL(100.0, it->value);

    map<string, set<uint32_t> > &infoMap = policyChecker._fetchNoValidDataMap;
    CPPUNIT_ASSERT_EQUAL((size_t)2, infoMap.size());
    metric = makeMetricPath("s1", "n1", "m1");
    CPPUNIT_ASSERT_EQUAL((size_t)1, infoMap.count(metric));
    CPPUNIT_ASSERT_EQUAL((size_t)1, infoMap[metric].count(10000));
    metric = makeMetricPath("s2", "n1", "m1");
    CPPUNIT_ASSERT_EQUAL((size_t)1, infoMap.count(metric));
    CPPUNIT_ASSERT_EQUAL((size_t)1, infoMap[metric].count(20000));
}

void PolicyCheckerTest::testGenMultiNormalizedData()
{
    PolicyConfigItemBasePtr policyItem = TestUtil::generatePolicyItem("metric1", 15, 0, 0);
    ConfigWrapperPtr config(new ConfigWrapper);
    RequestPackagePtr requestPackage(new RequestPackage(config, policyItem));
    ProcessPackagePtr processPackage(new ProcessPackage(requestPackage));
    vector<MetricNodePtr> dataRoots(3);
    dataRoots[2] = generateMetricData("s3", "n1", 10000, "m2", vector<uint64_t>(2, 2), vector<double>(2, 10.0));
    processPackage->setMetricDataRoots(dataRoots);
    LOG(INFO)<< "metric tree:\n" << dataRoots[2]->toDebugString();

    FakePolicyChecker policyChecker;
    CPPUNIT_ASSERT(policyChecker.init(processPackage));
    vector<NormalizedMetricDataPtr> normalizedMetricDataVec;
    policyChecker.genNormalizedData(normalizedMetricDataVec);
    CPPUNIT_ASSERT_EQUAL((size_t)3, normalizedMetricDataVec.size());
    CPPUNIT_ASSERT(normalizedMetricDataVec[0]->empty());
    CPPUNIT_ASSERT(normalizedMetricDataVec[1]->empty());
    CPPUNIT_ASSERT_FAIL(normalizedMetricDataVec[2]->empty());
}

void PolicyCheckerTest::testCheck()
{
    FakePolicyChecker policyChecker;
    policyChecker.check();
    CPPUNIT_ASSERT_EQUAL(0, policyChecker.totalCheckCount);

    PolicyConfigItemBasePtr policyItem = TestUtil::generatePolicyItem("metric1", 15, 0, 0);
    ConfigWrapperPtr config(new ConfigWrapper);
    RequestPackagePtr requestPackage(new RequestPackage(config, policyItem));
    ProcessPackagePtr processPackage(new ProcessPackage(requestPackage));
    vector<MetricNodePtr> dataRoots(3);
    dataRoots[2] = generateMetricData("s3", "n1", 10000, "m2", vector<uint64_t>(2, 2), vector<double>(2, 10.0));
    processPackage->setMetricDataRoots(dataRoots);
    CPPUNIT_ASSERT(policyChecker.init(processPackage));

    //unkonw check tyep
    policyItem->setCheckType(CT_NONE);
    policyChecker.check();
    CPPUNIT_ASSERT_EQUAL(0, policyChecker.totalCheckCount);
    
    //normal
    policyItem->setCheckType(CT_SINGLE);
    policyChecker.check();
    CPPUNIT_ASSERT_EQUAL(1, policyChecker.totalCheckCount);
    CPPUNIT_ASSERT_EQUAL(1, policyChecker.checkSingleHostCount);

    policyItem->setCheckType(CT_HOST_TOTAL);
    policyChecker.check();
    CPPUNIT_ASSERT_EQUAL(2, policyChecker.totalCheckCount);
    CPPUNIT_ASSERT_EQUAL(1, policyChecker.checkHostTotalCount);

    policyItem->setCheckType(CT_METRIC_TOTAL);
    policyChecker.check();
    CPPUNIT_ASSERT_EQUAL(3, policyChecker.totalCheckCount);
    CPPUNIT_ASSERT_EQUAL(1, policyChecker.checkMetricTotalCount);
}

void PolicyCheckerTest::testCheckSingleHost()
{
    PolicyConfigItemBasePtr policyItem = TestUtil::generatePolicyItem("metric1", 15, 0, 0);
    ConfigWrapperPtr config(new ConfigWrapper);
    RequestPackagePtr requestPackage(new RequestPackage(config, policyItem));
    ProcessPackagePtr processPackage(new ProcessPackage(requestPackage));

    FakePolicyChecker policyChecker;
    CPPUNIT_ASSERT(!policyChecker._hasNoMetric);
    CPPUNIT_ASSERT(policyChecker.init(processPackage));
    vector<NormalizedMetricDataPtr> metricDataVec;
    NormalizedMetricDataPtr d1(new NormalizedMetricData);
    metricDataVec.push_back(d1);
    NormalizedMetricDataPtr d2(new NormalizedMetricData);
    metricDataVec.push_back(d2);
    
    //nothing
    policyChecker.checkSingleHost(metricDataVec);
    CPPUNIT_ASSERT_EQUAL((size_t)0, policyChecker.checkHostValueVec.size());
    CPPUNIT_ASSERT_EQUAL((size_t)0, policyChecker._missedMetric.size());
    CPPUNIT_ASSERT(policyChecker._hasNoMetric);
    
    //all metric is missed
    policyChecker._hasNoMetric = false;
    StringSet metricSet;
    metricSet.insert("m1");
    metricSet.insert("m2");
    requestPackage->swapMetricSet(metricSet);
    policyChecker.checkSingleHost(metricDataVec);
    CPPUNIT_ASSERT_EQUAL((size_t)0, policyChecker.checkHostValueVec.size());
    //singleHost will not set missed metric
    CPPUNIT_ASSERT_EQUAL((size_t)0, policyChecker._missedMetric.size());
    CPPUNIT_ASSERT(!policyChecker._hasNoMetric);
    
    //normal
    policyChecker._fetchNoValidDataMap.clear();
    MetricDataItem item;
    item.host = 1;
    item.time = 10;
    item.value = 100;
    d1->addMetricData("m2", item);
    policyChecker.checkSingleHost(metricDataVec);
    CPPUNIT_ASSERT_EQUAL((size_t)0, policyChecker._missedMetric.size());
    CPPUNIT_ASSERT_EQUAL((size_t)1, policyChecker.checkHostValueVec.size());
    CPPUNIT_ASSERT(item == policyChecker.checkHostValueVec[0]);
}

void PolicyCheckerTest::testHostTotal()
{
    PolicyConfigItemBasePtr policyItem = TestUtil::generatePolicyItem("metric1", 15, 0, 0);
    ConfigWrapperPtr config(new ConfigWrapper);
    RequestPackagePtr requestPackage(new RequestPackage(config, policyItem));
    ProcessPackagePtr processPackage(new ProcessPackage(requestPackage));

    FakePolicyChecker policyChecker;
    CPPUNIT_ASSERT(policyChecker.init(processPackage));
    CPPUNIT_ASSERT(!policyChecker._hasNoMetric);
    vector<NormalizedMetricDataPtr> metricDataVec;
    NormalizedMetricDataPtr d1(new NormalizedMetricData);
    metricDataVec.push_back(d1);
    NormalizedMetricDataPtr d2(new NormalizedMetricData);
    metricDataVec.push_back(d2);
    
    //nothing
    policyChecker.checkHostTotal(metricDataVec);
    CPPUNIT_ASSERT(policyChecker._hasNoMetric);
    CPPUNIT_ASSERT_EQUAL((size_t)0, policyChecker.checkHostValueVec.size());
    CPPUNIT_ASSERT_EQUAL((size_t)0, policyChecker._missedMetric.size());
    
    //all metric is missed
    policyChecker._hasNoMetric = false;
    StringSet metricSet;
    metricSet.insert("m1");
    metricSet.insert("m2");
    requestPackage->swapMetricSet(metricSet);
    policyChecker.checkHostTotal(metricDataVec);
    CPPUNIT_ASSERT(!policyChecker._hasNoMetric);
    CPPUNIT_ASSERT_EQUAL((size_t)0, policyChecker.checkHostValueVec.size());
    CPPUNIT_ASSERT_EQUAL((size_t)2, policyChecker._missedMetric.size());
    CPPUNIT_ASSERT(policyChecker.isMetricMissed("m1"));
    CPPUNIT_ASSERT(policyChecker.isMetricMissed("m2"));
    
    //normal
    policyChecker._fetchNoValidDataMap.clear();
    policyChecker._missedMetric.clear();
    MetricDataItem item;
    item.host = 1;
    item.time = 10;
    item.value = 100;
    ValueListPtr metric(new ValueList());
    TimeListPtr timeList(new TimeList());
    item.rawValue = metric;
    metric->push_back(item.value);
    item.rawTime = timeList;
    timeList->push_back(item.time);
    d1->addMetricData("m2", item);
    item.host = 2;
    d1->addMetricData("m2", item);
    policyChecker.checkHostTotal(metricDataVec);
    CPPUNIT_ASSERT_EQUAL((size_t)1, policyChecker._missedMetric.size());
    CPPUNIT_ASSERT_EQUAL((size_t)1, policyChecker.checkHostTotaltValueVec.size());
    item.host = 0;
    item.time = (10 + 10) / 2;
    item.value = 100 + 100;
    CPPUNIT_ASSERT(item == policyChecker.checkHostTotaltValueVec[0]);
}

void PolicyCheckerTest::testMetricTotal()
{
    PolicyConfigItemBasePtr policyItem = TestUtil::generatePolicyItem("metric1", 15, 0, 0);
    ConfigWrapperPtr config(new ConfigWrapper);
    RequestPackagePtr requestPackage(new RequestPackage(config, policyItem));
    ProcessPackagePtr processPackage(new ProcessPackage(requestPackage));

    FakePolicyChecker policyChecker;
    CPPUNIT_ASSERT(policyChecker.init(processPackage));
    CPPUNIT_ASSERT(!policyChecker._hasNoMetric);
    vector<NormalizedMetricDataPtr> metricDataVec;
    NormalizedMetricDataPtr d1(new NormalizedMetricData);
    metricDataVec.push_back(d1);
    NormalizedMetricDataPtr d2(new NormalizedMetricData);
    metricDataVec.push_back(d2);
    
    //nothing
    policyChecker.checkMetricTotal(metricDataVec);
    CPPUNIT_ASSERT(policyChecker._hasNoMetric);
    CPPUNIT_ASSERT_EQUAL((size_t)0, policyChecker.checkHostValueVec.size());
    CPPUNIT_ASSERT_EQUAL((size_t)0, policyChecker._missedMetric.size());
    
    //all metric is missed
    policyChecker._hasNoMetric = false;
    StringSet metricSet;
    metricSet.insert("m1");
    metricSet.insert("m2");
    metricSet.insert("m3");
    requestPackage->swapMetricSet(metricSet);
    policyChecker.checkMetricTotal(metricDataVec);
    CPPUNIT_ASSERT(!policyChecker._hasNoMetric);
    CPPUNIT_ASSERT_EQUAL((size_t)0, policyChecker.checkHostValueVec.size());
    CPPUNIT_ASSERT_EQUAL((size_t)1, policyChecker._missedMetric.size());
    CPPUNIT_ASSERT(policyChecker.isMetricMissed("metric1"));
    
    //normal
    policyChecker._fetchNoValidDataMap.clear();
    policyChecker._missedMetric.clear();
    MetricDataItem item;
    item.host = 1;
    item.time = 10;
    item.value = 100;
    ValueListPtr metric(new ValueList());
    TimeListPtr timeList(new TimeList());
    item.rawValue = metric;
    metric->push_back(item.value);
    item.rawTime = timeList;
    timeList->push_back(item.time);
    d1->addMetricData("m2", item);
    item.host = 2;
    d1->addMetricData("m2", item);
    d1->addMetricData("m3", item);
    policyChecker.checkMetricTotal(metricDataVec);
    CPPUNIT_ASSERT_EQUAL((size_t)0, policyChecker._missedMetric.size());
    CPPUNIT_ASSERT_EQUAL((size_t)1, policyChecker.checkMetricTotaltValueVec.size());
    item.host = 0;
    item.time = (10 + 10 + 10) / 3;
    item.value = 100 + 100 + 100;
    CPPUNIT_ASSERT(item == policyChecker.checkMetricTotaltValueVec[0]);
}

void PolicyCheckerTest::testRemoveShieldAbnormalInfo()
{
    PolicyConfigItemBasePtr policyItem = TestUtil::generatePolicyItem("metric1", 15, 0, 0);
    policyItem->setGroup("g1");
    ConfigWrapperPtr config(new ConfigWrapper);
    RequestPackagePtr requestPackage(new RequestPackage(config, policyItem));
    ProcessPackagePtr processPackage(new ProcessPackage(requestPackage));

    FakePolicyChecker checker;
    CPPUNIT_ASSERT(checker.init(processPackage)); 
    
    ShieldItem shieldItem;
    shieldItem.group = "g1";
    shieldItem.metric = "s.*.m";
    shieldItem.hostSet.insert("*");
    shieldItem.endTime = 20000;
    config->_shieldConfig._shieldItemVec.push_back(shieldItem);
    
    //no removed
    map<string, map<uint32_t, double> > abnormalInfo;
    abnormalInfo["s1.n1.m1"][1000] = 100;
    checker.removeShieldAbnormalInfo(10000, abnormalInfo);
    CPPUNIT_ASSERT_EQUAL((size_t)1, abnormalInfo.size());
    CPPUNIT_ASSERT_EQUAL(100.0, abnormalInfo["s1.n1.m1"][1000]);
    
    //time not matched
    abnormalInfo["s.n.m"][2000] = 200;
    abnormalInfo["s.n.m"][3000] = 300;
    abnormalInfo["s.n1.m"][4000] = 400;
    checker.removeShieldAbnormalInfo(30000, abnormalInfo);
    CPPUNIT_ASSERT_EQUAL((size_t)3, abnormalInfo.size());
    CPPUNIT_ASSERT_EQUAL(100.0, abnormalInfo["s1.n1.m1"][1000]);
    CPPUNIT_ASSERT_EQUAL((size_t)2, abnormalInfo["s.n.m"].size());
    CPPUNIT_ASSERT_EQUAL(400.0, abnormalInfo["s.n1.m"][4000]);
    
    //time macthed
    checker.removeShieldAbnormalInfo(10000, abnormalInfo);
    CPPUNIT_ASSERT_EQUAL((size_t)1, abnormalInfo.size());
    CPPUNIT_ASSERT_EQUAL(100.0, abnormalInfo["s1.n1.m1"][1000]);
}

void PolicyCheckerTest::testGenAbnormalStr()
{
    map<string, map<uint32_t, double> > abnormalInfo;
    //empty abnormalInfo
    FakePolicyChecker checker;
    string ret;
    checker.genAbnormalStr(abnormalInfo, ret);
    CPPUNIT_ASSERT_EQUAL(string(""), ret);
    
    //normal
    abnormalInfo["m1"][1000] = 1000;
    abnormalInfo["m1"][2000] = 2000;
    abnormalInfo["m2"][3000] = 3000;
    checker.genAbnormalStr(abnormalInfo, ret);
    CPPUNIT_ASSERT_EQUAL(string("m1:232.3.0.0[1000.00]208.7.0.0[2000.00],m2:184.11.0.0[3000.00]"), ret);
}

void PolicyCheckerTest::testGetAlarmMsgContent()
{
    PolicyConfigItemBasePtr policyItem = TestUtil::generatePolicyItem(
            "shortcut.success_qps/app_for_sug_xx.success_qps", 15, 0, 0);
    policyItem->setGroup("sc_online");
    ConfigWrapperPtr config(new ConfigWrapper);
    string sqlitePath = string(TEST_DATA_PATH) + "/config_wrapper/";
    CPPUNIT_ASSERT(config->loadConfig(sqlitePath, 3, "11.99.88.51:20010"));
    RequestPackagePtr requestPackage(new RequestPackage(config, policyItem));
    ProcessPackagePtr processPackage(new ProcessPackage(requestPackage));

    FakePolicyChecker checker;
    CPPUNIT_ASSERT(checker.init(processPackage));
    checker._hasNoMetric = true;
    CPPUNIT_ASSERT_EQUAL(string("shortcut.success_qps/app_for_sug_xx.success_qps"
                                ":has no matched metric"),
                         checker.getAlarmMsgContent(0));

    //shield metric
    policyItem->setGroup("online_tmd");
    policyItem->setMetric("tmd.xxx.sm.cn*s/qps_overmax.wait_qps");
    CPPUNIT_ASSERT(checker.getAlarmMsgContent(0).empty());
    policyItem->setGroup("sc_online");
    policyItem->setMetric("shortcut.success_qps/app_for_sug_xx.success_qps");

    //nothing
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

    checker._fetchNoValidDataMap.clear();
    policyItem->setGroup("online_tmd");
    checker._missedMetric.insert("metric1");
    checker._missedMetric.insert("tmd.xxx.sm.cn*s/qps_overmax.wait_qps");
    checker._missedMetric.insert("metric2");
    CPPUNIT_ASSERT_EQUAL(string("Fetch no valid data:[metric1,metric2]"), checker.getAlarmMsgContent(currTime));
}

RA_END_NAMESPACE(process);

