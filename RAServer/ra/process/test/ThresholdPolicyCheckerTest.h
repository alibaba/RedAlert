#ifndef RA_THRESHOLDPOLICYCHECKERTEST_H
#define RA_THRESHOLDPOLICYCHECKERTEST_H

#include <ra/common/Common.h>
#include <ra/common/Log.h>
#include <ra/process/ThresholdPolicyChecker.h>
#include <cppunit/extensions/HelperMacros.h>

RA_BEGIN_NAMESPACE(process);

class ThresholdPolicyCheckerTest : public CppUnit::TestFixture {
    CPPUNIT_TEST_SUITE(ThresholdPolicyCheckerTest);
    CPPUNIT_TEST(testSimpleProcess);
    CPPUNIT_TEST(testInit);
    CPPUNIT_TEST(testCheckValue);
    CPPUNIT_TEST(testGetAlarmMsgContent);
    CPPUNIT_TEST(testGetAlarmMsgPrefix);
    CPPUNIT_TEST_SUITE_END();
public:
    ThresholdPolicyCheckerTest();
    ~ThresholdPolicyCheckerTest();
public:
    void setUp();
    void tearDown();
    void testSimpleProcess();
    void testInit();
    void testCheckValue();
    void testGetAlarmMsgContent();
    void testGetAlarmMsgPrefix();
    
private:
    RA_LOG_DECLARE();
};

RA_END_NAMESPACE(process);

#endif //RA_THRESHOLDPOLICYCHECKERTEST_H
