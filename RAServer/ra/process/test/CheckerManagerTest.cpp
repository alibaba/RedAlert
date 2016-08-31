#include <ra/process/test/CheckerManagerTest.h>
#include <ra/test/test.h>
#include <cppunit/TestAssert.h>
#include <ra/alarm/AlarmManager.h>
#include <ra/tree/ProcessPackage.h>
#include <ra/test_util/TestUtil.h>

using namespace std;
RA_USE_NAMESPACE(test_util);
RA_USE_NAMESPACE(tree);
RA_USE_NAMESPACE(alarm);
RA_USE_NAMESPACE(config);

RA_BEGIN_NAMESPACE(process);

CPPUNIT_TEST_SUITE_REGISTRATION(CheckerManagerTest);

CheckerManagerTest::CheckerManagerTest() { 
}

CheckerManagerTest::~CheckerManagerTest() { 
}

void CheckerManagerTest::setUp() { 
}

void CheckerManagerTest::tearDown() { 
}

void CheckerManagerTest::testSimpleProcess() { 
}

void CheckerManagerTest::testStartStop()
{
    AlarmManager alarmManager;
    CheckerManager checkManager;
    CPPUNIT_ASSERT(!checkManager.init(1, 1, NULL));
    CPPUNIT_ASSERT(checkManager.init(1, 1, &alarmManager));
    checkManager._alarmManager = NULL;

    CPPUNIT_ASSERT(!checkManager.start());
    checkManager._alarmManager = &alarmManager;
    CPPUNIT_ASSERT(checkManager.start());
    CPPUNIT_ASSERT(!checkManager.start());
    checkManager.stop();
    CPPUNIT_ASSERT(checkManager.start());
}

void CheckerManagerTest::testPushProcessPackage()
{
    AlarmManager alarmManager;
    CheckerManager checkerManager;
    
    PolicyConfigItemBasePtr policyItem = TestUtil::generatePolicyItem("metric1", 15, 0, 0);
    ConfigWrapperPtr config(new ConfigWrapper);
    RequestPackagePtr requestPackage(new RequestPackage(config, policyItem));
    ProcessPackagePtr ptr(new ProcessPackage(requestPackage));

    //not started
    CPPUNIT_ASSERT(!checkerManager.pushProcessPackage(ptr));

    //init workitem failed
    CPPUNIT_ASSERT(checkerManager.init(1, 1, &alarmManager));
    ProcessPackagePtr processPackage;
    CPPUNIT_ASSERT(!checkerManager.pushProcessPackage(processPackage));
    
    //push workItem failed
    checkerManager._isBlocked = false;
    checkerManager._started = true;
    checkerManager._checkerPool = new util::ThreadPool(2, 1);
    CPPUNIT_ASSERT(checkerManager.pushProcessPackage(ptr));
    CPPUNIT_ASSERT(checkerManager.pushProcessPackage(ptr));
    CPPUNIT_ASSERT(!checkerManager.pushProcessPackage(ptr));

    delete checkerManager._checkerPool;
    checkerManager._checkerPool = NULL;
    checkerManager._started = false;
}

RA_END_NAMESPACE(process);

