#ifndef RA_CONSISTENTHASHTEST_H
#define RA_CONSISTENTHASHTEST_H

#include <ra/common/CommonDefine.h>
#include <ra/common/Log.h>
#include <ra/util/ConsistentHash.h>
#include <cppunit/extensions/HelperMacros.h>

RA_BEGIN_NAMESPACE(util);

class ConsistentHashTest : public CppUnit::TestFixture {
    CPPUNIT_TEST_SUITE(ConsistentHashTest);
    CPPUNIT_TEST(testSimpleProcess);
    CPPUNIT_TEST(testGetService);
    CPPUNIT_TEST_SUITE_END();
public:
    ConsistentHashTest();
    ~ConsistentHashTest();
public:
    void setUp();
    void tearDown();
    void testSimpleProcess();
    void testGetService();
};

RA_END_NAMESPACE(util);

#endif //RA_CONSISTENTHASHTEST_H
