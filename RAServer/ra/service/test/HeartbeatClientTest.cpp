#include <ra/test/test.h>
#include <ra/app/RaApp.h>
#include <ra/common/Json.h>
#include <ra/common/HttpServer.h>
#include <ra/config/ConfigWrapper.h>
#include <ra/service/HeartbeatClient.h>
#include <ra/service/test/HeartbeatClientTest.h>

RA_BEGIN_NAMESPACE(service);
RA_USE_NAMESPACE(app);
RA_USE_NAMESPACE(config);
RA_USE_NAMESPACE(common);
using namespace std;
using namespace std::tr1;

RA_LOG_SETUP(service, HeartbeatClientTest);
CPPUNIT_TEST_SUITE_REGISTRATION(HeartbeatClientTest);

class FakeHandler: public HttpRequestHandler {
public:
    FakeHandler() :status(0) { }

    virtual void process(const HttpRequest* request, HttpResponse* response) {
        if (request->method != HTTP_REQ_POST) {
            sendResponse(response, HTTP_RESP_BADMETHOD, "Wrong http method, expect POST");
            return;
        }
        if (status > 0) {
            response->status = status;
            return;
        }
        JsonPtr json = Json::load(request->body);
        JsonObjectPtr object = dynamic_pointer_cast<JsonObject>(json);
        JsonObject::iterator iter = object->find(HEARTBEAT_REQ_ADDRESS_KEY);
        if (iter == object->end()) {
            sendResponse(response, HTTP_RESP_BADREQUEST, "Cannot find address");
            return;
        }
        JsonStringPtr str = dynamic_pointer_cast<JsonString>(iter->second);
        const string &address = *str;
        if (address != "127.0.0.1:80") {
            sendResponse(response, HTTP_RESP_BADREQUEST, "Address in heartbeat is not 127.0.0.1:80");
            return;
        }
        response->status = HTTP_RESP_OK;
        response->message = "OK";
        response->body = "[{\"id\": \"a\", \"address\": \"127.0.0.1:80\"}, {\"id\": \"b\", \"address\": \"127.0.0.2:80\"}]";
    }

public:
    int status;

private:
    RA_LOG_DECLARE();
};

RA_LOG_SETUP(common, FakeHandler);

HeartbeatClientTest::HeartbeatClientTest() {
}

HeartbeatClientTest::~HeartbeatClientTest() {
}

void HeartbeatClientTest::setUp() {
}

void HeartbeatClientTest::tearDown() {
}

void HeartbeatClientTest::testHeartbeat() {
    HttpServer server;
    FakeHandler handler;
    CPPUNIT_ASSERT(server.init("127.0.0.1", 18082));
    CPPUNIT_ASSERT(server.registerHandler(HEARTBEAT_API_PATH, &handler));
    CPPUNIT_ASSERT(server.start());

    int32_t version = 1;
    ConfigWrapperPtr configWrapper(new ConfigWrapper());
    configWrapper->_configVersion = version;
    RaApp app;
    app.setConfigWrapper(configWrapper);
    uint32_t heartbeatInterval = 1U;
    HeartbeatClient client(&app, "127.0.0.1", 18082, "127.0.0.1:80", heartbeatInterval);
    CPPUNIT_ASSERT(client.init());
    CPPUNIT_ASSERT(client.start());
    map<string, string> hostIds;
    for (int i = 0; i < 3 && hostIds.empty(); i++) {
        sleep(heartbeatInterval * 2);
        client.getAliveHostIds(hostIds);
    }
    client.getAliveHostIds(hostIds);
    CPPUNIT_ASSERT_EQUAL((size_t)2U, hostIds.size());
    CPPUNIT_ASSERT_EQUAL(string("a"), hostIds["127.0.0.1:80"]);
    CPPUNIT_ASSERT_EQUAL(string("b"), hostIds["127.0.0.2:80"]);
    CPPUNIT_ASSERT(client.stop());
    CPPUNIT_ASSERT(server.stop());
}

RA_END_NAMESPACE(service);
