#ifndef RA_DEVIATIONPOLICYITEMTEST_H
#define RA_DEVIATIONPOLICYITEMTEST_H

#include <ra/common/CommonDefine.h>
#include <ra/common/Log.h>
#include <ra/config/DeviationPolicyItem.h>
#include <cppunit/extensions/HelperMacros.h>

RA_BEGIN_NAMESPACE(config);

class DeviationPolicyItemTest : public CppUnit::TestFixture
{
    CPPUNIT_TEST_SUITE(DeviationPolicyItemTest);
    CPPUNIT_TEST(testSimpleProcess);
    CPPUNIT_TEST(testIsDiffTooMuch);
    CPPUNIT_TEST(testAscValueDiffTooMuch);
    CPPUNIT_TEST(testAscRatioDiffTooMuch);
    CPPUNIT_TEST(testDescRatioDiffTooMuch);
    CPPUNIT_TEST(testDescValueDiffTooMuch);
    CPPUNIT_TEST(testCompatible_010);
    CPPUNIT_TEST_SUITE_END();
public:
    DeviationPolicyItemTest();
    ~DeviationPolicyItemTest();
public:
    void setUp();
    void tearDown();
    void testSimpleProcess();
    void testIsDiffTooMuch();
    void testAscValueDiffTooMuch();
    void testAscRatioDiffTooMuch();
    void testDescRatioDiffTooMuch();
    void testDescValueDiffTooMuch();
    void testCompatible_010();
private:
    RA_LOG_DECLARE();
};

RA_END_NAMESPACE(config);

#endif //RA_DEVIATIONPOLICYITEMTEST_H
