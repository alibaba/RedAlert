#include <ra/config/test/ConfigWrapperTest.h>
#include <ra/test/test.h>
#include <cppunit/TestAssert.h>
#include <string>
using namespace std;

RA_BEGIN_NAMESPACE(config);
RA_LOG_SETUP(config, ConfigWrapperTest);

CPPUNIT_TEST_SUITE_REGISTRATION(ConfigWrapperTest);

ConfigWrapperTest::ConfigWrapperTest() { 
}

ConfigWrapperTest::~ConfigWrapperTest() { 
}

void ConfigWrapperTest::setUp() { 
    RA_LOG(DEBUG, "setUp!");
}

void ConfigWrapperTest::tearDown() { 
    RA_LOG(DEBUG, "tearDown!");
}

void ConfigWrapperTest::testSimpleProcess() { 
    RA_LOG(DEBUG, "Begin Test!");

    // CPPUNIT_ASSERT(false);
    // CPPUNIT_ASSERT_EQUAL(0, 1);
}

void ConfigWrapperTest::testLoadPairVals() 
{
    ConfigWrapper configWrapper;
    CPPUNIT_ASSERT_EQUAL(0.0, configWrapper.getSmoothingFactor());
    CPPUNIT_ASSERT_EQUAL(0.0, configWrapper.getTrendFactor());

    //path not exist
    string sqlitePath = string(TEST_DATA_PATH) + "/config_wrapper/not_exist_sqlite";
    bool ret = configWrapper.loadPairVals(sqlitePath);
    CPPUNIT_ASSERT(!ret);

    //there is no table
    
    //miss option
    sqlitePath = string(TEST_DATA_PATH) + "/config_wrapper/sqlite_miss_options";
    ret = configWrapper.loadPairVals(sqlitePath);
    CPPUNIT_ASSERT(!ret);

    //normal
    sqlitePath = string(TEST_DATA_PATH) + "/config_wrapper/sqlite_normal";
    ret = configWrapper.loadPairVals(sqlitePath);
    CPPUNIT_ASSERT(ret);
    CPPUNIT_ASSERT_EQUAL(0.45, configWrapper.getSmoothingFactor());
    CPPUNIT_ASSERT_EQUAL(0.2, configWrapper.getTrendFactor());
}

void ConfigWrapperTest::testLoadConfig()
{
    ConfigWrapper config;
    string configRoot = string(TEST_DATA_PATH) + "/config_wrapper/";

    //there is no this config version
    int32_t configVersion = 5;
    CPPUNIT_ASSERT(!config.loadConfig(configRoot, configVersion, "127.0.0.1:1234"));

    //load empty sqlite
    configVersion = 2;
    CPPUNIT_ASSERT(config.loadConfig(configRoot, configVersion, "127.0.0.1:1234"));
    CPPUNIT_ASSERT_EQUAL(configVersion, config.getConfigVersion());
    CPPUNIT_ASSERT(!config._redAlertConfig.containThisServer());
    CPPUNIT_ASSERT_EQUAL((size_t)0, config._dataSourceConfig.getAmonitorCount());
    CPPUNIT_ASSERT_EQUAL((size_t)0, config._shieldConfig.getShieldItemCount());
    CPPUNIT_ASSERT_EQUAL((size_t)0, config._policyConfig.getPolicyCount());

    //load success, but has no this server id
    configVersion = 3;
    CPPUNIT_ASSERT(config.loadConfig(configRoot, configVersion, "127.0.0.1:1234"));
    CPPUNIT_ASSERT_EQUAL(configVersion, config.getConfigVersion());
    CPPUNIT_ASSERT(!config._redAlertConfig.containThisServer());
    CPPUNIT_ASSERT_EQUAL((size_t)0, config._dataSourceConfig.getAmonitorCount());
    CPPUNIT_ASSERT_EQUAL((size_t)0, config._shieldConfig.getShieldItemCount());
    CPPUNIT_ASSERT_EQUAL((size_t)0, config._policyConfig.getPolicyCount());

    //load success, and has this server
    CPPUNIT_ASSERT(config.loadConfig(configRoot, configVersion, "11.99.88.51:20010"));
    CPPUNIT_ASSERT_EQUAL(configVersion, config.getConfigVersion());
    CPPUNIT_ASSERT(config._redAlertConfig.containThisServer());
    CPPUNIT_ASSERT_EQUAL((size_t)2, config._dataSourceConfig.getAmonitorCount());
    CPPUNIT_ASSERT_EQUAL((size_t)2, config._shieldConfig.getShieldItemCount());
    CPPUNIT_ASSERT_EQUAL((size_t)8, config._policyConfig.getPolicyCount());
    CPPUNIT_ASSERT_EQUAL((size_t)2, config._redAlertConfig.getRedAlertCount());
}

void ConfigWrapperTest::testGetPolicyItems()
{
    ConfigWrapper config;
    PolicyConfig& policyConfig = config._policyConfig;
    PolicyConfigItemBasePtr ptr = TrendPolicyItemPtr(new TrendPolicyItem());
    ptr->setMetric("trendMetric");
    policyConfig._policyItemVec.push_back(ptr);
    
    ptr = ThresholdPolicyItemPtr(new ThresholdPolicyItem());
    ptr->setMetric("thresholdMetric");
    policyConfig._policyItemVec.push_back(ptr);

    ptr = AvailabilityPolicyItemPtr(new AvailabilityPolicyItem());
    ptr->setMetric("AvailabilityMetric");
    policyConfig._policyItemVec.push_back(ptr);

    ptr = SingularityPolicyItemPtr(new SingularityPolicyItem());
    ptr->setMetric("singularityMetric");
    policyConfig._policyItemVec.push_back(ptr);

    ptr = CyclePolicyItemPtr(new CyclePolicyItem());
    ptr->setMetric("cycleMetric");
    policyConfig._policyItemVec.push_back(ptr);

    vector<PolicyConfigItemBasePtr> policyItemVec;
    config.getPolicyItems(policyItemVec);
    CPPUNIT_ASSERT_EQUAL((size_t)5, policyItemVec.size());
    
    CPPUNIT_ASSERT_EQUAL(string("trendMetric"), policyItemVec[0]->getMetric());
    CPPUNIT_ASSERT_EQUAL(string("thresholdMetric"), policyItemVec[1]->getMetric());
    CPPUNIT_ASSERT_EQUAL(string("AvailabilityMetric"), policyItemVec[2]->getMetric());
    CPPUNIT_ASSERT_EQUAL(string("singularityMetric"), policyItemVec[3]->getMetric());
    CPPUNIT_ASSERT_EQUAL(string("cycleMetric"), policyItemVec[4]->getMetric());
}

RA_END_NAMESPACE(config);

