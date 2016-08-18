#include <cppunit/TestAssert.h>
#include <ra/test/test.h>
#include <ra/test_util/TestUtil.h>
#include <ra/test_util/FakeMetricFetcher.h>
#include <ra/alarm/AlarmManager.h>
#include <ra/tree/ProcessPackage.h>
#include <ra/process/FetcherManager.h>
#include <ra/process/test/FetcherManagerTest.h>

using namespace std;
RA_USE_NAMESPACE(tree);
RA_USE_NAMESPACE(alarm);
RA_USE_NAMESPACE(config);
RA_USE_NAMESPACE(fetcher);
RA_USE_NAMESPACE(test_util);

RA_BEGIN_NAMESPACE(process);
RA_LOG_SETUP(process, FetcherManagerTest);

CPPUNIT_TEST_SUITE_REGISTRATION(FetcherManagerTest);

FetcherManagerTest::FetcherManagerTest() { 
}

FetcherManagerTest::~FetcherManagerTest() { 
}

void FetcherManagerTest::setUp() { 
    RA_LOG(DEBUG, "setUp!");
}

void FetcherManagerTest::tearDown() { 
    RA_LOG(DEBUG, "tearDown!");
}

void FetcherManagerTest::testSimpleProcess() { 
    RA_LOG(DEBUG, "Begin Test!");

}

void FetcherManagerTest::testStartStop()
{
    AlarmManager alarmManager;
    CheckerManager checkerManager;

    size_t threadNum = 2;
    size_t queueSize = 10;
    const string& internalAlarmGroup = "internal_group";
    const string& internalAlarmLevel = "alimonitor";
    int32_t interalMinAlarmInterval = 35;
    uint32_t retrieveMetricsPointCount = 5;

    FetcherManager fetcherManager;
    CPPUNIT_ASSERT(!fetcherManager.init(NULL, threadNum, queueSize, 
            internalAlarmGroup, internalAlarmLevel, 
            interalMinAlarmInterval,
            retrieveMetricsPointCount, NULL));
    CPPUNIT_ASSERT(!fetcherManager.start());

    CPPUNIT_ASSERT(!fetcherManager.init(&alarmManager, threadNum, 
            queueSize, internalAlarmGroup, internalAlarmLevel,
            interalMinAlarmInterval,
            retrieveMetricsPointCount, NULL));
    CPPUNIT_ASSERT(!fetcherManager.start());

    CPPUNIT_ASSERT(!fetcherManager.init(&alarmManager, threadNum, 
            queueSize, internalAlarmGroup, internalAlarmLevel,
            interalMinAlarmInterval,
            0, &checkerManager));
    CPPUNIT_ASSERT(!fetcherManager.start());

    CPPUNIT_ASSERT(fetcherManager.init(&alarmManager, threadNum, 
            queueSize, internalAlarmGroup, internalAlarmLevel,
            interalMinAlarmInterval,
            retrieveMetricsPointCount, &checkerManager));
    CPPUNIT_ASSERT(fetcherManager.start());
    CPPUNIT_ASSERT(!fetcherManager.start());

    fetcherManager.stop();
    CPPUNIT_ASSERT(fetcherManager.start());
}

void FetcherManagerTest::testGetMetricTree()
{
    FetcherManager fetcherManager;
    string spec = "spec";
    int64_t start = 0;
    int64_t end = 15;
    FakeMetricFetcherPtr fakeFetcher(new FakeMetricFetcher());
    ConfigWrapperPtr configPtr(new ConfigWrapper);
    configPtr->_configVersion = 15;
    configPtr->_redAlertConfig._serviceName = "test_service";
    configPtr->_dataSourceConfig._metricFetcherMap[spec] = fakeFetcher;

    //get service failed
    MetricNodePtr metricRoot;
    fakeFetcher->allocOK = false;
    CPPUNIT_ASSERT_FAIL(fetcherManager.getMetricTree(spec, start, end, configPtr, metricRoot));
    CPPUNIT_ASSERT(metricRoot == NULL);

    //normal
    fakeFetcher->allocOK = true;
    CPPUNIT_ASSERT(fetcherManager.getMetricTree(spec, start, end, configPtr, metricRoot));
    CPPUNIT_ASSERT(metricRoot != NULL);
}

void FetcherManagerTest::testRetrieveMetrics()
{
    AlarmManager alarmManager;
    CheckerManager checkerManager;
    size_t threadNum = 2;
    size_t queueSize = 10;
    const string& internalAlarmGroup = "internal_group";
    const string& internalAlarmLevel = "alimonitor";
    int32_t interalMinAlarmInterval = 30;
    uint32_t retrieveMetricsPointCount = 5;
    string spec1 = "spec1";
    string spec2 = "spec2";

    FetcherManager fetcherManager;
    CPPUNIT_ASSERT(fetcherManager.init(&alarmManager, threadNum, 
            queueSize, internalAlarmGroup, internalAlarmLevel,
            interalMinAlarmInterval,
            retrieveMetricsPointCount, &checkerManager));

    PolicyConfigItemBasePtr policyItem = TestUtil::generatePolicyItem(METRIC_ROOT_NODE_LABEL, 15, 0, 0);
    FakeMetricFetcherPtr fakeFetcher(new FakeMetricFetcher());
    ConfigWrapperPtr config(new ConfigWrapper);
    config->_configVersion = 15;
    config->_redAlertConfig._serviceName = "test_service";
    config->_dataSourceConfig._metricFetcherMap[spec1] = fakeFetcher;
    config->_dataSourceConfig._metricFetcherMap[spec2] = fakeFetcher;
    RequestPackagePtr requestPackage(new RequestPackage(config, policyItem));
    ProcessPackagePtr processPackage(new ProcessPackage(requestPackage));

    //time range count is 0
    int64_t curTimeUs = 100;
    CPPUNIT_ASSERT_FAIL(fetcherManager.retrieveMetrics(processPackage, curTimeUs));

    //specSet is empty
    policyItem = TestUtil::generatePolicyItem(METRIC_ROOT_NODE_LABEL, 15, 2, 0);
    requestPackage.reset(new RequestPackage(config, policyItem));
    processPackage.reset(new ProcessPackage(requestPackage));
    CPPUNIT_ASSERT(fetcherManager.retrieveMetrics(processPackage, curTimeUs));

    //retrieveMetrics failed
    fakeFetcher->allocOK = true;
    requestPackage->updateMetricRootMap(spec1, fakeFetcher->allocTree(-1, -1));
    requestPackage->updateMetricRootMap(spec2, fakeFetcher->allocTree(-1, -1));
    fakeFetcher->retrieveOK = false;
    CPPUNIT_ASSERT_FAIL(fetcherManager.retrieveMetrics(processPackage, curTimeUs));

    //normal
    fakeFetcher->retrieveOK = true;
    CPPUNIT_ASSERT(fetcherManager.retrieveMetrics(processPackage, curTimeUs));
    vector<MetricNodePtr> dataRoots = processPackage->getMetricDataRoots();
    CPPUNIT_ASSERT(dataRoots.size() > 0U);
}

RA_END_NAMESPACE(process);

