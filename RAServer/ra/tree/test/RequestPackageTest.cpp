#include <cppunit/TestAssert.h>
#include <ra/test/test.h>
#include <ra/fetcher/MetricNode.h>
#include <ra/tree/test/RequestPackageTest.h>
#include <ra/config/ConfigWrapper.h>
#include <ra/config/PolicyConfigItemBase.h>

using namespace std;
RA_USE_NAMESPACE(config);
RA_USE_NAMESPACE(fetcher);

RA_BEGIN_NAMESPACE(tree);

CPPUNIT_TEST_SUITE_REGISTRATION(RequestPackageTest);

RequestPackageTest::RequestPackageTest() { 
}

RequestPackageTest::~RequestPackageTest() { 
}

void RequestPackageTest::setUp() { 
}

void RequestPackageTest::tearDown() { 
}

void RequestPackageTest::testSimpleProcess() { 
    ConfigWrapperPtr config(new ConfigWrapper);
    PolicyConfigItemBasePtr policyItem(new PolicyConfigItemBase);
    RequestPackage requestPackage(config, policyItem);

    policyItem->setMetric("metric");
    CPPUNIT_ASSERT_EQUAL(string("metric"), 
                         requestPackage.getPolicyItem()->getMetric());

    vector<RetrieveTimeRange> retrieveTimeRangeVec;
    requestPackage.getRetrieveTimeRange(retrieveTimeRangeVec);
    CPPUNIT_ASSERT_EQUAL((size_t)0, retrieveTimeRangeVec.size());

    StringSet metricSet = requestPackage.getMetricSet();
    CPPUNIT_ASSERT_EQUAL((size_t)0, metricSet.size());
    metricSet.insert("m1");
    requestPackage.swapMetricSet(metricSet);
    metricSet = requestPackage.getMetricSet();
    CPPUNIT_ASSERT_EQUAL((size_t)1, metricSet.size());
    metricSet = requestPackage.getMetricSet();
    CPPUNIT_ASSERT_EQUAL((size_t)1, metricSet.size());

    MetricNodePtr ptr = requestPackage.getMetricRoot("spec");
    CPPUNIT_ASSERT(NULL == ptr);
    MetricNodePtr rootPtr(new MetricNode(METRIC_ROOT_NODE_LABEL));
    requestPackage.updateMetricRootMap("spec", rootPtr);
    ptr = requestPackage.getMetricRoot("spec");
    CPPUNIT_ASSERT(NULL != ptr);
}

RA_END_NAMESPACE(tree);

