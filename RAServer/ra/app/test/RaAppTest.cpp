#include <sys/socket.h>
#include <arpa/inet.h>
#include <cppunit/TestAssert.h>
#include <contrib/filesystem/FileSystemFactory.h>
#include <ra/test/test.h>
#include <ra/util/FileUtil.h>
#include <ra/common/HttpServer.h>
#include <ra/app/test/RaAppTest.h>
#include <errno.h>
RA_BEGIN_NAMESPACE(app);
RA_USE_NAMESPACE(util);
RA_USE_NAMESPACE(fs);
RA_LOG_SETUP(app, RaAppTest);
using namespace std;
CPPUNIT_TEST_SUITE_REGISTRATION(RaAppTest);

#define ARRAY_LEN(a) (sizeof(a)/sizeof(a[0]))

RaAppTest::RaAppTest() {
}

RaAppTest::~RaAppTest() {
}

void RaAppTest::setUp() {
    RA_LOG(INFO, "setUp!");
    _bootConfFile = string(TEST_DATA_PATH) + "/ra_app/bootstrap.json";
    _logConfFile = string(TEST_DATA_PATH) + "/ra_app/ra_log.conf";

    _port = selectPort();
    snprintf(_portStr, sizeof(_portStr), "%u", _port);
    RA_LOG(INFO, "RA port: %s", _portStr);
    FileSystemPtr fs = FileSystemFactory::create(DEFAULT_FILE_SYSTEM);
    CPPUNIT_ASSERT(FileUtil::init(fs));
}

void RaAppTest::tearDown() {
    RA_LOG(INFO, "tearDown!");
}

uint32_t RaAppTest::selectPort()
{
        int32_t _mockfd = socket(PF_INET, SOCK_STREAM, 0);
        if (_mockfd < 0) {
            RA_LOG(ERROR, "create socket error %d", errno);
            return 0;
        }

        if (listen(_mockfd, 128) < 0) {
            RA_LOG(ERROR, "listen socket error %d", errno);
            return 0;
        }

        sockaddr_in addr;
        socklen_t addrlen = sizeof(addr);
        if (getsockname(_mockfd,  (sockaddr*)&addr, &addrlen) < 0) {
            RA_LOG(ERROR, "getpeername error %d", errno);
            return 0;
        }

        if (_mockfd > 0) {
            close(_mockfd);
        }

        return (uint32_t)ntohs(addr.sin_port);
}

void RaAppTest::testSimpleProcess() {
    common::HttpServer _server;
    CPPUNIT_ASSERT(_server.init("127.0.0.1", 18083));
    RaApp app;
    const char* argv[] = {"ra", "-p", _portStr, "-l", (char*)_logConfFile.c_str(), "-c", (char*)_bootConfFile.c_str()};
    CPPUNIT_ASSERT(app.init(ARRAY_LEN(argv), (char **)argv));
}

void RaAppTest::testInitSuccess() {
    common::HttpServer _server;
    CPPUNIT_ASSERT(_server.init("127.0.0.1", 18083));
    RaApp app;
    // test normal usage
    const char* argv[] = {"ra", "-p", _portStr, "-l", (char*)_logConfFile.c_str(), "-c", (char*)_bootConfFile.c_str()};
    CPPUNIT_ASSERT(app.init(ARRAY_LEN(argv), (char **)argv));
    CPPUNIT_ASSERT_EQUAL(_logConfFile, app._logConfFile);
    CPPUNIT_ASSERT_EQUAL(_port, app._port);
    CPPUNIT_ASSERT_EQUAL(string("127.0.0.1:") + _portStr, app.getLocalAddress());
}

void RaAppTest::testInitLackOfPort() {
    common::HttpServer _server;
    CPPUNIT_ASSERT(_server.init("127.0.0.1", 18083));
    RaApp app;
    // test lack of port
    const char* argv[] = {"ra", "-l", (char*)_logConfFile.c_str(), "-c", (char*)_bootConfFile.c_str()};
    CPPUNIT_ASSERT(!app.init(ARRAY_LEN(argv), (char **)argv));
}

void RaAppTest::testLoadBootstrapConf()
{
    RaApp app;
    //boot strap not exist
    string path = string(TEST_DATA_PATH) + "/ra_app/bootstrap_not_exist.json";
    CPPUNIT_ASSERT(!app.loadBootstrapConf(path));

    //wrong format
    path = string(TEST_DATA_PATH) + "/ra_app/bootstrap_wrong_format.json";
    CPPUNIT_ASSERT(!app.loadBootstrapConf(path));

    //normal
    path = string(TEST_DATA_PATH) + "/ra_app/bootstrap.json";
    CPPUNIT_ASSERT(app.loadBootstrapConf(path));
    
    const config::BootstrapConfig& config = app._bootstrapConfig;
    CPPUNIT_ASSERT_EQUAL(string("tcp:127.0.0.1:15776"), config.getAliMonitorUrl());
    CPPUNIT_ASSERT_EQUAL((size_t)1024, config.getAlarmQueueSize());
    CPPUNIT_ASSERT_EQUAL((size_t)1, config.getAlarmThreadNum());
    CPPUNIT_ASSERT_EQUAL((size_t)2048, config.getCheckerQueueSize());
    CPPUNIT_ASSERT_EQUAL((size_t)2, config.getCheckerThreadNum());
    CPPUNIT_ASSERT_EQUAL((size_t)4096, config.getFetcherQueueSize());
    CPPUNIT_ASSERT_EQUAL((size_t)4, config.getFetcherThreadNum());
    CPPUNIT_ASSERT_EQUAL((uint32_t)7, config.getRetrieveMetricsPointCount());
    CPPUNIT_ASSERT_EQUAL(25, config.getMaxRandomLastRetriveTimeValueSec());
    CPPUNIT_ASSERT_EQUAL(900, config.getReloadTreeIntervalSec());
    CPPUNIT_ASSERT_EQUAL(string("shenma_ra_internal_alarm"), config.getInternalAlarmGroup());
    CPPUNIT_ASSERT_EQUAL(string("alimonitor"), config.getInternalAlarmLevel());
    CPPUNIT_ASSERT_EQUAL(45, config.getInteralMinAlarmInterval());
    CPPUNIT_ASSERT_EQUAL(10086, config.getAmonitorAgentPort());
    CPPUNIT_ASSERT_EQUAL(string("shenma-alarm"), config.getMailUser());
    CPPUNIT_ASSERT_EQUAL(string("passwd"), config.getMailPwd());
    CPPUNIT_ASSERT_EQUAL(string("smtp://mail.abc.com:300"), config.getMailServerUrl());
    CPPUNIT_ASSERT_EQUAL((int64_t)25, config.getSendOkCodeIntervalSec());
 }

RA_END_NAMESPACE(app);
