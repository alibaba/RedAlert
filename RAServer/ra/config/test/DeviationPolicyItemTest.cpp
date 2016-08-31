#include <ra/config/test/DeviationPolicyItemTest.h>
#include <ra/common/Json.h>
#include <ra/test/test.h>
#include <cppunit/TestAssert.h>

RA_BEGIN_NAMESPACE(config);
RA_USE_NAMESPACE(common);
RA_LOG_SETUP(config, DeviationPolicyItemTest);

CPPUNIT_TEST_SUITE_REGISTRATION(DeviationPolicyItemTest);

DeviationPolicyItemTest::DeviationPolicyItemTest()
{
}

DeviationPolicyItemTest::~DeviationPolicyItemTest()
{
}

void DeviationPolicyItemTest::setUp()
{
}

void DeviationPolicyItemTest::tearDown()
{
}

void DeviationPolicyItemTest::testSimpleProcess()
{
    std::string jsonStr = "{"
            "\"alarmLevel\":\"warning\","
            "\"alarmGroup\":\"dev\","
            "\"checkType\":\"total\","
            "\"fetchInterval_s\":60,"
            "\"minAlarmInterval_s\":300,"
            "\"ascDiffRatio\":0.2,"
            "\"ascDiffValue\":10.01,"
            "\"descDiffRatio\":0.3,"
            "\"descDiffValue\":9.9"
            "}";

    DeviationPolicyItem config;
    JsonPtr json = Json::load(jsonStr);
    CPPUNIT_ASSERT(json);
    CPPUNIT_ASSERT(fromJson(json, config));

    CPPUNIT_ASSERT_EQUAL(std::string("warning"), config.getAlarmLevel());
    CPPUNIT_ASSERT_EQUAL(std::string("dev"), config.getAlarmGroup());
    CPPUNIT_ASSERT_EQUAL(CT_METRIC_TOTAL, config.getCheckType());
    CPPUNIT_ASSERT_EQUAL(int32_t(60), config.getFetchInterval());
    CPPUNIT_ASSERT_EQUAL(int32_t(300), config.getMinAlarmInterval());
    CPPUNIT_ASSERT_DOUBLES_EQUAL(0.2, config.getAscDiffRatio(), 0.001);
    CPPUNIT_ASSERT_DOUBLES_EQUAL(10.01, config.getAscDiffValue(), 0.001);
    CPPUNIT_ASSERT_DOUBLES_EQUAL(0.3, config.getDescDiffRatio(), 0.001);
    CPPUNIT_ASSERT_DOUBLES_EQUAL(9.9, config.getDescDiffValue(), 0.001);
}

void DeviationPolicyItemTest::testIsDiffTooMuch()
{
    std::string jsonStr = "{"
            "\"group\":\"default\","
            "\"metric\":\"test_2\","
            "\"alarmLevel\":\"warning\","
            "\"alarmGroup\":\"dev\","
            "\"checkType\":\"total\","
            "\"fetchInterval_s\":60,"
            "\"minAlarmInterval_s\":300,"
            "\"ascDiffRatio\":0.2,"
            "\"ascDiffValue\":5,"
            "\"descDiffRatio\":0.3,"
            "\"descDiffValue\":20"
            "}";

    DeviationPolicyItem config;
    JsonPtr json = Json::load(jsonStr);
    CPPUNIT_ASSERT(json);
    CPPUNIT_ASSERT(fromJson(json, config));

    CPPUNIT_ASSERT(config.isDiffTooMuch(60.5, 50));
    CPPUNIT_ASSERT(!config.isDiffTooMuch(59.5, 50));
    
    CPPUNIT_ASSERT(config.isDiffTooMuch(69.5, 100));
    CPPUNIT_ASSERT(!config.isDiffTooMuch(70.5, 100));
}

void DeviationPolicyItemTest::testAscValueDiffTooMuch()
{
    std::string jsonStr = "{"
            "\"group\":\"default\","
            "\"metric\":\"test_2\","
            "\"alarmLevel\":\"warning\","
            "\"alarmGroup\":\"dev\","
            "\"checkType\":\"total\","
            "\"fetchInterval_s\":60,"
            "\"minAlarmInterval_s\":300,"
            "\"ascDiffRatio\":0.0,"
            "\"ascDiffValue\":5,"
            "\"descDiffRatio\":0.0,"
            "\"descDiffValue\":0.0"
            "}";

    DeviationPolicyItem config;
    JsonPtr json = Json::load(jsonStr);
    CPPUNIT_ASSERT(json);
    CPPUNIT_ASSERT(fromJson(json, config));
    CPPUNIT_ASSERT(config.isDiffTooMuch(5.1, 0));
    CPPUNIT_ASSERT(!config.isDiffTooMuch(4.9, 0));
    CPPUNIT_ASSERT(!config.isDiffTooMuch(0, 10));
}

