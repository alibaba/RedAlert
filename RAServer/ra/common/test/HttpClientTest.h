#ifndef RA_HTTPCLIENTTEST_H
#define RA_HTTPCLIENTTEST_H

#include <cppunit/extensions/HelperMacros.h>
#include <ra/common/CommonDefine.h>
#include <ra/common/Log.h>
#include <ra/common/HttpClient.h>

RA_BEGIN_NAMESPACE(common);

class HttpClientTest : public CppUnit::TestFixture {
    CPPUNIT_TEST_SUITE(HttpClientTest);
    CPPUNIT_TEST(testInit);
    CPPUNIT_TEST(testRequest);
    CPPUNIT_TEST(testInvalidRequest);
    CPPUNIT_TEST(testLargeBody);
    CPPUNIT_TEST(testEncodedQuery);
    CPPUNIT_TEST_SUITE_END();

public:
    HttpClientTest();
    ~HttpClientTest();

public:
    void setUp();
    void tearDown();

    void testInit();
    void testRequest();
    void testInvalidRequest();
    void testLargeBody();
    void testEncodedQuery();
};

RA_END_NAMESPACE(common);

#endif //RA_HTTPCLIENTTEST_H
