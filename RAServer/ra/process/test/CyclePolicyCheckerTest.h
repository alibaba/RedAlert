#ifndef RA_CYCLEPOLICYCHECKERTEST_H
#define RA_CYCLEPOLICYCHECKERTEST_H

#include <ra/common/Common.h>
#include <ra/common/Log.h>
#include <ra/process/CyclePolicyChecker.h>
#include <cppunit/extensions/HelperMacros.h>

RA_BEGIN_NAMESPACE(process);

class CyclePolicyCheckerTest : public CppUnit::TestFixture {
    CPPUNIT_TEST_SUITE(CyclePolicyCheckerTest);
    CPPUNIT_TEST(testSimpleProcess);
    CPPUNIT_TEST(testInit);
    CPPUNIT_TEST(testCheckHostValue);
    CPPUNIT_TEST(testCheckHostTotalValue);
    CPPUNIT_TEST(testCheckMetricTotalVal);
    CPPUNIT_TEST(testGetAlarmMsgContent);
    CPPUNIT_TEST(testGetAlarmMsgPrefix);
    CPPUNIT_TEST_SUITE_END();
public:
    CyclePolicyCheckerTest();
    ~CyclePolicyCheckerTest();
public:
    void setUp();
    void tearDown();
    void testSimpleProcess();
    void testInit();
    void testCheckHostValue();
    void testCheckHostTotalValue();
    void testCheckMetricTotalVal();
    void testGetAlarmMsgContent();
    void testGetAlarmMsgPrefix();
};

RA_END_NAMESPACE(process);

#endif //RA_CYCLEPOLICYCHECKERTEST_H
