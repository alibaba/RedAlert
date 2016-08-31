#ifndef RA_POLICYCONFIGTEST_H
#define RA_POLICYCONFIGTEST_H

#include <ra/common/CommonDefine.h>
#include <ra/common/Log.h>
#include <ra/config/PolicyConfig.h>
#include <cppunit/extensions/HelperMacros.h>
#include <ra/util/ConsistentHash.h>

RA_BEGIN_NAMESPACE(config);

class PolicyConfigTest : public CppUnit::TestFixture {
    CPPUNIT_TEST_SUITE(PolicyConfigTest);
    CPPUNIT_TEST(testLoadConfig);
    CPPUNIT_TEST(testFilterMetrics);
    CPPUNIT_TEST_SUITE_END();

public:
    PolicyConfigTest();
    ~PolicyConfigTest();

public:
    void setUp();
    void tearDown();

    void testLoadConfig();
    void testFilterMetrics();
};

RA_END_NAMESPACE(config);

#endif //RA_POLICYCONFIGTEST_H
