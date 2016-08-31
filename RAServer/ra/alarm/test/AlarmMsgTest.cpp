#include <cppunit/TestAssert.h>
#include <ra/test/test.h>
#include <ra/test_util/TestUtil.h>
#include <ra/test_util/SimpleMetricFetcher.h>
#include <ra/fetcher/MetricNode.h>
#include <ra/alarm/test/AlarmMsgTest.h>

using namespace std;
RA_USE_NAMESPACE(tree);
RA_USE_NAMESPACE(config);
RA_USE_NAMESPACE(fetcher);
RA_USE_NAMESPACE(test_util);

RA_BEGIN_NAMESPACE(alarm);

CPPUNIT_TEST_SUITE_REGISTRATION(AlarmMsgTest);

AlarmMsgTest::AlarmMsgTest() { 
}

AlarmMsgTest::~AlarmMsgTest() { 
}

void AlarmMsgTest::setUp() { 
}

void AlarmMsgTest::tearDown() { 
}

void AlarmMsgTest::testSimpleProcess() { 
}

void AlarmMsgTest::testGetDescription()
{
    RequestPackagePtr requestPackage;
    ProcessPackagePtr processPackage(new ProcessPackage(requestPackage));
    AlarmMsg msg(processPackage, CODE_CRITICAL);
    
    vector<MetricNodePtr> metricDataRoots;
    MetricFetcherPtr simpleFetcher(new SimpleMetricFetcher());
    string ret;

    //no response
    ret = msg.getDescription();
    CPPUNIT_ASSERT_EQUAL(string("there is no response"), ret);

    //all response is NULL
    metricDataRoots.resize(2);
    processPackage->setMetricDataRoots(metricDataRoots);
    ret = msg.getDescription();
    CPPUNIT_ASSERT_EQUAL(string("2 metric data trees:\n#1:[NULL]\n#2:[NULL]\n"), ret);

    //one NULL, one normal
    metricDataRoots[0] = simpleFetcher->allocTree();
    processPackage->setMetricDataRoots(metricDataRoots);
    ret = msg.getDescription();

    //two normal
    metricDataRoots[1] = simpleFetcher->allocTree();
    processPackage->setMetricDataRoots(metricDataRoots);
    ret = msg.getDescription();
}

RA_END_NAMESPACE(alarm);

