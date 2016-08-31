#ifndef RA_LIST_METRIC_HANDLER_TEST_H
#define RA_LIST_METRIC_HANDLER_TEST_H

#include <cppunit/extensions/HelperMacros.h>
#include <ra/common/CommonDefine.h>
#include <ra/common/Log.h>

RA_BEGIN_NAMESPACE(service);

class ListMetricHandlerTest: public CppUnit::TestFixture {
    CPPUNIT_TEST_SUITE(ListMetricHandlerTest);
    CPPUNIT_TEST(testListMetric);
    CPPUNIT_TEST_SUITE_END();
public:
    ListMetricHandlerTest();
    virtual ~ListMetricHandlerTest();

    void setUp();
    void tearDown();

    void testListMetric();

private:
    common::JsonArrayPtr doTestList(common::HttpClient& client, const std::string& url);
};

RA_END_NAMESPACE(service);

#endif /* RA_LIST_METRIC_HANDLER_TEST_H */

