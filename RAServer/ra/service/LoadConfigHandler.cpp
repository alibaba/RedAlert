#include <tr1/memory>
#include <ra/common/Json.h>
#include <ra/common/HttpClient.h>
#include <ra/config/ConfigDownloader.h>
#include <ra/service/LoadConfigHandler.h>
#include <ra/app/RaApp.h>

RA_BEGIN_NAMESPACE(service);
RA_USE_NAMESPACE(common);
RA_USE_NAMESPACE(config);
using namespace std;
using namespace std::tr1;

RA_LOG_SETUP(service, LoadConfigHandler);

LoadConfigHandler::LoadConfigHandler(ra::app::RaApp *app): _app(app) {
}

LoadConfigHandler::~LoadConfigHandler() {
}

void LoadConfigHandler::process(const HttpRequest* request, HttpResponse* response) {
    if (request->method != HTTP_REQ_POST) {
        RA_LOG(WARN, "Bad request method, expect POST");
        sendResponse(response, HTTP_RESP_BADMETHOD, "Bad request method, expect POST");
        return;
    }
    const string &body = request->body;
    JsonPtr json = Json::load(body);
    JsonObjectPtr object = dynamic_pointer_cast<JsonObject>(json);
    if (json == NULL || object == NULL) {
        RA_LOG(WARN, "Cannot load json in request, raw request: '%s'", body.c_str());
        sendResponse(response, HTTP_RESP_BADREQUEST, "Cannot load json in request");
        return;
    }
    JsonObject::iterator iter = object->find(HTTP_API_LOAD_CONFIG_PATH_KEY.c_str());
    if (iter == object->end()) {
        RA_LOG(WARN, "No config path in load request");
        sendResponse(response, HTTP_RESP_BADREQUEST, "No config path in load request");
        return;
    }
    JsonStringPtr configPathPtr = dynamic_pointer_cast<JsonString>(iter->second);
    if (configPathPtr == NULL) {
        RA_LOG(WARN, "Value of config path is not a string");
        sendResponse(response, HTTP_RESP_BADREQUEST, "Value of config path is not a string");
        return;
    }
    const string& configPathStr = *configPathPtr;
    RA_LOG(INFO, "Try to load config at path '%s'", configPathStr.c_str());
    if (!doLoadConfig(configPathStr)) {
        sendResponse(response, HTTP_RESP_INTERNAL, "Error in loading config");
        return;
    }
    sendResponse(response, HTTP_RESP_OK, "OK Config Loaded");
}

bool LoadConfigHandler::doLoadConfig(const string& configPathStr) {
    string localAddress = _app->getLocalAddress();
    RA_LOG(INFO, "Try to load config in path '%s'", configPathStr.c_str());
    ConfigDownloader downloader;
    if (!downloader.init(_app->getLocalConfPath())) {
        RA_LOG(WARN, "init downloader failed, ra[%s]", localAddress.c_str());
        return false;
    }
    int32_t ret = downloader.downloadConf(configPathStr);
    if (ret < 0) {
        RA_LOG(WARN, "download config [%s] failed, ra[%s]", configPathStr.c_str(), localAddress.c_str());
        return false;
    }
    if (ret == 0) {
        RA_LOG(INFO, "reload config request version is not bigger than local config "
                     "version[%d]", downloader.getVersion());
    }
    RA_LOG(INFO, "download config rev %d succ, ra[%s]",
           downloader.getVersion(), localAddress.c_str());
    int32_t configVersion = downloader.getVersion();
    assert(configVersion != INVALID_CONFIG_VERSION);
    if (!_app->loadConfig(configVersion)) {
        RA_LOG(WARN, "load config failed, ra[%s]", localAddress.c_str());
        return false;
    }
    RA_LOG(INFO, "reload config rev %d succ!, ra[%s]",
           downloader.getVersion(), localAddress.c_str());
    return true;
}

RA_END_NAMESPACE(service);