void DeviationPolicyItemTest::testAscRatioDiffTooMuch()
{
    std::string jsonStr = "{"
            "\"group\":\"default\","
            "\"metric\":\"test_2\","
            "\"alarmLevel\":\"warning\","
            "\"alarmGroup\":\"dev\","
            "\"checkType\":\"total\","
            "\"fetchInterval_s\":60,"
            "\"minAlarmInterval_s\":300,"
            "\"ascDiffRatio\":0.5,"
            "\"ascDiffValue\":0.0,"
            "\"descDiffRatio\":0.0,"
            "\"descDiffValue\":0.0"
            "}";

    DeviationPolicyItem config;
    JsonPtr json = Json::load(jsonStr);
    CPPUNIT_ASSERT(json);
    CPPUNIT_ASSERT(fromJson(json, config));
    CPPUNIT_ASSERT(config.isDiffTooMuch(1.6, 1));
    CPPUNIT_ASSERT(!config.isDiffTooMuch(1.4, 1));
    CPPUNIT_ASSERT(!config.isDiffTooMuch(0, 5));
}

void DeviationPolicyItemTest::testDescRatioDiffTooMuch()
{
    std::string jsonStr = "{"
            "\"group\":\"default\","
            "\"metric\":\"test_2\","
            "\"alarmLevel\":\"warning\","
            "\"alarmGroup\":\"dev\","
            "\"checkType\":\"total\","
            "\"fetchInterval_s\":60,"
            "\"minAlarmInterval_s\":300,"
            "\"ascDiffRatio\":0,"
            "\"ascDiffValue\":0,"
            "\"descDiffRatio\":0.4,"
            "\"descDiffValue\":0.0"
            "}";

    DeviationPolicyItem config;
    JsonPtr json = Json::load(jsonStr);
    CPPUNIT_ASSERT(json);
    CPPUNIT_ASSERT(fromJson(json, config));
    CPPUNIT_ASSERT(config.isDiffTooMuch(5.9, 10));
    CPPUNIT_ASSERT(!config.isDiffTooMuch(6.1, 10));
    CPPUNIT_ASSERT(!config.isDiffTooMuch(10, 2));
}

void DeviationPolicyItemTest::testDescValueDiffTooMuch()
{
    std::string jsonStr = "{"
            "\"group\":\"default\","
            "\"metric\":\"test_2\","
            "\"alarmLevel\":\"warning\","
            "\"alarmGroup\":\"dev\","
            "\"checkType\":\"total\","
            "\"fetchInterval_s\":60,"
            "\"minAlarmInterval_s\":300,"
            "\"ascDiffRatio\":0,"
            "\"ascDiffValue\":0,"
            "\"descDiffRatio\":0,"
            "\"descDiffValue\":30"
            "}";

    DeviationPolicyItem config;
    JsonPtr json = Json::load(jsonStr);
    CPPUNIT_ASSERT(json);
    CPPUNIT_ASSERT(fromJson(json, config));
    CPPUNIT_ASSERT(config.isDiffTooMuch(10, 40.1));
    CPPUNIT_ASSERT(!config.isDiffTooMuch(10, 39.9));
    CPPUNIT_ASSERT(!config.isDiffTooMuch(50.1, 5));
}

void DeviationPolicyItemTest::testCompatible_010()
{
    std::string jsonStr = "{"
            "\"group\":\"default\","
            "\"metric\":\"test_2\","
            "\"alarmLevel\":\"warning\","
            "\"alarmGroup\":\"dev\","
            "\"checkType\":\"total\","
            "\"fetchInterval_s\":60,"
            "\"minAlarmInterval_s\":300,"
            "\"maxDiffRatio\":0.3,"
            "\"minDiffValue\":10.0"
            "}";

    DeviationPolicyItem config;
    JsonPtr json = Json::load(jsonStr);
    CPPUNIT_ASSERT(json);
    CPPUNIT_ASSERT(fromJson(json, config));
    CPPUNIT_ASSERT_DOUBLES_EQUAL(0.3, config.getAscDiffRatio(), 0.0001);
    CPPUNIT_ASSERT_DOUBLES_EQUAL(0.3, config.getDescDiffRatio(), 0.0001);
    CPPUNIT_ASSERT_DOUBLES_EQUAL(10.0, config.getAscDiffValue(), 0.0001);
    CPPUNIT_ASSERT_DOUBLES_EQUAL(10.0, config.getDescDiffValue(), 0.0001);
}

RA_END_NAMESPACE(config);

