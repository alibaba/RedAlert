#include <ra/tree/test/ProcessPackageTest.h>
#include <ra/config/ConfigWrapper.h>
#include <ra/test/test.h>
#include <cppunit/TestAssert.h>

using namespace std;
RA_USE_NAMESPACE(config);

RA_BEGIN_NAMESPACE(tree);

CPPUNIT_TEST_SUITE_REGISTRATION(ProcessPackageTest);

ProcessPackageTest::ProcessPackageTest() { 
}

ProcessPackageTest::~ProcessPackageTest() { 
}

void ProcessPackageTest::setUp() { 
}

void ProcessPackageTest::tearDown() { 
}

void ProcessPackageTest::testSimpleProcess() { 
    ConfigWrapperPtr config(new ConfigWrapper);
    PolicyConfigItemBasePtr policyItem(new PolicyConfigItemBase);
    policyItem->setFetchInterval(30);
    RequestPackagePtr requestPackage(new RequestPackage(config, policyItem));

    ProcessPackage processPackage(requestPackage);
    CPPUNIT_ASSERT_EQUAL((int64_t)0, processPackage._lastRetrieveTimeUs);
    CPPUNIT_ASSERT(NULL != processPackage._requestPackage);
    
    processPackage.setLastRetrieveTimeUs(10);
    CPPUNIT_ASSERT_EQUAL((int64_t)10, processPackage.getLastRetrieveTimeUs());
    
    set<int64_t> lastRetrieveTimeSet;
    const int32_t testCount = 10;
    for (int32_t i = 0; i < testCount; ++i) {
        int64_t curTimeUs = 10000 * MICROSECONDS;
        processPackage.setRandomLastRetrieveTime(curTimeUs, 
                DEFAULT_LAST_RETRIEVE_TIME_RANDOM_VALUE);
        int64_t t = processPackage.getLastRetrieveTimeUs();
        CPPUNIT_ASSERT(t <= curTimeUs);
        CPPUNIT_ASSERT(t > curTimeUs - min(30, DEFAULT_LAST_RETRIEVE_TIME_RANDOM_VALUE) * MICROSECONDS);
        lastRetrieveTimeSet.insert(t);
    }
    CPPUNIT_ASSERT(lastRetrieveTimeSet.size() > 2);
}

RA_END_NAMESPACE(tree);

