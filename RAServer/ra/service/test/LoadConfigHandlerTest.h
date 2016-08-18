#ifndef RA_LOAD_CONFIG_HANDLER_TEST_H
#define RA_LOAD_CONFIG_HANDLER_TEST_H

#include <cppunit/extensions/HelperMacros.h>
#include <ra/common/CommonDefine.h>
#include <ra/common/Log.h>

RA_BEGIN_NAMESPACE(service);

class LoadConfigHandlerTest: public CppUnit::TestFixture {
    CPPUNIT_TEST_SUITE(LoadConfigHandlerTest);
    CPPUNIT_TEST(testLoadConfig);
    CPPUNIT_TEST_SUITE_END();
public:
    LoadConfigHandlerTest();
    virtual ~LoadConfigHandlerTest();

    void setUp();
    void tearDown();

    void testLoadConfig();

private:
    RA_LOG_DECLARE();
};

RA_END_NAMESPACE(service);

#endif /* RA_LOAD_CONFIG_HANDLER_TEST_H */

