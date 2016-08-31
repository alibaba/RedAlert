#ifndef RA_CYCLEPOLICYITEMTEST_H
#define RA_CYCLEPOLICYITEMTEST_H

#include <ra/common/CommonDefine.h>
#include <ra/common/Log.h>
#include <ra/config/CyclePolicyItem.h>
#include <cppunit/extensions/HelperMacros.h>

RA_BEGIN_NAMESPACE(config);

class CyclePolicyItemTest : public CppUnit::TestFixture {
    CPPUNIT_TEST_SUITE(CyclePolicyItemTest);
    CPPUNIT_TEST(testSimpleProcess);
    CPPUNIT_TEST(testDefaultCycleTime);
    CPPUNIT_TEST(testDiffTooMuch);    
    CPPUNIT_TEST_SUITE_END();
public:
    CyclePolicyItemTest();
    ~CyclePolicyItemTest();
public:
    void setUp();
    void tearDown();
    void testSimpleProcess();
    void testDefaultCycleTime();
    void testDiffTooMuch();
};

RA_END_NAMESPACE(config);

#endif //RA_CYCLEPOLICYITEMTEST_H
