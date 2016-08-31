#ifndef RA_REDALERTCONFIGTEST_H
#define RA_REDALERTCONFIGTEST_H

#include <ra/common/Common.h>
#include <ra/common/Log.h>
#include <ra/config/RedAlertConfig.h>
#include <cppunit/extensions/HelperMacros.h>

RA_BEGIN_NAMESPACE(config);

class RedAlertConfigTest : public CppUnit::TestFixture {
    CPPUNIT_TEST_SUITE(RedAlertConfigTest);
    CPPUNIT_TEST(testLoadConfig);
    CPPUNIT_TEST(testFilterMetrics);
    CPPUNIT_TEST_SUITE_END();
public:
    RedAlertConfigTest();
    ~RedAlertConfigTest();
public:
    void setUp();
    void tearDown();
    void testLoadConfig();
    void testFilterMetrics();
};

RA_END_NAMESPACE(config);

#endif //RA_REDALERTCONFIGTEST_H
