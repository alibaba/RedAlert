#include <ra/alarm/test/AlarmManagerTest.h>
#include <ra/test/test.h>
#include <ra/util/Util.h>
#include <cppunit/TestAssert.h>
#include <ra/test_util/TestUtil.h>

RA_USE_NAMESPACE(util);
RA_USE_NAMESPACE(tree);
RA_USE_NAMESPACE(config);
RA_USE_NAMESPACE(test_util);
using namespace std;
RA_BEGIN_NAMESPACE(alarm);
RA_LOG_SETUP(alarm, AlarmManagerTest);

CPPUNIT_TEST_SUITE_REGISTRATION(AlarmManagerTest);

AlarmManagerTest::AlarmManagerTest() { 
}

AlarmManagerTest::~AlarmManagerTest() { 
}

void AlarmManagerTest::setUp() { 
    RA_LOG(DEBUG, "setUp!");
}

void AlarmManagerTest::tearDown() { 
    RA_LOG(DEBUG, "tearDown!");
}

void AlarmManagerTest::testSimpleProcess() { 
    RA_LOG(DEBUG, "Begin Test!");
}

void AlarmManagerTest::testStartStop()
{
    size_t threadNum = 10;
    size_t queueSize = 8;
    const string& alimonitorUrl = "tcp:addr:port";
    const string& mailUser = "user";
    const string& mailPwd = "pwd";
    const string& mailServer = "mailServer";

    AlarmManager manager;
    CPPUNIT_ASSERT(manager.init(threadNum, queueSize, alimonitorUrl,
            mailUser, mailPwd, mailServer));
    CPPUNIT_ASSERT(manager.start());
    CPPUNIT_ASSERT(!manager.start());
    manager.stop();
    CPPUNIT_ASSERT(manager.start());
}

void AlarmManagerTest::testClearExpiredRecordTime()
{
    AlarmManager manager;
    manager._lastClearExpiredRecordTimeSec = 123;
    AlarmMsgKey key1;
    key1.id = 1;
    key1.metric = "m1";
    manager._lastAlarmTime[key1] = 1;
    AlarmMsgKey key2;
    key2.id = 2;
    key2.metric = "m2";
    manager._lastAlarmTime[key2] = 2;
    AlarmMsgKey key3;
    key3.id = 3;
    key3.metric = "m3";
    manager._lastAlarmTime[key3] = 3;
    
    //no expired
    manager.clearExpiredRecordTime(0);
    CPPUNIT_ASSERT_EQUAL((size_t)3, manager._lastAlarmTime.size());
    CPPUNIT_ASSERT_EQUAL((int64_t)123, manager._lastClearExpiredRecordTimeSec);

    //one expired
    manager.clearExpiredRecordTime(ALARM_RECORD_EXPIRE_TIME_SEC + 2);
    CPPUNIT_ASSERT_EQUAL((size_t)2, manager._lastAlarmTime.size());
    CPPUNIT_ASSERT_EQUAL((size_t)1, manager._lastAlarmTime.count(key2));
    CPPUNIT_ASSERT_EQUAL((int64_t)2, manager._lastAlarmTime[key2]);
    CPPUNIT_ASSERT_EQUAL((size_t)1, manager._lastAlarmTime.count(key3));
    CPPUNIT_ASSERT_EQUAL((int64_t)3, manager._lastAlarmTime[key3]);
    CPPUNIT_ASSERT_EQUAL((int64_t)ALARM_RECORD_EXPIRE_TIME_SEC + 2, 
                         manager._lastClearExpiredRecordTimeSec);
    
    //all expired
    manager.clearExpiredRecordTime(ALARM_RECORD_EXPIRE_TIME_SEC + 10);
    CPPUNIT_ASSERT_EQUAL((size_t)0, manager._lastAlarmTime.size());
    CPPUNIT_ASSERT_EQUAL((int64_t)ALARM_RECORD_EXPIRE_TIME_SEC + 10, 
                         manager._lastClearExpiredRecordTimeSec);
}

