#ifndef RA_AVAILABILITYPOLICYITEMTEST_H
#define RA_AVAILABILITYPOLICYITEMTEST_H

#include <ra/common/CommonDefine.h>
#include <ra/common/Log.h>
#include <ra/config/AvailabilityPolicyItem.h>
#include <cppunit/extensions/HelperMacros.h>

RA_BEGIN_NAMESPACE(config);

class AvailabilityPolicyItemTest : public CppUnit::TestFixture
{
    CPPUNIT_TEST_SUITE(AvailabilityPolicyItemTest);
    CPPUNIT_TEST(testSimpleProcess);
    CPPUNIT_TEST_SUITE_END();
public:
    AvailabilityPolicyItemTest();
    ~AvailabilityPolicyItemTest();
public:
    void setUp();
    void tearDown();
    void testSimpleProcess();
};

RA_END_NAMESPACE(config);

#endif //RA_AVAILABILITYPOLICYITEMTEST_H
