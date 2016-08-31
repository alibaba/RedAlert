#ifndef RA_PROCESSPACKAGETEST_H
#define RA_PROCESSPACKAGETEST_H

#include <ra/common/Common.h>
#include <ra/common/Log.h>
#include <ra/tree/ProcessPackage.h>
#include <cppunit/extensions/HelperMacros.h>

RA_BEGIN_NAMESPACE(tree);

class ProcessPackageTest : public CppUnit::TestFixture {
    CPPUNIT_TEST_SUITE(ProcessPackageTest);
    CPPUNIT_TEST(testSimpleProcess);
    CPPUNIT_TEST_SUITE_END();
public:
    ProcessPackageTest();
    ~ProcessPackageTest();
public:
    void setUp();
    void tearDown();
    void testSimpleProcess();
};

RA_END_NAMESPACE(tree);

#endif //RA_PROCESSPACKAGETEST_H
