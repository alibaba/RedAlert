#include <cppunit/TestAssert.h>
#include <ra/app/RaApp.h>
#include <ra/util/Util.h>
#include <ra/util/StringHash.h>
#include <ra/test/test.h>
#include <ra/test_util/TestUtil.h>
#include <ra/test_util/SimpleMetricFetcher.h>
#include <ra/test_util/FakeMetricFetcher.h>
#include <ra/fetcher/MetricFetcher.h>
#include <ra/fetcher/MetricTreeUtil.h>
#include <ra/tree/test/TreeManagerTest.h>

RA_USE_NAMESPACE(app);
RA_USE_NAMESPACE(util);
RA_USE_NAMESPACE(config);
RA_USE_NAMESPACE(fetcher);
RA_USE_NAMESPACE(process);
RA_USE_NAMESPACE(test_util);
using namespace std;
RA_BEGIN_NAMESPACE(tree);

CPPUNIT_TEST_SUITE_REGISTRATION(TreeManagerTest);

TreeManagerTest::TreeManagerTest() { 
}

TreeManagerTest::~TreeManagerTest() { 
}

void TreeManagerTest::setUp() { 
}

void TreeManagerTest::tearDown() { 
}

void TreeManagerTest::testSimpleProcess() { 
}

void TreeManagerTest::testStartStop()
{
    RaApp app;
    FetcherManager fetcher;
    TreeManager treeManager;
    int32_t maxRandomLastRetriveTimeValueSec = 23;
    CPPUNIT_ASSERT(!treeManager.start());

    CPPUNIT_ASSERT(!treeManager.init(&app, &fetcher, -1, maxRandomLastRetriveTimeValueSec));
    CPPUNIT_ASSERT(!treeManager.start());

    CPPUNIT_ASSERT(!treeManager.init(NULL, NULL, 60, maxRandomLastRetriveTimeValueSec));
    CPPUNIT_ASSERT(!treeManager.start());

    CPPUNIT_ASSERT(treeManager.init(&app, &fetcher, 60, maxRandomLastRetriveTimeValueSec));
    CPPUNIT_ASSERT(treeManager.start());

    //start twice
    CPPUNIT_ASSERT(!treeManager.start());

    treeManager.stop();
    CPPUNIT_ASSERT(treeManager.start());
}

void TreeManagerTest::testGetMetricTree()
{
    RaApp app;
    FetcherManager fetcher;
    TreeManager treeManager;
    CPPUNIT_ASSERT(treeManager.init(&app, &fetcher, 60, DEFAULT_LAST_RETRIEVE_TIME_RANDOM_VALUE));
    
    StringSet specSet;
    int64_t startTime = 0;
    int64_t endTime = 10;
    ConfigWrapperPtr configPtr(new ConfigWrapper());
    fetcher::MetricRootMap metricRootMap;

    //empty specSet
    bool ok = treeManager.getMetricTree(specSet, startTime, endTime, configPtr, metricRootMap);
    CPPUNIT_ASSERT(ok);
    CPPUNIT_ASSERT(metricRootMap.empty());

    //getServiceList failed
    specSet.insert("spec1");
    ok = treeManager.getMetricTree(specSet, startTime, endTime, configPtr, metricRootMap);
    CPPUNIT_ASSERT_FAIL(ok);

    //normal
    specSet.insert("spec1");
    configPtr->_dataSourceConfig._metricFetcherMap["spec1"] = MetricFetcherPtr(new SimpleMetricFetcher());
    ok = treeManager.getMetricTree(specSet, startTime, endTime, configPtr, metricRootMap);
    CPPUNIT_ASSERT(ok);
    CPPUNIT_ASSERT_EQUAL((size_t)1, metricRootMap.size());
}

