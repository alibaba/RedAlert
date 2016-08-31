#ifndef RA_CHECKERMANAGERTEST_H
#define RA_CHECKERMANAGERTEST_H

#include <ra/common/Common.h>
#include <ra/common/Log.h>
#include <ra/process/CheckerManager.h>
#include <cppunit/extensions/HelperMacros.h>

RA_BEGIN_NAMESPACE(process);

class CheckerManagerTest : public CppUnit::TestFixture {
    CPPUNIT_TEST_SUITE(CheckerManagerTest);
    CPPUNIT_TEST(testSimpleProcess);
    CPPUNIT_TEST(testStartStop);
    CPPUNIT_TEST(testPushProcessPackage);
    CPPUNIT_TEST_SUITE_END();
public:
    CheckerManagerTest();
    ~CheckerManagerTest();
public:
    void setUp();
    void tearDown();
    void testSimpleProcess();
    void testStartStop();
    void testPushProcessPackage();
};

RA_END_NAMESPACE(process);

#endif //RA_CHECKERMANAGERTEST_H
