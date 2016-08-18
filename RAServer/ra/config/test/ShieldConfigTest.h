#ifndef RA_SHIELDCONFIGTEST_H
#define RA_SHIELDCONFIGTEST_H

#include <ra/common/CommonDefine.h>
#include <ra/common/Log.h>
#include <ra/config/ShieldConfig.h>
#include <cppunit/extensions/HelperMacros.h>

RA_BEGIN_NAMESPACE(config);

class ShieldConfigTest : public CppUnit::TestFixture {
    CPPUNIT_TEST_SUITE(ShieldConfigTest);
    CPPUNIT_TEST(testLoadConfig);
    CPPUNIT_TEST(testIsShielded);
    CPPUNIT_TEST(testShieldItem);
    CPPUNIT_TEST_SUITE_END();
public:
    ShieldConfigTest();
    ~ShieldConfigTest();
public:
    void setUp();
    void tearDown();
    void testLoadConfig();

    void testIsShielded();
    void testShieldItem();

private:
    RA_LOG_DECLARE();
};

RA_END_NAMESPACE(config);

#endif //RA_SHIELDCONFIGTEST_H
