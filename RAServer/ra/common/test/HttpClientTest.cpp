#include <errno.h>
#include <unistd.h>
#include <cppunit/TestAssert.h>
#include <ra/test/test.h>
#include <ra/util/Util.h>
#include <ra/common/HttpServer.h>
#include <ra/common/test/HttpClientTest.h>

RA_BEGIN_NAMESPACE(common);
RA_USE_NAMESPACE(util);
using namespace std;

CPPUNIT_TEST_SUITE_REGISTRATION(HttpClientTest);

class FakeHandler: public HttpRequestHandler {
public:
    FakeHandler() :status(0) { }

    virtual void process(const HttpRequest* request, HttpResponse* response) {
        LOG(INFO) << "method " << request->method 
		  << ", uri '" << request->uri.serialize() <<"', body '" 
		  << request->body.substr(0, 80) << "'";
        if (status > 0) {
            response->status = status;
            return;
        }
        response->status = HTTP_RESP_OK;
        response->message = "OK";
        response->body = request->body;
    }

public:
    int status;
};

HttpClientTest::HttpClientTest() { 
}

HttpClientTest::~HttpClientTest() { 
}

void HttpClientTest::setUp() { 
}

void HttpClientTest::tearDown() { 
}

void HttpClientTest::testInit() { 
    HttpClient client;
    CPPUNIT_ASSERT(client.init());
}

void HttpClientTest::testRequest() {
    HttpServer server;
    FakeHandler handler;
    CPPUNIT_ASSERT(server.init("127.0.0.1", 18080));
    string path = "/handler";
    string url = "http://127.0.0.1:18080";
    CPPUNIT_ASSERT(server.registerHandler(path, &handler));
    CPPUNIT_ASSERT(server.start());

    HttpClient client;
    CPPUNIT_ASSERT(client.init());
    HttpResponse response;
    string body = "hello";
    CPPUNIT_ASSERT(client.get(url + path, &response));
    CPPUNIT_ASSERT_EQUAL(HTTP_RESP_OK, response.status);
    CPPUNIT_ASSERT(client.post(url + path, body, &response));
    CPPUNIT_ASSERT_EQUAL(HTTP_RESP_OK, response.status);
    CPPUNIT_ASSERT(server.stop());
}

void HttpClientTest::testInvalidRequest() {
    HttpServer server;
    FakeHandler handler;
    CPPUNIT_ASSERT(server.init("127.0.0.1", 18080));
    string path = "/handler";
    string url = "http://127.0.0.1:18080";
    CPPUNIT_ASSERT(server.registerHandler(path, &handler));
    CPPUNIT_ASSERT(server.start());

    HttpClient client;
    CPPUNIT_ASSERT(client.init());
    HttpResponse response;
    CPPUNIT_ASSERT(client.get(url + "/not_exist_path", &response));
    CPPUNIT_ASSERT_EQUAL(HTTP_RESP_NOTFOUND, response.status);
    CPPUNIT_ASSERT(client.get("http://127.0.0.2", &response));
    CPPUNIT_ASSERT(response.status != HTTP_RESP_OK);
    CPPUNIT_ASSERT_FAIL(client.get("127.0.0.1", &response));
    CPPUNIT_ASSERT(server.stop());
}

void HttpClientTest::testLargeBody() {
    HttpServer server;
    FakeHandler handler;
    CPPUNIT_ASSERT(server.init("127.0.0.1", 18080));
    string path = "/handler";
    string url = "http://127.0.0.1:18080";
    CPPUNIT_ASSERT(server.registerHandler(path, &handler));
    CPPUNIT_ASSERT(server.start());

    HttpClient client;
    CPPUNIT_ASSERT(client.init());
    HttpResponse response;
    string body = Util::randomAlphabet(1024 * 10);
    CPPUNIT_ASSERT(client.post(url + path, body, &response));
    CPPUNIT_ASSERT_EQUAL(HTTP_RESP_OK, response.status);
    CPPUNIT_ASSERT(server.stop());
}

void HttpClientTest::testEncodedQuery() {
    HttpServer server;
    FakeHandler handler;
    CPPUNIT_ASSERT(server.init("127.0.0.1", 18080));
    string path = "/handler";
    string url = "http://127.0.0.1:18080";
    CPPUNIT_ASSERT(server.registerHandler(path, &handler));
    CPPUNIT_ASSERT(server.start());

    HttpClient client;
    CPPUNIT_ASSERT(client.init());
    HttpResponse response;
    string body = "hello";
    string query = "?query=" + HttpUri::encode("*");
    CPPUNIT_ASSERT(client.post(url + path + query, body, &response));
    CPPUNIT_ASSERT_EQUAL(HTTP_RESP_OK, response.status);
    CPPUNIT_ASSERT(server.stop());
}

RA_END_NAMESPACE(common);

