#include <tr1/memory>
#include <ra/app/RaApp.h>
#include <ra/util/Util.h>
#include <ra/common/Json.h>
#include <ra/config/ConfigWrapper.h>
#include <ra/service/HeartbeatClient.h>

RA_BEGIN_NAMESPACE(service);
RA_USE_NAMESPACE(common);
RA_USE_NAMESPACE(config);
RA_USE_NAMESPACE(util);
using namespace std;
using namespace std::tr1;

HeartbeatClient::HeartbeatClient(
    app::RaApp *app, string remoteHost, uint16_t remotePort, string localAddress, uint32_t heartbeatInterval)
    : _app(app), _remoteHost(remoteHost), _remotePort(remotePort), _localAddress(localAddress),
      _heartbeatInterval(heartbeatInterval), _running(false) {
}

HeartbeatClient::~HeartbeatClient() {
    stop();
}

bool HeartbeatClient::init() {
    if (!_client.init()) {
        LOG(ERROR) << "Cannot initialize http client for sending heartbeats";
        return false;
    }
    return true;
}

bool HeartbeatClient::start() {
    int err = pthread_create(&_thread, NULL, &HeartbeatClient::workLoop, this);
    if (err != 0) {
        LOG(ERROR) << "Cannot start thread for heartbeat client";
        return false;
    }
    _running = true;
    LOG(INFO) << "Start sending heartbeats to '" << _remoteHost << ":" << _remotePort << "'";
    return true;
}

bool HeartbeatClient::stop() {
    if (!_running) return true;
    _running = false;
    pthread_join(_thread, NULL);
    LOG(INFO) << "Stop sending heartbeats to '" << _remoteHost << ":" << _remotePort <<"'";
    return true;
}

std::string HeartbeatClient::createRequestBody() {
    JsonObject heartbeatObject;
    ConfigWrapperPtr config = _app->getConfigWrapper();
    double configVersion = config == NULL ? 0 : config->getConfigVersion();
    JsonStringPtr address(new JsonString(_localAddress));
    JsonNumberPtr interval(new JsonNumber(_heartbeatInterval));
    JsonNumberPtr version(new JsonNumber(configVersion));
    heartbeatObject.insert(HEARTBEAT_REQ_ADDRESS_KEY, address);
    heartbeatObject.insert(HEARTBEAT_REQ_INTERVAL_KEY, interval);
    heartbeatObject.insert(HEARTBEAT_REQ_VERSION_KEY, version);
    return heartbeatObject.dump();
}

bool HeartbeatClient::parseResponseBody(const std::string& body, map<string, string>& hostIds) {
    JsonPtr json = Json::load(body);
    JsonArrayPtr array = dynamic_pointer_cast<JsonArray>(json);
    if (json == NULL || array == NULL) {
        LOG(ERROR) << "Cannot load response, raw response: " << body;
        return false;
    }
    JsonArray::iterator iter;
    for (iter = array->begin(); iter != array->end(); iter++) {
        JsonObjectPtr object = dynamic_pointer_cast<JsonObject>(*iter);
        if (object == NULL) continue;
        map<string, string> hostInfo;
        if(!fromJson(object, hostInfo)) continue;
        const string& id = hostInfo[HEARTBEAT_RESP_ID_KEY];
        const string& addr = hostInfo[HEARTBEAT_RESP_ADDRESS_KEY];
        if (id.empty() || addr.empty()) continue;
        hostIds[addr] = id;
    }
    return true;
}

void* HeartbeatClient::workLoop(void *arg) {
    HeartbeatClient *client = (HeartbeatClient *)arg;
    const string& remoteHost = client->_remoteHost;
    uint16_t remotePort = client->_remotePort;
    int64_t lastTime = 0, curruntTime = 0;
    while (client->_running) {
        // wait for next heartbeat
        curruntTime = Util::currentTimeInSeconds();
        if (curruntTime - lastTime < client->_heartbeatInterval) {
            sleep(1);
            continue;
        }
        lastTime = Util::currentTimeInSeconds();
        // send heartbeat
        VLOG(1) << "Send heartbeat to '" << remoteHost << ":" << remotePort <<"' at time " << curruntTime;
        string requestBody = client->createRequestBody();
        HttpRequest request(HTTP_REQ_POST, remoteHost, remotePort, HEARTBEAT_API_PATH, requestBody);
        request.headers[HTTP_API_CONTENT_TYPE_KEY] = HTTP_API_JSON_CONTENT_TYPE;
        HttpResponse response;
        if (!client->_client.request(&request, &response)) {
            LOG(WARNING) << "Cannot send heartbeat to '" << remoteHost <<":" << remotePort <<"'";
            continue;
        }
        if (response.status != HTTP_RESP_OK) {
            LOG(ERROR) << "Error response from '" << remoteHost << ":" 
		       << remotePort <<"', status: "<< response.status 
		       <<", message: '" << response.message << "'";
            continue;
        }
        VLOG(1) <<  "Alive hosts: " << response.body;
        // parse response
        map<string, string> hostIds;
        if (client->parseResponseBody(response.body, hostIds)) {
            client->setAliveHostIds(hostIds);
        }
    }
    return NULL;
}

RA_END_NAMESPACE(service);

