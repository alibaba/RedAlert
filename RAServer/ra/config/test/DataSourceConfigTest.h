#ifndef RA_DATA_SOURCE_CONFIG_TEST_H
#define RA_DATA_SOURCE_CONFIG_TEST_H

#include <ra/common/Common.h>
#include <ra/common/Log.h>
#include <ra/config/DataSourceConfig.h>
#include <cppunit/extensions/HelperMacros.h>
#include <set>

RA_BEGIN_NAMESPACE(config);

typedef std::map<std::string, int> Str2IntMap;

class DataSourceConfigTest : public CppUnit::TestFixture {
    CPPUNIT_TEST_SUITE(DataSourceConfigTest);
    CPPUNIT_TEST(testLoadConfig);
    CPPUNIT_TEST_SUITE_END();
public:
    DataSourceConfigTest();
    ~DataSourceConfigTest();
public:
    void setUp();
    void tearDown();
    void testLoadConfig();

private:
    StringSet _specs;

private:
    RA_LOG_DECLARE();
};

RA_END_NAMESPACE(config);

#endif //RA_DATA_SOURCE_CONFIG_TEST_H
