#include <ra/config/test/ThresholdPolicyItemTest.h>
#include <ra/common/Json.h>
#include <ra/test/test.h>
#include <cppunit/TestAssert.h>

RA_BEGIN_NAMESPACE(config);
RA_USE_NAMESPACE(common);

CPPUNIT_TEST_SUITE_REGISTRATION(ThresholdPolicyItemTest);

ThresholdPolicyItemTest::ThresholdPolicyItemTest()
{
}

ThresholdPolicyItemTest::~ThresholdPolicyItemTest()
{
}

void ThresholdPolicyItemTest::setUp()
{
}

void ThresholdPolicyItemTest::tearDown()
{
}

void ThresholdPolicyItemTest::testSimpleProcess()
{
    std::string jsonStr = "{"
                          "\"alarmLevel\": \"warning\","
                          "\"alarmGroup\": \"dev\","
                          "\"checkType\": \"single\","
                          "\"fetchInterval_s\": 60,"
                          "\"minAlarmInterval_s\": 300,"
                          "\"upBound\":4.2,"
                          "\"downBound\":10.0"
                          "}";

    ThresholdPolicyItem config;
    JsonPtr json = Json::load(jsonStr);
    CPPUNIT_ASSERT(json);
    CPPUNIT_ASSERT(fromJson(json, config));

    CPPUNIT_ASSERT_EQUAL(std::string("warning"), config.getAlarmLevel());
    CPPUNIT_ASSERT_EQUAL(std::string("dev"), config.getAlarmGroup());
    CPPUNIT_ASSERT_EQUAL(CT_SINGLE, config.getCheckType());
    CPPUNIT_ASSERT_EQUAL(int32_t(60), config.getFetchInterval());
    CPPUNIT_ASSERT_EQUAL(int32_t(300), config.getMinAlarmInterval());
    CPPUNIT_ASSERT_DOUBLES_EQUAL(4.2, config.getUpBound(), 0.001);
    CPPUNIT_ASSERT_DOUBLES_EQUAL(10.0, config.getDownBound(), 0.001);

}

RA_END_NAMESPACE(config);

