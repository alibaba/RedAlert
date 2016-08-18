#include <ra/config/test/RedAlertConfigTest.h>
#include <ra/test/test.h>
#include <cppunit/TestAssert.h>
using namespace std;

RA_BEGIN_NAMESPACE(config);
RA_LOG_SETUP(config, RedAlertConfigTest);

CPPUNIT_TEST_SUITE_REGISTRATION(RedAlertConfigTest);

RedAlertConfigTest::RedAlertConfigTest() { 
}

RedAlertConfigTest::~RedAlertConfigTest() { 
}

void RedAlertConfigTest::setUp() { 
    RA_LOG(DEBUG, "setUp!");
}

void RedAlertConfigTest::tearDown() { 
    RA_LOG(DEBUG, "tearDown!");
}

void RedAlertConfigTest::testLoadConfig()
{
    RedAlertConfig raConfig;
    CPPUNIT_ASSERT_EQUAL(false, raConfig.containThisServer());

    //path not exist
    string sqlitePath = string(TEST_DATA_PATH) + "/config_wrapper/not_exist_sqlite";
    bool ret = raConfig.loadConfig(sqlitePath, "127.0.0.1:1234");
    CPPUNIT_ASSERT(!ret);

    //path not exist
    sqlitePath = string(TEST_DATA_PATH) + "/config_wrapper/sqlite_miss_options";
    ret = raConfig.loadConfig(sqlitePath, "127.0.0.1:1234");
    CPPUNIT_ASSERT(ret);
    CPPUNIT_ASSERT_EQUAL((size_t)0, raConfig._hostVec.size());

    //load success, not contain this server
    sqlitePath = string(TEST_DATA_PATH) + "/config_wrapper/sqlite_normal";
    ret = raConfig.loadConfig(sqlitePath, "127.0.0.1:1234");
    CPPUNIT_ASSERT(ret);
    CPPUNIT_ASSERT_EQUAL(false, raConfig.containThisServer());
    CPPUNIT_ASSERT_EQUAL((size_t)2, raConfig._hostVec.size());

    //load sucess, contain this server
    sqlitePath = string(TEST_DATA_PATH) + "/config_wrapper/sqlite_normal";
    ret = raConfig.loadConfig(sqlitePath, "11.99.88.51:10010");
    CPPUNIT_ASSERT(ret);
    CPPUNIT_ASSERT_EQUAL(true, raConfig.containThisServer());
    CPPUNIT_ASSERT_EQUAL((size_t)2, raConfig._hostVec.size());
    
    HostItem hostItem = raConfig._hostVec[0];
    CPPUNIT_ASSERT_EQUAL(1U, hostItem.getHostID());
    CPPUNIT_ASSERT_EQUAL(string("ra_51"), hostItem.getServiceName());
    CPPUNIT_ASSERT_EQUAL(string("11.99.88.51:20010"), hostItem.getIpAddress());
    CPPUNIT_ASSERT_EQUAL(1000U, hostItem.getWeight());

    hostItem = raConfig._hostVec[1];
    CPPUNIT_ASSERT_EQUAL(2U, hostItem.getHostID());
    CPPUNIT_ASSERT_EQUAL(string("ra_53"), hostItem.getServiceName());
    CPPUNIT_ASSERT_EQUAL(string("11.99.88.51:10010"), hostItem.getIpAddress());
    CPPUNIT_ASSERT_EQUAL(100U, hostItem.getWeight());
    
    CPPUNIT_ASSERT_EQUAL(1000U + 100U, raConfig._consistentHash.size());
}

void RedAlertConfigTest::testFilterMetrics()
{
    RedAlertConfig raConfig;
    raConfig._serviceName = "ra1";
    raConfig._consistentHash.addNode("ra1", 100U);
    
    bool ret = raConfig.isMyMetric("service.nodepath.metric");
    CPPUNIT_ASSERT(ret);
}

RA_END_NAMESPACE(config);