void AlarmManagerTest::testPushAlarmMsg()
{
    PolicyConfigItemBasePtr policyItem = TestUtil::generatePolicyItem("metric1", 15, 1, 1);
    policyItem->setGroup("g1");
    policyItem->setMinAlarmInterval(5);
    policyItem->setAlarmLevel("unkownLevel");
    ConfigWrapperPtr config(new ConfigWrapper);
    RequestPackagePtr requestPackage(new RequestPackage(config, policyItem));
    ProcessPackagePtr processPackage(new ProcessPackage(requestPackage));
    AlarmMsgPtr alarmMsg(new AlarmMsg(processPackage, CODE_CRITICAL));

    //not started
    size_t threadNum = 1;
    size_t queueSize = 5;
    const string& alimonitorUrl = "tcp:addr:port";
    const string& mailUser = "user";
    const string& mailPwd = "pwd";
    const string& mailServer = "mailServer";
    AlarmManager manager;
    manager.init(threadNum, queueSize, alimonitorUrl,
                 mailUser, mailPwd, mailServer);
    CPPUNIT_ASSERT(!manager.pushAlarmMessage(alarmMsg));
    
    //clearExpiredRecordTime, push success
    CPPUNIT_ASSERT(manager.start());
    manager._lastClearExpiredRecordTimeSec = 0;
    CPPUNIT_ASSERT(manager.pushAlarmMessage(alarmMsg));
    CPPUNIT_ASSERT(manager._lastClearExpiredRecordTimeSec > 0);
    CPPUNIT_ASSERT_EQUAL((size_t)1, manager._lastAlarmTime.size());
    AlarmMsgKey key;
    key.id = 1;
    key.metric = "metric1";
    CPPUNIT_ASSERT(manager._lastAlarmTime[key] > 0);

    //not clearExpiredRecordTime, restrained
    int64_t lastClearExpiredRecordTimeSec = manager._lastClearExpiredRecordTimeSec;
    int64_t lastAlarmTime = Util::currentTimeInSeconds() + 100;
    manager._lastAlarmTime[key] = lastAlarmTime;
    CPPUNIT_ASSERT(manager.pushAlarmMessage(alarmMsg));
    CPPUNIT_ASSERT_EQUAL(lastClearExpiredRecordTimeSec, 
                         manager._lastClearExpiredRecordTimeSec);
    CPPUNIT_ASSERT_EQUAL((size_t)1, manager._lastAlarmTime.size());
    CPPUNIT_ASSERT_EQUAL(lastAlarmTime, manager._lastAlarmTime[key]);
}

void AlarmManagerTest::testRaiseUcmtAlarm()
{
    PolicyConfigItemBasePtr policyItem = TestUtil::generatePolicyItem("metric1", 15, 1, 1);
    policyItem->setGroup("g1");
    policyItem->setAlarmGroup("sm_pe_api");
    policyItem->setMinAlarmInterval(5);
    policyItem->setAlarmLevel("ucmt");
    ConfigWrapperPtr config(new ConfigWrapper);
    RequestPackagePtr requestPackage(new RequestPackage(config, policyItem));
    ProcessPackagePtr processPackage(new ProcessPackage(requestPackage));
    AlarmMsgPtr alarmMsg(new AlarmMsg(processPackage, CODE_CRITICAL));
    alarmMsg->setAlarmMsgStr("..alarmMessage..");

    size_t threadNum = 1;
    size_t queueSize = 5;
    const string& alimonitorUrl = "tcp:addr:port";
    const string& mailUser = "user";
    const string& mailPwd = "pwd";
    const string& mailServer = "mailServer";
    AlarmManager manager;
    manager.init(threadNum, queueSize, alimonitorUrl,
                 mailUser, mailPwd, mailServer);
    
    manager.raiseUcmtAlarm(alarmMsg);
    // string alarmGroup = "sm_pe_api";
    // string alarmMsgStr = "test..";
    // int32_t code = CODE_OK;
    // bool ret = manager.doUcmtAlarm(alarmGroup, alarmMsgStr, code, "zdd001");
    // CPPUNIT_ASSERT(ret);
}

RA_END_NAMESPACE(alarm);

