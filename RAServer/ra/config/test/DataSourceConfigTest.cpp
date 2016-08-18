#include <ra/config/test/DataSourceConfigTest.h>
#include <ra/test/test.h>
#include <cppunit/TestAssert.h>
using namespace std;

RA_BEGIN_NAMESPACE(config);
RA_LOG_SETUP(config, DataSourceConfigTest);

CPPUNIT_TEST_SUITE_REGISTRATION(DataSourceConfigTest);

DataSourceConfigTest::DataSourceConfigTest() {
}

DataSourceConfigTest::~DataSourceConfigTest() {
}

void DataSourceConfigTest::setUp() {
    RA_LOG(DEBUG, "setUp!");
}

void DataSourceConfigTest::tearDown() {
    RA_LOG(DEBUG, "tearDown!");
}

void DataSourceConfigTest::testLoadConfig()
{
    DataSourceConfig amonConfig;

    //path not exist
    string sqlitePath = string(TEST_DATA_PATH) + "/config_wrapper/not_exist_sqlite";
    bool ret = amonConfig.loadConfig(sqlitePath);
    CPPUNIT_ASSERT(!ret);

    //empty table
    sqlitePath = string(TEST_DATA_PATH) + "/config_wrapper/sqlite_miss_options";
    ret = amonConfig.loadConfig(sqlitePath);
    CPPUNIT_ASSERT(ret);
    CPPUNIT_ASSERT_EQUAL((size_t)0, amonConfig._hostVec.size());


    //load sucess
    sqlitePath = string(TEST_DATA_PATH) + "/config_wrapper/sqlite_normal";
    ret = amonConfig.loadConfig(sqlitePath);
    CPPUNIT_ASSERT(ret);
    CPPUNIT_ASSERT_EQUAL((size_t)2, amonConfig._hostVec.size());
    
    HostItem hostItem = amonConfig._hostVec[0];
    CPPUNIT_ASSERT_EQUAL(4U, hostItem.getHostID());
    CPPUNIT_ASSERT_EQUAL(string("amon_51"), hostItem.getServiceName());
    CPPUNIT_ASSERT_EQUAL(string("11.99.88.51:10087"), hostItem.getIpAddress());

    hostItem = amonConfig._hostVec[1];
    CPPUNIT_ASSERT_EQUAL(5U, hostItem.getHostID());
    CPPUNIT_ASSERT_EQUAL(string("amon_1"), hostItem.getServiceName());
    CPPUNIT_ASSERT_EQUAL(string("11.99.88.1:10087"), hostItem.getIpAddress());

    //get specs
    set<string> specSet;
    amonConfig.getSpecs(specSet);
    CPPUNIT_ASSERT_EQUAL((size_t)2, specSet.size());
    CPPUNIT_ASSERT_EQUAL((size_t)1, specSet.count("11.99.88.51:10087"));
    CPPUNIT_ASSERT_EQUAL((size_t)1, specSet.count("11.99.88.1:10087"));
    amonConfig.getSpecs(specSet);
    CPPUNIT_ASSERT_EQUAL((size_t)2, specSet.size());
}

RA_END_NAMESPACE(config);