void TreeManagerTest::testMergePath()
{
    MetricTree tree;
    MetricPath path1, path2;
    string spec1 = "spec1";
    MetricNodePtr node1(new MetricNode("node1"));
    MetricNodePtr node2(new MetricNode("node2"));
    MetricNodePtr node3(new MetricNode("node3"));
    MetricNodePtr node4(new MetricNode("node4"));

    //empty path
    TreeManager treeManager;
    treeManager.mergePath(path1, spec1, tree);
    CPPUNIT_ASSERT_EQUAL((size_t)0, tree.getTreeSize());

    //one nodePath, two metric
    path1.push_back(node1);
    path1.push_back(node2);
    path1.push_back(node3);
    treeManager.mergePath(path1, spec1, tree);

    path2.assign(path1.begin(), path1.end());
    path2.pop_back();
    path2.push_back(node4);
    treeManager.mergePath(path2, spec1, tree);
    CPPUNIT_ASSERT_EQUAL((size_t)2, tree.getTreeSize());

    StringSet metricSet;
    tree.getMetrics("node1.node2.*", metricSet);
    CPPUNIT_ASSERT_EQUAL((size_t)2, metricSet.size());
    CPPUNIT_ASSERT_EQUAL((size_t)1, metricSet.count("node1.node2.node3"));
    CPPUNIT_ASSERT_EQUAL((size_t)1, metricSet.count("node1.node2.node4"));

    StringSet specSet = tree.getSpecs("node1.node2.node3");
    CPPUNIT_ASSERT_EQUAL((size_t)1, specSet.size());
    CPPUNIT_ASSERT_EQUAL((size_t)1, specSet.count(spec1));
    specSet = tree.getSpecs("node1.node2.node4");
    CPPUNIT_ASSERT_EQUAL((size_t)1, specSet.size());
    CPPUNIT_ASSERT_EQUAL((size_t)1, specSet.count(spec1));

    // two spec, same response
    string spec2 = "spec2";
    treeManager.mergePath(path1, spec2, tree);
    treeManager.mergePath(path2, spec2, tree);
    CPPUNIT_ASSERT_EQUAL((size_t)2, tree.getTreeSize());
    specSet = tree.getSpecs("node1.node2.node3");
    CPPUNIT_ASSERT_EQUAL((size_t)2, specSet.size());
    CPPUNIT_ASSERT_EQUAL((size_t)1, specSet.count(spec1));
    CPPUNIT_ASSERT_EQUAL((size_t)1, specSet.count(spec2));
    specSet = tree.getSpecs("node1.node2.node4");
    CPPUNIT_ASSERT_EQUAL((size_t)2, specSet.size());
    CPPUNIT_ASSERT_EQUAL((size_t)1, specSet.count(spec1));
    CPPUNIT_ASSERT_EQUAL((size_t)1, specSet.count(spec2));
}

void TreeManagerTest::testMergeTree()
{

    //empty specServiceNameSetMap
    MetricTree tree;
    CPPUNIT_ASSERT_EQUAL((size_t)0, tree.getTreeSize());

    //normal
    MetricNodePtr treeRoot;
    generateDefaultTree(tree, treeRoot);
    CPPUNIT_ASSERT_EQUAL((size_t)2, tree.getTreeSize());
    StringSet metricSet;
    tree.getMetrics("*", metricSet);
    CPPUNIT_ASSERT_EQUAL((size_t)2, metricSet.size());
    CPPUNIT_ASSERT_EQUAL((size_t)1, metricSet.count("node1.node2.node3"));
    CPPUNIT_ASSERT_EQUAL((size_t)1, metricSet.count("node1.node2.node4"));
    StringSet specSet = tree.getSpecs("node1.node2.node3");
    CPPUNIT_ASSERT_EQUAL((size_t)2, specSet.size());
    CPPUNIT_ASSERT_EQUAL((size_t)1, specSet.count("spec1"));
    CPPUNIT_ASSERT_EQUAL((size_t)1, specSet.count("spec2"));
    specSet = tree.getSpecs("node1.node2.node4");
    CPPUNIT_ASSERT_EQUAL((size_t)2, specSet.size());
    CPPUNIT_ASSERT_EQUAL((size_t)1, specSet.count("spec1"));
    CPPUNIT_ASSERT_EQUAL((size_t)1, specSet.count("spec2"));
}

