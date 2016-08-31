#ifndef RA_THRESHOLDPOLICYITEMTEST_H
#define RA_THRESHOLDPOLICYITEMTEST_H

#include <ra/common/CommonDefine.h>
#include <ra/common/Log.h>
#include <ra/config/ThresholdPolicyItem.h>
#include <cppunit/extensions/HelperMacros.h>

RA_BEGIN_NAMESPACE(config);

class ThresholdPolicyItemTest : public CppUnit::TestFixture
{
    CPPUNIT_TEST_SUITE(ThresholdPolicyItemTest);
    CPPUNIT_TEST(testSimpleProcess);
    CPPUNIT_TEST_SUITE_END();
public:
    ThresholdPolicyItemTest();
    ~ThresholdPolicyItemTest();
public:
    void setUp();
    void tearDown();
    void testSimpleProcess();
};

RA_END_NAMESPACE(config);

#endif //RA_THRESHOLDPOLICYITEMTEST_H
