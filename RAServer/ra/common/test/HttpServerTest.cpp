#include <sstream>
#include <errno.h>
#include <unistd.h>
#include <event2/event.h>
#include <cppunit/TestAssert.h>
#include <ra/test/test.h>
#include <ra/util/Util.h>
#include <ra/common/HttpServer.h>
#include <ra/common/test/HttpServerTest.h>

RA_BEGIN_NAMESPACE(common);
RA_USE_NAMESPACE(util);
using namespace std;

CPPUNIT_TEST_SUITE_REGISTRATION(HttpServerTest);

class EchoHandler: public HttpRequestHandler {
public:
    virtual void process(const HttpRequest* request, HttpResponse* response) {
        LOG(INFO) << "method " << request->method << ", uri '" 
		  << request->uri.serialize() <<  "', body '"
		  << request->body.substr(0, 80) << "'";
        response->status = HTTP_RESP_OK;
        response->message = "OK";
        response->body = request->body;
    }
};

HttpServerTest::HttpServerTest() {
}

HttpServerTest::~HttpServerTest() {
}

void HttpServerTest::LibeventLogCallback(int severity, const char* msg) {
    LOG(INFO) << "libevent: " << msg;
}

void HttpServerTest::setUp() {
    event_set_log_callback(HttpServerTest::LibeventLogCallback);
}

void HttpServerTest::tearDown() {
}

void HttpServerTest::testStartAndStop() {
    HttpServer server;
    CPPUNIT_ASSERT(server.init("127.0.0.1", 18080));
    CPPUNIT_ASSERT(server.start());
    usleep(10000);
    CPPUNIT_ASSERT(server.stop());
}

void HttpServerTest::testRequestHandler() {
    HttpServer server;
    CPPUNIT_ASSERT(server.init("127.0.0.1", 18080));
    string path = "/handler";
    string url = "http://127.0.0.1:18080";
    EchoHandler handler;
    CPPUNIT_ASSERT(server.registerHandler(path, &handler));
    CPPUNIT_ASSERT_FAIL(server.registerHandler(path, &handler));
    CPPUNIT_ASSERT(server.start());

    HttpClient client;
    CPPUNIT_ASSERT(client.init());
    HttpResponse response;
    CPPUNIT_ASSERT(client.get(url + path + "?key=val", &response));
    CPPUNIT_ASSERT_EQUAL(HTTP_RESP_OK, response.status);

    string body = "hello";
    CPPUNIT_ASSERT(client.post(url + path + "?key=val", body, &response));
    CPPUNIT_ASSERT_EQUAL(HTTP_RESP_OK, response.status);
    CPPUNIT_ASSERT_EQUAL(body, response.body);
    string bodyLenStr = response.headers["Content-Length"];
    stringstream ss(bodyLenStr);
    size_t bodyLen;
    ss >> bodyLen;
    CPPUNIT_ASSERT_EQUAL(body.size(), bodyLen);

    CPPUNIT_ASSERT(server.stop());
}

RA_END_NAMESPACE(common);