void TreeManagerTest::testIsNeedReloadTree()
{
    RaApp app;
    TreeManager treeManager;
    treeManager._app = &app;
    treeManager._configVersion = 5;
    treeManager._lastReloadTreeTimeUs = 5 * MICROSECONDS;
    treeManager._reloadTreeIntervalSec = 10;

    //no configPtr
    ConfigWrapperPtr configPtr;
    app.setConfigWrapper(configPtr);
    CPPUNIT_ASSERT(!treeManager.needReloadTree(20 * MICROSECONDS));
    
    //config version same, time not expire
    configPtr = ConfigWrapperPtr(new ConfigWrapper);
    app.setConfigWrapper(configPtr);
    configPtr->_configVersion = 5;
    CPPUNIT_ASSERT(!treeManager.needReloadTree(10 * MICROSECONDS));

    //config version not same, time not expire
    configPtr->_configVersion = 6;
    CPPUNIT_ASSERT(treeManager.needReloadTree(10 * MICROSECONDS));

    //config version same, time expire
    configPtr->_configVersion = 5;
    CPPUNIT_ASSERT(treeManager.needReloadTree(20 * MICROSECONDS));

    //config version not same, time expire
    configPtr->_configVersion = 6;
    CPPUNIT_ASSERT(treeManager.needReloadTree(20 * MICROSECONDS));
}

void TreeManagerTest::testGenMetricSubtree()
{
    TreeManager treeManager;
    MetricTree tree;
    MetricNodePtr treeRoot;
    generateDefaultTree(tree, treeRoot);

    //empty metricSet
    {
        StringSet metricSet;
        MetricNodePtr nodePtr = treeManager.genMetricSubtree(tree, metricSet);
        CPPUNIT_ASSERT(NULL == nodePtr);
    }

    //no metric meta
    {
        StringSet metricSet;
        const char *buffer[] = { "node1", "node2", "node999" };
        vector<string> labels(buffer, buffer + sizeof(buffer) / sizeof(char *));
        string metric = Util::joinString(labels, METRIC_NODE_PATH_SEP);
        metricSet.insert(metric);
        MetricNodePtr nodePtr = treeManager.genMetricSubtree(tree, metricSet);
        CPPUNIT_ASSERT(NULL == nodePtr);
    }

    //normal
    {
        StringSet metricSet;
        vector<string> labels;
        const char *buffer1[] = { "node1", "node2", "node3" };
        labels.assign(buffer1, buffer1 + sizeof(buffer1) / sizeof(char *));
        string metric1 = Util::joinString(labels, METRIC_NODE_PATH_SEP);
        metricSet.insert(metric1);
        const char *buffer2[] = { "node1", "node2", "node4" };
        labels.assign(buffer2, buffer2 + sizeof(buffer2) / sizeof(char *));
        string metric2 = Util::joinString(labels, METRIC_NODE_PATH_SEP);
        metricSet.insert(metric2);

        MetricNodePtr metricRoot = treeManager.genMetricSubtree(tree, metricSet);
        CPPUNIT_ASSERT(NULL != metricRoot);
        cout << "original:\n" << treeRoot->toDebugString() << endl;
        cout << "generated:\n" << metricRoot->toDebugString() << endl;
        int cmp = MetricTreeUtil::compareTree(treeRoot, metricRoot);
        CPPUNIT_ASSERT_EQUAL(0, cmp);
    }
}

