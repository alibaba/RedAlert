#include <stdlib.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <cppunit/TestAssert.h>
#include <contrib/filesystem/FileSystemFactory.h>
#include <ra/test/test.h>
#include <ra/util/FileUtil.h>
#include <ra/config/test/ConfigDownloaderTest.h>

using namespace std;

RA_BEGIN_NAMESPACE(config);
RA_USE_NAMESPACE(util);
RA_USE_NAMESPACE(fs);
RA_LOG_SETUP(config, ConfigDownloaderTest);

CPPUNIT_TEST_SUITE_REGISTRATION(ConfigDownloaderTest);

ConfigDownloaderTest::ConfigDownloaderTest() { 
}

ConfigDownloaderTest::~ConfigDownloaderTest() { 
}

void ConfigDownloaderTest::setUp() { 
    //create temp cache dir
    char cacheDir[1024];
    snprintf(cacheDir, sizeof(cacheDir), "%s/config_downloader/cache_XXXXXX", TEST_DATA_PATH);
    char* realCacheDir = mkdtemp(cacheDir);
    chdir(realCacheDir);
    mkdir("conf", 0777);
    _cacheDir = realCacheDir;
    FileSystemPtr fs = FileSystemFactory::create(DEFAULT_FILE_SYSTEM);
    CPPUNIT_ASSERT(FileUtil::init(fs));
}

void ConfigDownloaderTest::tearDown() { 
    if (!_cacheDir.empty()) {
        string cmdBuffer;
        cmdBuffer.append("rm -rvf ");
        cmdBuffer.append(_cacheDir);
        int err = system(cmdBuffer.c_str());
        CPPUNIT_ASSERT_EQUAL(0, err);
    }
}

void ConfigDownloaderTest::testSimpleProcess() { 
    ConfigDownloader downloader;
    CPPUNIT_ASSERT(downloader.init("./conf"));
    CPPUNIT_ASSERT_EQUAL(INVALID_CONFIG_VERSION, downloader.getVersion());
    string srcPath = string(TEST_DATA_PATH) + "/config_downloader/1";
    int ret = downloader.downloadConf(srcPath);
    CPPUNIT_ASSERT(ret > 0);
    CPPUNIT_ASSERT_EQUAL(1, downloader.getVersion());
    string confFilePath = downloader.getConfigFilePath();
    CPPUNIT_ASSERT_EQUAL(string("./conf/1/") + string(CONF_FILE_NAME), confFilePath);
    CPPUNIT_ASSERT_EQUAL(access(confFilePath.c_str(), R_OK), 0);
}

void ConfigDownloaderTest::testConfigNotExist() {
    ConfigDownloader downloader;
    CPPUNIT_ASSERT(downloader.init("./conf"));
    string srcPath = string(TEST_DATA_PATH) + "/config_downloader/4/";
    int ret = downloader.downloadConf(srcPath);
    CPPUNIT_ASSERT(ret < 0);
    CPPUNIT_ASSERT_EQUAL(INVALID_CONFIG_VERSION, downloader.getVersion());
}

void ConfigDownloaderTest::testVersionExist() {
    string srcPath = string(TEST_DATA_PATH) + "/config_downloader/2";
    char buff[128];
    getcwd(buff, sizeof(buff));
    string cmdBuffer;
    cmdBuffer.append("cp -rvf ");
    cmdBuffer.append(srcPath);
    cmdBuffer.append(" ./conf/");
    int err = system(cmdBuffer.c_str());
    CPPUNIT_ASSERT_EQUAL(0, err);
    ConfigDownloader downloader;
    CPPUNIT_ASSERT(downloader.init("./conf"));
    int ret = downloader.downloadConf(srcPath);
    CPPUNIT_ASSERT_EQUAL(0, ret);
    CPPUNIT_ASSERT_EQUAL(2, downloader.getVersion());
}

void ConfigDownloaderTest::testRemoteVersionIsSmaller() {
    string srcPath = string(TEST_DATA_PATH) + "/config_downloader/3";
    string cmdBuffer;
    cmdBuffer.append("cp -rvf ");
    cmdBuffer.append(srcPath);
    cmdBuffer.append(" ./conf/");
    int err = system(cmdBuffer.c_str());
    CPPUNIT_ASSERT_EQUAL(0, err);
    ConfigDownloader downloader;
    CPPUNIT_ASSERT(downloader.init("./conf"));

    string remoteConfigPath = string(TEST_DATA_PATH) + "/config_downloader///1//";
    int ret = downloader.downloadConf(srcPath);
    CPPUNIT_ASSERT_EQUAL(0, ret);
    CPPUNIT_ASSERT_EQUAL(3, downloader.getVersion());
}

RA_END_NAMESPACE(config);

