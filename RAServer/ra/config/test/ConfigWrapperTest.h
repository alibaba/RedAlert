#ifndef RA_CONFIGWRAPPERTEST_H
#define RA_CONFIGWRAPPERTEST_H

#include <ra/common/Common.h>
#include <ra/common/Log.h>
#include <ra/config/ConfigWrapper.h>
#include <cppunit/extensions/HelperMacros.h>

RA_BEGIN_NAMESPACE(config);

class ConfigWrapperTest : public CppUnit::TestFixture {
    CPPUNIT_TEST_SUITE(ConfigWrapperTest);
    CPPUNIT_TEST(testSimpleProcess);
    CPPUNIT_TEST(testLoadPairVals);
    CPPUNIT_TEST(testLoadConfig);
    CPPUNIT_TEST(testGetPolicyItems);
    CPPUNIT_TEST_SUITE_END();
public:
    ConfigWrapperTest();
    ~ConfigWrapperTest();
public:
    void setUp();
    void tearDown();
    void testSimpleProcess();
    void testLoadPairVals();
    void testLoadConfig();
    void testGetPolicyItems();
};

RA_END_NAMESPACE(config);

#endif //RA_CONFIGWRAPPERTEST_H
