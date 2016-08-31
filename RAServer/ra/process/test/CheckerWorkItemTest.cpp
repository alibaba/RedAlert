#include <ra/process/test/CheckerWorkItemTest.h>
#include <ra/test/test.h>
#include <ra/test_util/TestUtil.h>
#include <ra/util/Util.h>
#include <ra/config/ConfigWrapper.h>
#include <cppunit/TestAssert.h>

using namespace std;
RA_USE_NAMESPACE(tree);
RA_USE_NAMESPACE(util);
RA_USE_NAMESPACE(test_util);
RA_USE_NAMESPACE(config);
RA_USE_NAMESPACE(alarm);

RA_BEGIN_NAMESPACE(process);

CPPUNIT_TEST_SUITE_REGISTRATION(CheckerWorkItemTest);

CheckerWorkItemTest::CheckerWorkItemTest() { 
}

CheckerWorkItemTest::~CheckerWorkItemTest() { 
}

void CheckerWorkItemTest::setUp() { 
}

void CheckerWorkItemTest::tearDown() { 
}

void CheckerWorkItemTest::testSimpleProcess() { 
}

void CheckerWorkItemTest::testInit()
{
    CheckerWorkItem checkerWorkerItem;
    AlarmManager alarmManager;
    ConfigWrapperPtr config(new ConfigWrapper);

    PolicyConfigItemBasePtr policyItem;
    RequestPackagePtr requestPackage;
    ProcessPackagePtr processPackage(new ProcessPackage(requestPackage));

    //param is NULL
    CPPUNIT_ASSERT(!checkerWorkerItem.init(NULL, processPackage));
    ProcessPackagePtr tmp;
    CPPUNIT_ASSERT(!checkerWorkerItem.init(&alarmManager, tmp));

    //questPackage is NULL
    CPPUNIT_ASSERT(!checkerWorkerItem.init(&alarmManager, processPackage));

    //policyConfigitem is NULL
    requestPackage.reset(new RequestPackage(config, policyItem));
    processPackage.reset(new ProcessPackage(requestPackage));
    CPPUNIT_ASSERT(!checkerWorkerItem.init(&alarmManager, processPackage));

    //create policy checker failed
    policyItem = TestUtil::generatePolicyItem("metric1", 15, 0, 0, PT_NONE);
    requestPackage.reset(new RequestPackage(config, policyItem));
    processPackage.reset(new ProcessPackage(requestPackage));
    CPPUNIT_ASSERT(!checkerWorkerItem.init(&alarmManager, processPackage));

    //normal 
    policyItem = TestUtil::generatePolicyItem("metric1", 15, 0, 0, PT_THRESHOLD);
    requestPackage.reset(new RequestPackage(config, policyItem));
    processPackage.reset(new ProcessPackage(requestPackage));
    CPPUNIT_ASSERT(checkerWorkerItem.init(&alarmManager, processPackage));
}


RA_END_NAMESPACE(process);