void TreeManagerTest::testGenOneRequestPackage()
{
    TreeManager treeManager;
    MetricTree tree;
    MetricNodePtr treeRoot;
    generateDefaultTree(tree, treeRoot);
    ConfigWrapperPtr config(new ConfigWrapper);
    PolicyConfigItemBasePtr policyItem = TestUtil::generatePolicyItem("node1.node2.*", 15, 1);

    //normal
    RequestPackagePtr requestPackagePtr = treeManager.genOneRequestPackage(
        config, tree, policyItem);
    CPPUNIT_ASSERT(NULL != requestPackagePtr);

    StringSet metricSet = requestPackagePtr->getMetricSet();
    CPPUNIT_ASSERT_EQUAL((size_t)2, metricSet.size());
    CPPUNIT_ASSERT_EQUAL((size_t)1, metricSet.count("node1.node2.node3"));
    CPPUNIT_ASSERT_EQUAL((size_t)1, metricSet.count("node1.node2.node4"));

    vector<RetrieveTimeRange> retrieveTimeRangeVec;
    requestPackagePtr->getRetrieveTimeRange(retrieveTimeRangeVec);
    CPPUNIT_ASSERT_EQUAL((size_t)1, retrieveTimeRangeVec.size());
    CPPUNIT_ASSERT_EQUAL(-15, retrieveTimeRangeVec[0].start);
    CPPUNIT_ASSERT_EQUAL(0, retrieveTimeRangeVec[0].end);

    StringSet specSet;
    requestPackagePtr->getSpecs(specSet);
    CPPUNIT_ASSERT_EQUAL((size_t)2, specSet.size());
    CPPUNIT_ASSERT_EQUAL((size_t)1, specSet.count("spec1"));
    CPPUNIT_ASSERT_EQUAL((size_t)1, specSet.count("spec2"));
    MetricNodePtr metricRoot = requestPackagePtr->getMetricRoot("spec1");
    int cmp = MetricTreeUtil::compareTree(treeRoot, metricRoot);
    CPPUNIT_ASSERT_EQUAL(0, cmp);
    metricRoot = requestPackagePtr->getMetricRoot("spec2");
    cmp = MetricTreeUtil::compareTree(treeRoot, metricRoot);
    CPPUNIT_ASSERT_EQUAL(0, cmp);

    //all are filtered
    PolicyFilterItem policyFilterItem;
    policyFilterItem.metric = "node1.*";
    policyFilterItem.hostSet.insert("*");
    policyItem->_policyFilterVec.push_back(policyFilterItem);
    requestPackagePtr = treeManager.genOneRequestPackage(config, tree, policyItem);
    CPPUNIT_ASSERT(NULL != requestPackagePtr);
    metricSet = requestPackagePtr->getMetricSet();
    CPPUNIT_ASSERT(metricSet.empty());
    requestPackagePtr->getSpecs(specSet);
    CPPUNIT_ASSERT(specSet.empty());

    //not match any metric of tree
    policyItem->_policyFilterVec.clear();
    policyItem->setMetric("node2.*");
    requestPackagePtr = treeManager.genOneRequestPackage(config, tree, policyItem);
    CPPUNIT_ASSERT(NULL != requestPackagePtr);
    metricSet = requestPackagePtr->getMetricSet();
    CPPUNIT_ASSERT(metricSet.empty());
    requestPackagePtr->getSpecs(specSet);
    CPPUNIT_ASSERT(specSet.empty());
}

void TreeManagerTest::testGenRequestPackages()
{
    TreeManager treeManager;
    MetricTree tree;
    MetricNodePtr treeRoot;
    generateDefaultTree(tree, treeRoot);

    ConfigWrapperPtr config(new ConfigWrapper);
    PolicyConfigItemBasePtr policyItem = TestUtil::generatePolicyItem("node1.node2.node3", 15, 2);
    config->_policyConfig._policyItemVec.push_back(policyItem);
    policyItem = TestUtil::generatePolicyItem("node1.node2.*", 15, 1);
    config->_policyConfig._policyItemVec.push_back(policyItem);

    vector<RequestPackagePtr> requestPackageVec;
    treeManager.genRequestPackages(config, tree, requestPackageVec);
    CPPUNIT_ASSERT_EQUAL((size_t)2, requestPackageVec.size());

    RequestPackagePtr ptr = requestPackageVec[0];
    CPPUNIT_ASSERT(NULL != ptr);
    StringSet metricSet = ptr->getMetricSet();
    CPPUNIT_ASSERT_EQUAL((size_t)1, metricSet.size());
    StringSet specSet;
    ptr->getSpecs(specSet);
    CPPUNIT_ASSERT_EQUAL((size_t)2, specSet.size());

    ptr = requestPackageVec[1];
    CPPUNIT_ASSERT(NULL != ptr);
    metricSet = ptr->getMetricSet();
    CPPUNIT_ASSERT_EQUAL((size_t)2, metricSet.size());
    ptr->getSpecs(specSet);
    CPPUNIT_ASSERT_EQUAL((size_t)2, specSet.size());
}

