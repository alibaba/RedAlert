#ifndef RA_AVAILABILITYPOLICYCHECKERTEST_H
#define RA_AVAILABILITYPOLICYCHECKERTEST_H

#include <ra/common/Common.h>
#include <ra/common/Log.h>
#include <ra/process/AvailabilityPolicyChecker.h>
#include <cppunit/extensions/HelperMacros.h>

RA_BEGIN_NAMESPACE(process);

class AvailabilityPolicyCheckerTest : public CppUnit::TestFixture {
    CPPUNIT_TEST_SUITE(AvailabilityPolicyCheckerTest);
    CPPUNIT_TEST(testSimpleProcess);
    CPPUNIT_TEST(testInit);
    CPPUNIT_TEST(testCheckSingleHost);
    CPPUNIT_TEST(testGetAlarmMsgContent);
    CPPUNIT_TEST(testGetAlarmMsgPrefix);
    CPPUNIT_TEST_SUITE_END();
public:
    AvailabilityPolicyCheckerTest();
    ~AvailabilityPolicyCheckerTest();
public:
    void setUp();
    void tearDown();
    void testSimpleProcess();
    void testInit();
    void testCheckSingleHost();
    void testGetAlarmMsgContent();
    void testGetAlarmMsgPrefix();
private:
    RA_LOG_DECLARE();
};

RA_END_NAMESPACE(process);

#endif //RA_AVAILABILITYPOLICYCHECKERTEST_H
