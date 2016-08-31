#ifndef RA_POLICYCONFIGITEMBASETEST_H
#define RA_POLICYCONFIGITEMBASETEST_H

#include <ra/common/Common.h>
#include <ra/common/Log.h>
#include <ra/config/PolicyConfigItemBase.h>
#include <cppunit/extensions/HelperMacros.h>

RA_BEGIN_NAMESPACE(config);

class PolicyConfigItemBaseTest : public CppUnit::TestFixture {
    CPPUNIT_TEST_SUITE(PolicyConfigItemBaseTest);
    CPPUNIT_TEST(testSimpleProcess);
    CPPUNIT_TEST(testPolicyFilterItemJsonize);
    CPPUNIT_TEST(testPolicyFilterItemIsMatched);
    CPPUNIT_TEST(testIsFiltered);
    CPPUNIT_TEST(testJsonize);
    CPPUNIT_TEST_SUITE_END();
public:
    PolicyConfigItemBaseTest();
    ~PolicyConfigItemBaseTest();
public:
    void setUp();
    void tearDown();
    void testSimpleProcess();
    void testPolicyFilterItemJsonize();
    void testPolicyFilterItemIsMatched();
    void testIsFiltered();
    void testJsonize();
};

RA_END_NAMESPACE(config);

#endif //RA_POLICYCONFIGITEMBASETEST_H
