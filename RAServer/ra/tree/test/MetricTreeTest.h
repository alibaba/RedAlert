#ifndef RA_METRICTREETEST_H
#define RA_METRICTREETEST_H

#include <ra/common/Common.h>
#include <ra/common/Log.h>
#include <ra/tree/MetricTree.h>
#include <cppunit/extensions/HelperMacros.h>

RA_BEGIN_NAMESPACE(tree);

class MetricTreeTest : public CppUnit::TestFixture {
    CPPUNIT_TEST_SUITE(MetricTreeTest);
    CPPUNIT_TEST(testSimpleProcess);
    CPPUNIT_TEST(testGetHashValue);
    CPPUNIT_TEST_SUITE_END();
public:
    MetricTreeTest();
    ~MetricTreeTest();
public:
    void setUp();
    void tearDown();
    void testSimpleProcess();
    void testGetHashValue();
};

RA_END_NAMESPACE(tree);

#endif //RA_METRICTREETEST_H
