#ifndef RA_BOOTSTRAPCONFIGTEST_H
#define RA_BOOTSTRAPCONFIGTEST_H

#include <ra/common/CommonDefine.h>
#include <ra/common/Log.h>
#include <ra/config/BootstrapConfig.h>
#include <cppunit/extensions/HelperMacros.h>

RA_BEGIN_NAMESPACE(config);

class BootstrapConfigTest : public CppUnit::TestFixture {
    CPPUNIT_TEST_SUITE(BootstrapConfigTest);
    CPPUNIT_TEST(testSimpleProcess);
    CPPUNIT_TEST_SUITE_END();
public:
    BootstrapConfigTest();
    ~BootstrapConfigTest();
public:
    void setUp();
    void tearDown();
    void testSimpleProcess();
private:
    RA_LOG_DECLARE();
};

RA_END_NAMESPACE(config);

#endif //RA_BOOTSTRAPCONFIGTEST_H
