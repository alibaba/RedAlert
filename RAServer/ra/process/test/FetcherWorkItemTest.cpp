#include <ra/util/ThreadPool.h>
#include <cppunit/TestAssert.h>
#include <ra/test/test.h>
#include <ra/alarm/AlarmManager.h>
#include <ra/test_util/TestUtil.h>
#include <ra/test_util/FakeMetricFetcher.h>
#include <ra/process/FetcherManager.h>
#include <ra/process/test/FetcherWorkItemTest.h>

RA_USE_NAMESPACE(test_util);
RA_USE_NAMESPACE(config);
RA_USE_NAMESPACE(alarm);
RA_USE_NAMESPACE(tree);
RA_USE_NAMESPACE(util);

RA_BEGIN_NAMESPACE(process);
RA_LOG_SETUP(process, FetcherWorkItemTest);

CPPUNIT_TEST_SUITE_REGISTRATION(FetcherWorkItemTest);

FetcherWorkItemTest::FetcherWorkItemTest() { 
}

FetcherWorkItemTest::~FetcherWorkItemTest() { 
}

void FetcherWorkItemTest::setUp() { 
}

void FetcherWorkItemTest::tearDown() { 
}

void FetcherWorkItemTest::testSimpleProcess() { 
    FetcherManager fetcherManager;
    PolicyConfigItemBasePtr policyItem = TestUtil::generatePolicyItem(METRIC_ROOT_NODE_LABEL, 15, 1, 0);
    ConfigWrapperPtr config(new ConfigWrapper);
    FakeMetricFetcherPtr fakeFetcher(new FakeMetricFetcher());
    config->_dataSourceConfig._metricFetcherMap["spec1"] = fakeFetcher;
    RequestPackagePtr requestPackage(new RequestPackage(config, policyItem));
    ProcessPackagePtr processPackage(new ProcessPackage(requestPackage));
    processPackage->setLastRetrieveTimeUs(10);
    processPackage->setState(ProcessPackage::PPS_PROCESSING);
    AlarmManager alarmManager;

    //queue size is 1
    CheckerManager checkerManager;
    checkerManager.init(2, 1, &alarmManager);
    FetcherWorkItem workerItem(&fetcherManager, processPackage, &checkerManager, &alarmManager);
    CPPUNIT_ASSERT(checkerManager._isBlocked);
    checkerManager._isBlocked = false;
    checkerManager._checkerPool = new ThreadPool(2, 1);
    checkerManager._started = true;

    //retrieveMetrics failed
    fakeFetcher->allocOK = true;
    requestPackage->updateMetricRootMap("spec1", fakeFetcher->allocTree(-1, -1));
    fakeFetcher->retrieveOK = false;
    workerItem.process();
    CPPUNIT_ASSERT_EQUAL((size_t)0, checkerManager._checkerPool->getQueueSize());
    CPPUNIT_ASSERT_EQUAL((int64_t)10, processPackage->getLastRetrieveTimeUs());
    CPPUNIT_ASSERT_EQUAL(ProcessPackage::PPS_IDLE, processPackage->getState());

    //initialize checkerWorkerItem failed
    fakeFetcher->retrieveOK = true;
    policyItem->setTriggerType(PT_NONE);
    processPackage->setLastRetrieveTimeUs((int64_t)10);
    processPackage->setState(ProcessPackage::PPS_PROCESSING);
    workerItem.process();
    CPPUNIT_ASSERT_EQUAL((size_t)0, checkerManager._checkerPool->getQueueSize());
    CPPUNIT_ASSERT((int64_t)10 != processPackage->getLastRetrieveTimeUs());
    CPPUNIT_ASSERT_EQUAL(ProcessPackage::PPS_IDLE, processPackage->getState());

    policyItem->setTriggerType(PT_THRESHOLD);
    CPPUNIT_ASSERT(checkerManager.pushProcessPackage(processPackage));
    CPPUNIT_ASSERT_EQUAL((size_t)1, checkerManager._checkerPool->getQueueSize());

    processPackage->setLastRetrieveTimeUs((int64_t)10);
    processPackage->setState(ProcessPackage::PPS_PROCESSING);
    workerItem.process();
    CPPUNIT_ASSERT_EQUAL((size_t)2, checkerManager._checkerPool->getQueueSize());
    CPPUNIT_ASSERT((int64_t)10 != processPackage->getLastRetrieveTimeUs());
    CPPUNIT_ASSERT_EQUAL(ProcessPackage::PPS_PROCESSING, processPackage->getState());

    //push work item failed, queue size is full
    policyItem->setTriggerType(PT_THRESHOLD);
    processPackage->setLastRetrieveTimeUs((int64_t)10);
    processPackage->setState(ProcessPackage::PPS_PROCESSING);
    workerItem.process();
    CPPUNIT_ASSERT_EQUAL((size_t)2, checkerManager._checkerPool->getQueueSize());
    CPPUNIT_ASSERT((int64_t)10 != processPackage->getLastRetrieveTimeUs());
    CPPUNIT_ASSERT_EQUAL(ProcessPackage::PPS_IDLE, processPackage->getState());

    delete checkerManager._checkerPool;
    checkerManager._checkerPool = NULL;
    checkerManager._started = false;
}

RA_END_NAMESPACE(process);

