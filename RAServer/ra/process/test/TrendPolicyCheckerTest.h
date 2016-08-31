#ifndef RA_TRENDPOLICYCHECKERTEST_H
#define RA_TRENDPOLICYCHECKERTEST_H

#include <ra/common/Common.h>
#include <ra/common/Log.h>
#include <ra/process/TrendPolicyChecker.h>
#include <cppunit/extensions/HelperMacros.h>

RA_BEGIN_NAMESPACE(process);

class TrendPolicyCheckerTest : public CppUnit::TestFixture {
    CPPUNIT_TEST_SUITE(TrendPolicyCheckerTest);
    CPPUNIT_TEST(testSimpleProcess);
    CPPUNIT_TEST(testInit);
    CPPUNIT_TEST(testCheckValue);
    CPPUNIT_TEST(testGetAlarmMsgContent);
    CPPUNIT_TEST(testGetAlarmMsgPrefix);
    CPPUNIT_TEST_SUITE_END();
public:
    TrendPolicyCheckerTest();
    ~TrendPolicyCheckerTest();
public:
    void setUp();
    void tearDown();
    void testSimpleProcess();
    void testInit();
    void testCheckValue();
    void testGetAlarmMsgContent();
    void testGetAlarmMsgPrefix();
};

RA_END_NAMESPACE(process);

#endif //RA_TRENDPOLICYCHECKERTEST_H
