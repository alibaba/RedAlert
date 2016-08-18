#ifndef RA_HOSTITEMTEST_H
#define RA_HOSTITEMTEST_H

#include <ra/common/CommonDefine.h>
#include <ra/common/Log.h>
#include <ra/config/HostItem.h>
#include <cppunit/extensions/HelperMacros.h>

RA_BEGIN_NAMESPACE(config);

class HostItemTest : public CppUnit::TestFixture {
    CPPUNIT_TEST_SUITE(HostItemTest);
    CPPUNIT_TEST(testSimpleProcess);
    CPPUNIT_TEST_SUITE_END();
public:
    HostItemTest();
    ~HostItemTest();
public:
    void setUp();
    void tearDown();
    void testSimpleProcess();
private:
    RA_LOG_DECLARE();
};

RA_END_NAMESPACE(config);

#endif //RA_HOSTITEMTEST_H
