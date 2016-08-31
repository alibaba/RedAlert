#include <sstream>
#include <contrib/filesystem/FileSystemFactory.h>
#include <ra/common/Json.h>
#include <ra/util/FileUtil.h>
#include <ra/config/ConfigWrapper.h>
#include <ra/app/RaApp.h>
#include <ra/test/test.h>
#include <ra/service/test/LoadConfigHandlerTest.h>

RA_BEGIN_NAMESPACE(service);
RA_USE_NAMESPACE(common);
RA_USE_NAMESPACE(config);
RA_USE_NAMESPACE(app);
RA_USE_NAMESPACE(fs);
RA_USE_NAMESPACE(util);
using namespace std;
using namespace std::tr1;

RA_LOG_SETUP(service, LoadConfigHandlerTest);
CPPUNIT_TEST_SUITE_REGISTRATION(LoadConfigHandlerTest);

LoadConfigHandlerTest::LoadConfigHandlerTest() {
}

LoadConfigHandlerTest::~LoadConfigHandlerTest() {
}

void LoadConfigHandlerTest::setUp() {
    FileSystemPtr fs = FileSystemFactory::create(DEFAULT_FILE_SYSTEM);
    CPPUNIT_ASSERT(FileUtil::init(fs));
}

void LoadConfigHandlerTest::tearDown() {
}

void LoadConfigHandlerTest::testLoadConfig() {
    string configPath = string(TEST_DATA_PATH) + "/load_config_handler";
    int32_t version = 1;
    ConfigWrapperPtr configWrapper(new ConfigWrapper());
    configWrapper->_configVersion = version;
    RaApp app;
    app._workDir = configPath;
    app.setConfigWrapper(configWrapper);
    LoadConfigHandler handler(&app);
    HttpServer server;
    CPPUNIT_ASSERT(server.init("127.0.0.1", 18082));
    CPPUNIT_ASSERT(server.registerHandler(HTTP_API_PATH_LOAD_CONFIG, &handler));
    CPPUNIT_ASSERT(server.start());

    HttpClient client;
    CPPUNIT_ASSERT(client.init());
    string url = "http://127.0.0.1:18082" + HTTP_API_PATH_LOAD_CONFIG;
    stringstream body;
    body << "{\"" << HTTP_API_LOAD_CONFIG_PATH_KEY << "\": \"" << configPath << "/" << version << "\"}";
    HttpResponse response;
    client.post(url, body.str(), &response);
    CPPUNIT_ASSERT_EQUAL(HTTP_RESP_OK, response.status);

    CPPUNIT_ASSERT(server.stop());
}

RA_END_NAMESPACE(service);
