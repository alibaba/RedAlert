#ifndef RA_REQUESTPACKAGETEST_H
#define RA_REQUESTPACKAGETEST_H

#include <ra/common/Common.h>
#include <ra/common/Log.h>
#include <ra/tree/RequestPackage.h>
#include <cppunit/extensions/HelperMacros.h>

RA_BEGIN_NAMESPACE(tree);

class RequestPackageTest : public CppUnit::TestFixture {
    CPPUNIT_TEST_SUITE(RequestPackageTest);
    CPPUNIT_TEST(testSimpleProcess);
    CPPUNIT_TEST_SUITE_END();
public:
    RequestPackageTest();
    ~RequestPackageTest();
public:
    void setUp();
    void tearDown();
    void testSimpleProcess();
};

RA_END_NAMESPACE(tree);

#endif //RA_REQUESTPACKAGETEST_H
