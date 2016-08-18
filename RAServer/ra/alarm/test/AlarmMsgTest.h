#ifndef RA_ALARMMSGTEST_H
#define RA_ALARMMSGTEST_H

#include <ra/common/Common.h>
#include <ra/common/Log.h>
#include <ra/alarm/AlarmMsg.h>
#include <cppunit/extensions/HelperMacros.h>

RA_BEGIN_NAMESPACE(alarm);

class AlarmMsgTest : public CppUnit::TestFixture {
    CPPUNIT_TEST_SUITE(AlarmMsgTest);
    CPPUNIT_TEST(testSimpleProcess);
    CPPUNIT_TEST(testGetDescription);
    CPPUNIT_TEST_SUITE_END();
public:
    AlarmMsgTest();
    ~AlarmMsgTest();
public:
    void setUp();
    void tearDown();
    void testSimpleProcess();
    void testGetDescription();
private:
    RA_LOG_DECLARE();
};

RA_END_NAMESPACE(alarm);

#endif //RA_ALARMMSGTEST_H
