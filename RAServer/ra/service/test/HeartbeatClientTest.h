#ifndef RA_HEARTBEAT_CLIENTTEST_H
#define RA_HEARTBEAT_CLIENTTEST_H

#include <cppunit/extensions/HelperMacros.h>
#include <ra/common/Common.h>
#include <ra/common/Log.h>

RA_BEGIN_NAMESPACE(service);

class HeartbeatClientTest: public CppUnit::TestFixture {
    CPPUNIT_TEST_SUITE(HeartbeatClientTest);
    CPPUNIT_TEST(testHeartbeat);
    CPPUNIT_TEST_SUITE_END();
public:
    HeartbeatClientTest();
    virtual ~HeartbeatClientTest();

    void setUp();
    void tearDown();

    void testHeartbeat();

private:
    RA_LOG_DECLARE();
};

RA_END_NAMESPACE(service);

#endif /* HEARTBEATCLIENTTEST_H */

