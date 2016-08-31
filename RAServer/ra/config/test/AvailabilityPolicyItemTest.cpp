#include <ra/config/test/AvailabilityPolicyItemTest.h>
#include <ra/common/Json.h>
#include <ra/test/test.h>
#include <cppunit/TestAssert.h>

RA_BEGIN_NAMESPACE(config);
RA_USE_NAMESPACE(common);

CPPUNIT_TEST_SUITE_REGISTRATION(AvailabilityPolicyItemTest);

AvailabilityPolicyItemTest::AvailabilityPolicyItemTest()
{
}

AvailabilityPolicyItemTest::~AvailabilityPolicyItemTest()
{
}

void AvailabilityPolicyItemTest::setUp()
{
}

void AvailabilityPolicyItemTest::tearDown()
{
}

void AvailabilityPolicyItemTest::testSimpleProcess()
{
    std::string jsonStr = "{"
                          "\"alarmLevel\": \"warning\","
                          "\"alarmGroup\": \"dev\","
                          "\"checkType\": \"single\","
                          "\"fetchInterval_s\": 60,"
                          "\"minAlarmInterval_s\": 300,"
                          "\"minHostNum\":99,"
                          "\"maxMissHostNum\":101"
                          "}";

    AvailabilityPolicyItem config;
    JsonPtr json = Json::load(jsonStr);
    CPPUNIT_ASSERT(json);
    CPPUNIT_ASSERT(fromJson(json, config));

    CPPUNIT_ASSERT_EQUAL(std::string("warning"), config.getAlarmLevel());
    CPPUNIT_ASSERT_EQUAL(std::string("dev"), config.getAlarmGroup());
    CPPUNIT_ASSERT_EQUAL(CT_SINGLE, config.getCheckType());
    CPPUNIT_ASSERT_EQUAL(int32_t(60), config.getFetchInterval());
    CPPUNIT_ASSERT_EQUAL(int32_t(300), config.getMinAlarmInterval());
    CPPUNIT_ASSERT_EQUAL(int32_t(99), config.getMinHostNum());
    CPPUNIT_ASSERT_EQUAL(int32_t(101), config.getMaxMissHostNum());
}

RA_END_NAMESPACE(config);

