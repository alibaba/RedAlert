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

LoadConfigHandler::LoadConfigHandler(ra::app::RaApp *app): _app(app) {
}

LoadConfigHandler::~LoadConfigHandler() {
}

void LoadConfigHandler::process(const HttpRequest* request, HttpResponse* response) {
    if (request->method != HTTP_REQ_POST) {
        LOG(ERROR) << "Bad request method, expect POST";
        sendResponse(response, HTTP_RESP_BADMETHOD, "Bad request method, expect POST");
        return;
    }
    const string &body = request->body;
    JsonPtr json = Json::load(body);
    JsonObjectPtr object = dynamic_pointer_cast<JsonObject>(json);
    if (json == NULL || object == NULL) {
        LOG(WARNING) <<  "Cannot load json in request, raw request: " << body;
        sendResponse(response, HTTP_RESP_BADREQUEST, "Cannot load json in request");
        return;
    }
    JsonObject::iterator iter = object->find(HTTP_API_LOAD_CONFIG_PATH_KEY.c_str());
    if (iter == object->end()) {
        LOG(WARNING) << "No config path in load request";
        sendResponse(response, HTTP_RESP_BADREQUEST, "No config path in load request");
        return;
    }
    JsonStringPtr configPathPtr = dynamic_pointer_cast<JsonString>(iter->second);
    if (configPathPtr == NULL) {
        LOG(WARNING) << "Value of config path is not a string";
        sendResponse(response, HTTP_RESP_BADREQUEST, "Value of config path is not a string");
        return;
    }
    const string& configPathStr = *configPathPtr;
    LOG(INFO) << "Try to load config at path " << configPathStr;
    if (!doLoadConfig(configPathStr)) {
        sendResponse(response, HTTP_RESP_INTERNAL, "Error in loading config");
        return;
    }
    sendResponse(response, HTTP_RESP_OK, "OK Config Loaded");
}

bool LoadConfigHandler::doLoadConfig(const string& configPathStr) {
    string localAddress = _app->getLocalAddress();
    LOG(INFO) << "Try to load config in path " << configPathStr;
    ConfigDownloader downloader;
    if (!downloader.init(_app->getLocalConfPath())) {
        LOG(ERROR) << "init downloader failed, ra[" << localAddress <<"]";
        return false;
    }
    int32_t ret = downloader.downloadConf(configPathStr);
    if (ret < 0) {
        LOG(ERROR) << "download config [" << configPathStr <<"] failed, ra[" << localAddress << "]";
        return false;
    }
    if (ret == 0) {
        LOG(INFO) << "reload config request version is not bigger than local config "
		  << "version " << downloader.getVersion();
    }
    LOG(INFO) << "download config rev " << downloader.getVersion() << " succ, ra[" << localAddress <<"]";
    int32_t configVersion = downloader.getVersion();
    assert(configVersion != INVALID_CONFIG_VERSION);
    if (!_app->loadConfig(configVersion)) {
        LOG(ERROR) << "load config failed, ra[" << localAddress << "]";
        return false;
    }
    LOG(INFO) << "reload config rev " << downloader.getVersion() <<" succ!, ra[" << localAddress <<"]";
    return true;
}

RA_END_NAMESPACE(service);
