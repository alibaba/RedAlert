#ifndef RA_SINGULARITYPOLICYCHECKERTEST_H
#define RA_SINGULARITYPOLICYCHECKERTEST_H

#include <ra/common/Common.h>
#include <ra/common/Log.h>
#include <ra/process/SingularityPolicyChecker.h>
#include <cppunit/extensions/HelperMacros.h>

RA_BEGIN_NAMESPACE(process);

class SingularityPolicyCheckerTest : public CppUnit::TestFixture {
    CPPUNIT_TEST_SUITE(SingularityPolicyCheckerTest);
    CPPUNIT_TEST(testSimpleProcess);
    CPPUNIT_TEST(testInit);
    CPPUNIT_TEST(testCheckHostVal);
    CPPUNIT_TEST(testGetAlarmMsgContent);
    CPPUNIT_TEST(testGetAlarmMsgPrefix);
    CPPUNIT_TEST_SUITE_END();
public:
    SingularityPolicyCheckerTest();
    ~SingularityPolicyCheckerTest();
public:
    void setUp();
    void tearDown();
    void testSimpleProcess();
    void testInit();
    void testCheckHostVal();
    void testGetAlarmMsgContent();
    void testGetAlarmMsgPrefix();
};

RA_END_NAMESPACE(process);

#endif //RA_SINGULARITYPOLICYCHECKERTEST_H
