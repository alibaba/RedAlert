#ifndef RA_HTTP_SERVER_TEST_H
#define RA_HTTP_SERVER_TEST_H

#include <cppunit/extensions/HelperMacros.h>
#include <ra/common/CommonDefine.h>
#include <ra/common/Log.h>

RA_BEGIN_NAMESPACE(common);

class HttpServerTest: public CppUnit::TestFixture {
    CPPUNIT_TEST_SUITE(HttpServerTest);
    CPPUNIT_TEST(testStartAndStop);
    CPPUNIT_TEST(testRequestHandler);
    CPPUNIT_TEST_SUITE_END();
public:
    HttpServerTest();
    virtual ~HttpServerTest();

    void setUp();
    void tearDown();

    void testStartAndStop();
    void testRequestHandler();

private:
    RA_LOG_DECLARE();
    static void LibeventLogCallback(int severity, const char *msg);
};

RA_END_NAMESPACE(common);

#endif /* RA_HTTP_SERVER_TEST_H */

