#ifndef RA_ALARMMANAGERTEST_H
#define RA_ALARMMANAGERTEST_H

#include <ra/common/Common.h>
#include <ra/common/Log.h>
#include <ra/alarm/AlarmManager.h>
#include <cppunit/extensions/HelperMacros.h>

RA_BEGIN_NAMESPACE(alarm);

class AlarmManagerTest : public CppUnit::TestFixture {
    CPPUNIT_TEST_SUITE(AlarmManagerTest);
//    CPPUNIT_TEST(testSimpleProcess);
    CPPUNIT_TEST(testStartStop);
//    CPPUNIT_TEST(testClearExpiredRecordTime);
//    CPPUNIT_TEST(testPushAlarmMsg);
//    CPPUNIT_TEST(testRaiseUcmtAlarm);
    CPPUNIT_TEST_SUITE_END();
public:
    AlarmManagerTest();
    ~AlarmManagerTest();
public:
    void setUp();
    void tearDown();
    void testSimpleProcess();
    void testStartStop();
    void testClearExpiredRecordTime();
    void testPushAlarmMsg();
    void testRaiseUcmtAlarm();
};

RA_END_NAMESPACE(alarm);

#endif //RA_ALARMMANAGERTEST_H