void TreeManagerTest::testGenProcessPackages()
{
    vector<RequestPackagePtr> requestPackageVec;
    ConfigWrapperPtr config(new ConfigWrapper);
    config->_smoothingFactor = 0.6;
    config->_trendFactor = 0.3;
    PolicyConfigItemBasePtr policyItem = TestUtil::generatePolicyItem("metric1", 100, 1, 0);
    RequestPackagePtr requestPackage(new RequestPackage(config, policyItem));
    requestPackageVec.push_back(requestPackage);
    policyItem = TestUtil::generatePolicyItem("metric2", 15, 1, 2);
    requestPackage.reset(new RequestPackage(config, policyItem));
    requestPackageVec.push_back(requestPackage);
    
    TreeManager treeManager;

    // old processPackageMap is NULL
    TreeManager::ProcessPackageMap processPackageMap;
    treeManager.genProcessPackages(requestPackageVec, processPackageMap);
    CPPUNIT_ASSERT_EQUAL((size_t)2, processPackageMap.size());
    ProcessPackageKey key1;
    key1.id = 0;
    key1.metric = "metric1";
    int64_t lastRetrieveTimeUs = processPackageMap[key1]->getLastRetrieveTimeUs();
    int64_t curTimeUs = Util::currentTimeInMicroseconds();
    CPPUNIT_ASSERT(lastRetrieveTimeUs > 0);
    CPPUNIT_ASSERT(lastRetrieveTimeUs <= curTimeUs);
    TrendDataPtr trendData = processPackageMap[key1]->getTrendData();
    CPPUNIT_ASSERT_EQUAL(0.6, trendData->getSmoothingFactor());
    CPPUNIT_ASSERT_EQUAL(0.3, trendData->getTrendFactor());
    CPPUNIT_ASSERT_EQUAL(MICROSECONDS * 100 * TREND_AUTO_EXPIRE_TIMES,
                         trendData->getAutoExpireTimeUs());
    
    ProcessPackageKey key2;
    key2.id = 2;
    key2.metric = "metric2";
    lastRetrieveTimeUs = processPackageMap[key2]->getLastRetrieveTimeUs();
    CPPUNIT_ASSERT(lastRetrieveTimeUs > 0);
    CPPUNIT_ASSERT(lastRetrieveTimeUs <= curTimeUs);
    trendData = processPackageMap[key2]->getTrendData();
    CPPUNIT_ASSERT_EQUAL(0.6, trendData->getSmoothingFactor());
    CPPUNIT_ASSERT_EQUAL(0.3, trendData->getTrendFactor());
    CPPUNIT_ASSERT_EQUAL(MICROSECONDS * 15 * TREND_AUTO_EXPIRE_TIMES,
                         trendData->getAutoExpireTimeUs());

    // normal
    treeManager._processPackageMap.reset(new TreeManager::ProcessPackageMap());
    TreeManager::ProcessPackageMap &oldProcessPackageMap = *treeManager._processPackageMap;
    oldProcessPackageMap[key1].reset(new ProcessPackage(requestPackage));
    oldProcessPackageMap[key1]->_lastRetrieveTimeUs = 2;
    oldProcessPackageMap[key1]->_trendData.reset(new TrendData);
    oldProcessPackageMap[key2].reset(new ProcessPackage(requestPackage));
    oldProcessPackageMap[key2]->_lastRetrieveTimeUs = 10;
    oldProcessPackageMap[key2]->_trendData.reset(new TrendData);
    TrendKey trendKey;
    trendKey.host = 0;
    trendKey.metric = "m1";
    curTimeUs = Util::currentTimeInMicroseconds();
    oldProcessPackageMap[key2]->_trendData->feedTrend(trendKey, curTimeUs, 0.0);
    CPPUNIT_ASSERT_EQUAL((size_t)1, oldProcessPackageMap[key2]->_trendData->getSize());

    processPackageMap.clear();
    treeManager.genProcessPackages(requestPackageVec, processPackageMap);
    CPPUNIT_ASSERT_EQUAL((size_t)2, processPackageMap.size());
    lastRetrieveTimeUs = processPackageMap[key1]->getLastRetrieveTimeUs();
    CPPUNIT_ASSERT_EQUAL((int64_t)2, lastRetrieveTimeUs);
    trendData = processPackageMap[key1]->getTrendData();
    CPPUNIT_ASSERT_EQUAL(0.6, trendData->getSmoothingFactor());
    CPPUNIT_ASSERT_EQUAL(0.3, trendData->getTrendFactor());
    CPPUNIT_ASSERT_EQUAL(MICROSECONDS * 100 * TREND_AUTO_EXPIRE_TIMES,
                         trendData->getAutoExpireTimeUs());
    CPPUNIT_ASSERT_EQUAL((size_t)0, trendData->getSize());
    
    lastRetrieveTimeUs = processPackageMap[key2]->getLastRetrieveTimeUs();
    CPPUNIT_ASSERT_EQUAL((int64_t)10, lastRetrieveTimeUs);
    trendData = processPackageMap[key2]->getTrendData();
    CPPUNIT_ASSERT_EQUAL(0.6, trendData->getSmoothingFactor());
    CPPUNIT_ASSERT_EQUAL(0.3, trendData->getTrendFactor());
    CPPUNIT_ASSERT_EQUAL(MICROSECONDS * 15 * TREND_AUTO_EXPIRE_TIMES,
                         trendData->getAutoExpireTimeUs());
    CPPUNIT_ASSERT_EQUAL((size_t)1, trendData->getSize());
}

