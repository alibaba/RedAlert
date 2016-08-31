#include <ra/config/test/ShieldConfigTest.h>
#include <ra/test/test.h>
#include <ra/util/Util.h>
#include <cppunit/TestAssert.h>

using namespace std;
using namespace ra::util;
RA_USE_NAMESPACE(util);

RA_BEGIN_NAMESPACE(config);
RA_LOG_SETUP(config, ShieldConfigTest);

CPPUNIT_TEST_SUITE_REGISTRATION(ShieldConfigTest);

ShieldConfigTest::ShieldConfigTest() { 
}

ShieldConfigTest::~ShieldConfigTest() { 
}

void ShieldConfigTest::setUp() { 
}

void ShieldConfigTest::tearDown() { 
}

void ShieldConfigTest::testLoadConfig()
{
    ShieldConfig shieldConfig;

    //path not exist
    string sqlitePath = string(TEST_DATA_PATH) + "/config_wrapper/not_exist_sqlite";
    bool ret = shieldConfig.loadConfig(sqlitePath);
    CPPUNIT_ASSERT(!ret);

    //empty table
    sqlitePath = string(TEST_DATA_PATH) + "/config_wrapper/sqlite_miss_options";
    ret = shieldConfig.loadConfig(sqlitePath);
    CPPUNIT_ASSERT(ret);
    CPPUNIT_ASSERT_EQUAL((size_t)0, shieldConfig._shieldItemVec.size());

    //load sucess
    sqlitePath = string(TEST_DATA_PATH) + "/config_wrapper/sqlite_normal";
    ret = shieldConfig.loadConfig(sqlitePath);
    CPPUNIT_ASSERT(ret);
    CPPUNIT_ASSERT_EQUAL((size_t)2, shieldConfig._shieldItemVec.size());
    
    ShieldItem item = shieldConfig._shieldItemVec[0];
    CPPUNIT_ASSERT_EQUAL(67U, item.id);
    CPPUNIT_ASSERT_EQUAL(string("sc_online"), item.group);
    CPPUNIT_ASSERT_EQUAL(string("shortcut.success_qps/app_for_sug_*.success_qps"), item.metric);
    CPPUNIT_ASSERT_EQUAL((size_t)2, item.hostSet.size());
    CPPUNIT_ASSERT(item.hostSet.count(string("10.99.1.*")) == 1);
    CPPUNIT_ASSERT(item.hostSet.count(string("10.99.*.100")) == 1);
    time_t endTime = 0;
    Util::formatTime("2018-11-21 18:50:04", endTime);
    CPPUNIT_ASSERT_EQUAL(endTime, item.endTime);

    item = shieldConfig._shieldItemVec[1];
    CPPUNIT_ASSERT_EQUAL(72U, item.id);
    CPPUNIT_ASSERT_EQUAL(string("online_tmd"), item.group);
    CPPUNIT_ASSERT_EQUAL(string("tmd.*.sm.cn*s/qps_overmax.wait_qps"), item.metric);
    CPPUNIT_ASSERT_EQUAL((size_t)1, item.hostSet.size());
    CPPUNIT_ASSERT_EQUAL(string("*"), *item.hostSet.begin());
    endTime = 0;
    Util::formatTime("2015-03-11 14:35:12", endTime);
    CPPUNIT_ASSERT_EQUAL(endTime, item.endTime);
}

void ShieldConfigTest::testIsShielded()
{
    ShieldConfig shieldConfig;
    //load sucess
    string sqlitePath = string(TEST_DATA_PATH) + "/config_wrapper/sqlite_normal";
    bool ret = shieldConfig.loadConfig(sqlitePath);
    CPPUNIT_ASSERT(ret);
    CPPUNIT_ASSERT_EQUAL((size_t)2, shieldConfig._shieldItemVec.size());
    CPPUNIT_ASSERT_EQUAL((size_t)0, shieldConfig._metricShieldTimeMap.size());

    //not shield metrics
    time_t currTime = Util::currentTimeInSeconds();
    CPPUNIT_ASSERT(!shieldConfig.isShielded(currTime, "sc_online", "xxxxx.xxx", 0));
    CPPUNIT_ASSERT_EQUAL((size_t)1, shieldConfig._metricShieldTimeMap.size());
    CPPUNIT_ASSERT_EQUAL((time_t)0, shieldConfig._metricShieldTimeMap.begin()->second);
    CPPUNIT_ASSERT(!shieldConfig.isShielded(currTime, "sc_online", "xxxxx.xxx", 0));
    CPPUNIT_ASSERT_EQUAL((size_t)1, shieldConfig._metricShieldTimeMap.size());

    //shield metric, but endTime < currTime
    CPPUNIT_ASSERT(!shieldConfig.isShielded(currTime, "online_tmd", "tmd.xx.sm.cnxxs/qps_overmax.wait_qps", 0));
    CPPUNIT_ASSERT_EQUAL((size_t)2, shieldConfig._metricShieldTimeMap.size());

    //shield metric
    CPPUNIT_ASSERT(Util::formatTime("2000-01-01 12:00:00", currTime));
    uint32_t host;
    CPPUNIT_ASSERT(Util::StringToIP("10.99.1.0", host));
    CPPUNIT_ASSERT(shieldConfig.isShielded(currTime, "sc_online", "shortcut.success_qps/app_for_sug_xx.success_qps", host));
    CPPUNIT_ASSERT_EQUAL((size_t)3, shieldConfig._metricShieldTimeMap.size());

    //test again
    shieldConfig._metricShieldTimeMap.clear();
    shieldConfig._shieldItemVec.clear();
    ShieldItem item;
    item.group = "group";
    item.metric = "metric";
    item.hostSet.insert("x.x.x.x");
    item.endTime = 10000;
    shieldConfig._shieldItemVec.push_back(item);
    CPPUNIT_ASSERT(!shieldConfig.isShielded(100, "group", "metric", 12345));
    
    item.hostSet.insert("10.99.9.9");
    shieldConfig._shieldItemVec.push_back(item);
    CPPUNIT_ASSERT(Util::StringToIP("10.99.9.9", host));
    CPPUNIT_ASSERT(shieldConfig.isShielded(100, "group", "metric", host));
}

void ShieldConfigTest::testShieldItem()
{
    ShieldItem item;
    item.id = 1;
    item.group = "aa*cc**dd";
    item.metric = "mm*nn";
    item.hostSet.insert("10.99.1.*");
    item.hostSet.insert("10.99.2.100");
    
    //matched
    uint32_t host;
    CPPUNIT_ASSERT(Util::StringToIP("10.99.1.8", host));
    CPPUNIT_ASSERT(item.isMatched("aaccdd", "mmxxnn", host));

    //matched 2
    CPPUNIT_ASSERT(Util::StringToIP("10.99.2.100", host));
    CPPUNIT_ASSERT(item.isMatched("aaxxccxxdd", "mmnn", host));

    //group not match
    CPPUNIT_ASSERT(Util::StringToIP("10.99.2.100", host));
    CPPUNIT_ASSERT(!item.isMatched("aaxxccxxdd####", "mmnn", host));

    //metric not match
    CPPUNIT_ASSERT(Util::StringToIP("10.99.2.100", host));
    CPPUNIT_ASSERT(!item.isMatched("aaxxccxxdd", "mmnn####", host));

    //host not match
    CPPUNIT_ASSERT(Util::StringToIP("10.99.2.101", host));
    CPPUNIT_ASSERT(!item.isMatched("aaxxccxxdd", "mmnn", host));
}


RA_END_NAMESPACE(config);

