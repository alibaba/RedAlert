#ifndef RA_CONFIGDOWNLOADERTEST_H
#define RA_CONFIGDOWNLOADERTEST_H

#include <ra/common/CommonDefine.h>
#include <ra/common/Log.h>
#include <ra/config/ConfigDownloader.h>
#include <cppunit/extensions/HelperMacros.h>

RA_BEGIN_NAMESPACE(config);

class ConfigDownloaderTest : public CppUnit::TestFixture {
    CPPUNIT_TEST_SUITE(ConfigDownloaderTest);
    CPPUNIT_TEST(testSimpleProcess);
    CPPUNIT_TEST(testConfigNotExist);
    CPPUNIT_TEST(testVersionExist);
    CPPUNIT_TEST(testRemoteVersionIsSmaller);
    CPPUNIT_TEST_SUITE_END();
public:
    ConfigDownloaderTest();
    ~ConfigDownloaderTest();
public:
    void setUp();
    void tearDown();
    void testSimpleProcess();
    void testConfigNotExist();
    void testVersionExist();
    void testRemoteVersionIsSmaller();
private:
    std::string _cacheDir;
};

RA_END_NAMESPACE(config);

#endif //RA_CONFIGDOWNLOADERTEST_H