void TreeManagerTest::testWorkLoop()
{
    RaApp app;
    FetcherManager fetcher;
    TreeManager treeManager;
    int64_t reloadTreeIntervalSec = 60;
    CPPUNIT_ASSERT(treeManager.init(&app, &fetcher, reloadTreeIntervalSec, 
            DEFAULT_LAST_RETRIEVE_TIME_RANDOM_VALUE));

    int64_t curTimeUs = 100 * MICROSECONDS;
    CPPUNIT_ASSERT(!treeManager.doWorkLoop(curTimeUs));

    app._configWrapper.reset(new ConfigWrapper());
    app._configWrapper->_configVersion = 5;
    CPPUNIT_ASSERT(treeManager.doWorkLoop(curTimeUs));
    CPPUNIT_ASSERT_EQUAL(5, treeManager._configVersion);
    CPPUNIT_ASSERT_EQUAL((int64_t)100 * MICROSECONDS, treeManager._lastReloadTreeTimeUs);

    
    treeManager._configVersion = 0;
    treeManager._lastReloadTreeTimeUs = 0;
    PolicyConfigItemBasePtr policyItem = TestUtil::generatePolicyItem("node999.*", 5, 1, 66);
    app._configWrapper->_policyConfig._policyItemVec.push_back(policyItem);
    app._configWrapper->_redAlertConfig._containThisServer = true;
    CPPUNIT_ASSERT(treeManager.doWorkLoop(curTimeUs));
    CPPUNIT_ASSERT_EQUAL(5, treeManager._configVersion);
    CPPUNIT_ASSERT_EQUAL((int64_t)100 * MICROSECONDS, treeManager._lastReloadTreeTimeUs);

    treeManager._configVersion = 0;
    treeManager._lastReloadTreeTimeUs = 0;
    app._configWrapper->_dataSourceConfig._metricFetcherMap["spec1"].reset();
    app._configWrapper->_dataSourceConfig._metricFetcherMap["spec2"].reset();
    CPPUNIT_ASSERT_FAIL(treeManager.doWorkLoop(curTimeUs));
    CPPUNIT_ASSERT_EQUAL(0, treeManager._configVersion);
    CPPUNIT_ASSERT_EQUAL((int64_t)0, treeManager._lastReloadTreeTimeUs);

    FakeMetricFetcherPtr fakeFetcher(new FakeMetricFetcher());
    OptionMap options;
    options["tree_depth"] = "3";
    fakeFetcher->init(options);
    app._configWrapper->_dataSourceConfig._metricFetcherMap["spec1"] = fakeFetcher;
    app._configWrapper->_dataSourceConfig._metricFetcherMap["spec2"] = fakeFetcher;
    fakeFetcher->allocOK = false;
    CPPUNIT_ASSERT_FAIL(treeManager.doWorkLoop(curTimeUs));
    CPPUNIT_ASSERT_EQUAL(0, treeManager._configVersion);
    CPPUNIT_ASSERT_EQUAL((int64_t)0, treeManager._lastReloadTreeTimeUs);

    fakeFetcher->allocOK = true;
    CPPUNIT_ASSERT(treeManager.doWorkLoop(curTimeUs));
    CPPUNIT_ASSERT_EQUAL(5, treeManager._configVersion);
    CPPUNIT_ASSERT_EQUAL((int64_t)100 * MICROSECONDS, treeManager._lastReloadTreeTimeUs);

    treeManager._configVersion = 5;
    treeManager._lastReloadTreeTimeUs = 0;
    app._configWrapper->_policyConfig._policyItemVec[0]->setMetric("node1.*.node3");
    policyItem = TestUtil::generatePolicyItem("node1.node2.*", 10, 1, 88);
    app._configWrapper->_policyConfig._policyItemVec.push_back(policyItem);
    uint64_t hashValue = StringHash::hashString64("node1.node2.node3node1.node2.node4");
    treeManager._lastTreeHashValue = hashValue;
    CPPUNIT_ASSERT_FAIL(treeManager.doWorkLoop(curTimeUs));
    CPPUNIT_ASSERT_EQUAL(hashValue, treeManager._lastTreeHashValue);
    CPPUNIT_ASSERT_EQUAL(5, treeManager._configVersion);
    CPPUNIT_ASSERT_EQUAL((int64_t)100 * MICROSECONDS, treeManager._lastReloadTreeTimeUs);
    
    treeManager._lastTreeHashValue = INVALID_HASH_VALUE;
    treeManager._configVersion = 0;
    treeManager._lastReloadTreeTimeUs = 0;
    CPPUNIT_ASSERT(treeManager.doWorkLoop(curTimeUs));
    CPPUNIT_ASSERT_EQUAL(hashValue, treeManager._lastTreeHashValue);
    CPPUNIT_ASSERT_EQUAL(5, treeManager._configVersion);
    CPPUNIT_ASSERT_EQUAL((int64_t)100 * MICROSECONDS, treeManager._lastReloadTreeTimeUs);
    TreeManager::ProcessPackageMapPtr processPackageMapPtr = treeManager.getProcessPackageMap();
    TreeManager::ProcessPackageMap& processPackageMap = *processPackageMapPtr;
    CPPUNIT_ASSERT_EQUAL((size_t)2, processPackageMap.size());

    ProcessPackageKey key;
    key.id = 66;
    key.metric = "node1.*.node3";
    int64_t lastRetrieveTimeUs = processPackageMap[key]->getLastRetrieveTimeUs();
    curTimeUs = Util::currentTimeInMicroseconds();
    CPPUNIT_ASSERT(lastRetrieveTimeUs > 0);
    CPPUNIT_ASSERT(lastRetrieveTimeUs <= curTimeUs);
    const RequestPackagePtr ptr1 = processPackageMap[key]->getRequestPackage();
    StringSet metricSet = ptr1->getMetricSet();
    CPPUNIT_ASSERT_EQUAL((size_t)1, metricSet.size());
    CPPUNIT_ASSERT_EQUAL((size_t)1, metricSet.count("node1.node2.node3"));

    key.id = 88;
    key.metric = "node1.node2.*";
    lastRetrieveTimeUs = processPackageMap[key]->getLastRetrieveTimeUs();
    CPPUNIT_ASSERT(lastRetrieveTimeUs > 0);
    CPPUNIT_ASSERT(lastRetrieveTimeUs <= curTimeUs);
    const RequestPackagePtr ptr2 = processPackageMap[key]->getRequestPackage();
    metricSet = ptr2->getMetricSet();
    CPPUNIT_ASSERT_EQUAL((size_t)2, metricSet.size());
    CPPUNIT_ASSERT_EQUAL((size_t)1, metricSet.count("node1.node2.node3"));
    CPPUNIT_ASSERT_EQUAL((size_t)1, metricSet.count("node1.node2.node4"));
}

void TreeManagerTest::generateDefaultTree(MetricTree& tree, MetricNodePtr &root) const
{
    string spec1 = "spec1";
    string spec2 = "spec2";
    root.reset(new MetricNode(METRIC_ROOT_NODE_LABEL));
    MetricNodePtr node1(new MetricNode("node1"));
    MetricNodePtr node2(new MetricNode("node2"));
    MetricNodePtr node3(new MetricNode("node3"));
    MetricNodePtr node4(new MetricNode("node4"));
    root->addChild(node1);
    node1->addChild(node2);
    node2->addChild(node3);
    node2->addChild(node4);

    MetricRootMap metricRootMap;
    TreeManager treeManager;
    metricRootMap[spec1] = root;
    metricRootMap[spec2] = root;
    treeManager.mergeTree(metricRootMap, tree);
}

RA_END_NAMESPACE(tree);
