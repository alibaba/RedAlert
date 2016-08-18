#include <ra/config/test/BootstrapConfigTest.h>
#include <ra/common/Json.h>
#include <ra/test/test.h>
#include <cppunit/TestAssert.h>

using namespace std;

RA_BEGIN_NAMESPACE(config);
RA_USE_NAMESPACE(common);
RA_LOG_SETUP(config, BootstrapConfigTest);

CPPUNIT_TEST_SUITE_REGISTRATION(BootstrapConfigTest);

BootstrapConfigTest::BootstrapConfigTest() {
}

BootstrapConfigTest::~BootstrapConfigTest() {
}

void BootstrapConfigTest::setUp() {
    RA_LOG(DEBUG, "setUp!");
}

void BootstrapConfigTest::tearDown() {
    RA_LOG(DEBUG, "tearDown!");
}

void BootstrapConfigTest::testSimpleProcess() {
    RA_LOG(DEBUG, "Begin Test!");
    
    string jsonStr = "{"
	    "\"aliMonitorUrl\":\"alimonitorurl\","
            "\"alarmQueueSize\":1024,"
            "\"checkerThreadNum\":2,"
            "\"checkerQueueSize\":2048,"
            "\"fetcherThreadNum\":3,"
            "\"fetcherQueueSize\":3072,"
            "\"retrieveMetricsPointCount\":6,"
            "\"maxRandomLastRetriveTimeValueSec\":30,"
            "\"reloadTreeIntervalSec\":3600,"
            "\"internalAlarmGroup\":\"group\","
            "\"internalAlarmLevel\":\"level\","
            "\"interalMinAlarmInterval\":35,"
            "\"amonitorAgentPort\":1088,"
            "\"sampleInterval_s\":4,"
            "\"mailUser\":\"foo\","
            "\"mailPassword\":\"passwd\","
            "\"mailServerUrl\":\"smtp://mail.abc.com:300\","
            "\"sendOkCodeIntervalSec\": 99"
            "}";
    BootstrapConfig config;
    JsonPtr json = Json::load(jsonStr);
    CPPUNIT_ASSERT(json);
    CPPUNIT_ASSERT(fromJson(json, config));
    CPPUNIT_ASSERT_EQUAL(string("alimonitorurl"), config.getAliMonitorUrl());
    CPPUNIT_ASSERT_EQUAL((size_t)1024, config.getAlarmQueueSize());
    CPPUNIT_ASSERT_EQUAL((size_t)1, config.getAlarmThreadNum());
    CPPUNIT_ASSERT_EQUAL((size_t)2048, config.getCheckerQueueSize());
    CPPUNIT_ASSERT_EQUAL((size_t)2, config.getCheckerThreadNum());
    CPPUNIT_ASSERT_EQUAL((size_t)3072, config.getFetcherQueueSize());
    CPPUNIT_ASSERT_EQUAL((size_t)3, config.getFetcherThreadNum());
    CPPUNIT_ASSERT_EQUAL((uint32_t)6, config.getRetrieveMetricsPointCount());
    CPPUNIT_ASSERT_EQUAL(30, config.getMaxRandomLastRetriveTimeValueSec());

    CPPUNIT_ASSERT_EQUAL(3600, config.getReloadTreeIntervalSec());
    CPPUNIT_ASSERT_EQUAL(string("group"), config.getInternalAlarmGroup());
    CPPUNIT_ASSERT_EQUAL(string("level"), config.getInternalAlarmLevel());
    CPPUNIT_ASSERT_EQUAL(35, config.getInteralMinAlarmInterval());
    CPPUNIT_ASSERT_EQUAL(1088, config.getAmonitorAgentPort());
    CPPUNIT_ASSERT_EQUAL(string("foo"), config.getMailUser());
    CPPUNIT_ASSERT_EQUAL(string("passwd"), config.getMailPwd());
    CPPUNIT_ASSERT_EQUAL(string("smtp://mail.abc.com:300"), config.getMailServerUrl());
    CPPUNIT_ASSERT_EQUAL((int64_t)99, config.getSendOkCodeIntervalSec());
}

RA_END_NAMESPACE(config);

