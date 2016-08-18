#ifndef RA_HTTPCLIENT_H
#define RA_HTTPCLIENT_H

#include <event2/event.h>
#include <event2/http.h>
#include <event2/http_struct.h>
#include <ra/common/CommonDefine.h>
#include <ra/common/Log.h>

RA_BEGIN_NAMESPACE(common);

enum HTTP_REQUEST_TYPE {
	HTTP_REQ_GET     = 1 << 0,
	HTTP_REQ_POST    = 1 << 1,
	HTTP_REQ_HEAD    = 1 << 2,
	HTTP_REQ_PUT     = 1 << 3,
	HTTP_REQ_DELETE  = 1 << 4,
	HTTP_REQ_OPTIONS = 1 << 5,
	HTTP_REQ_TRACE   = 1 << 6,
	HTTP_REQ_CONNECT = 1 << 7,
	HTTP_REQ_PATCH   = 1 << 8
};

#define HTTP_RESP_OK                 200
#define HTTP_RESP_NOCONTENT          204
#define HTTP_RESP_MOVEPERM           301
#define HTTP_RESP_MOVETEMP           302
#define HTTP_RESP_NOTMODIFIED        304
#define HTTP_RESP_BADREQUEST         400
#define HTTP_RESP_NOTFOUND           404
#define HTTP_RESP_BADMETHOD          405
#define HTTP_RESP_ENTITYTOOLARGE     413
#define HTTP_RESP_EXPECTATIONFAILED  417
#define HTTP_RESP_INTERNAL           500
#define HTTP_RESP_NOTIMPLEMENTED     501
#define HTTP_RESP_SERVUNAVAIL        503

class HttpUri {
public:
    HttpUri() { }
    HttpUri(std::string path_): path(path_) { }
    std::string serialize() const;
    static std::string encode(const std::string &input);
    static std::string decode(const std::string &input);

public:
    std::string path;
    std::map<std::string, std::string> arguments;
};

class HttpRequest {
public:
    HttpRequest() { }
    HttpRequest(enum HTTP_REQUEST_TYPE method_, std::string host_, uint16_t port_ = 80,
                std::string path_ = "/", std::string body_ = "")
        : method(method_), host(host_), port(port_), uri(path_), body(body_) { }

public:
    enum HTTP_REQUEST_TYPE method;
    std::string host;
    uint16_t port;
    HttpUri uri;
    std::map<std::string, std::string> headers;
    std::string body;
};

class HttpResponse {
public:
    HttpResponse() : status(0) { }

public:
    int status;
    std::string message;
    std::map<std::string, std::string> headers;
    std::string body;
};

class HttpClient {
public:
    HttpClient(bool persistentConn = true, uint32_t timeout = 1U, uint32_t retries = 1U);     // timeout: seconds
    virtual ~HttpClient();

private:
    HttpClient(const HttpClient &);
    HttpClient& operator=(const HttpClient &);

public:
    bool init();
    bool request(const HttpRequest *request, HttpResponse *response);
    bool request(enum HTTP_REQUEST_TYPE method, std::string url,
                 const std::map<std::string, std::string> &headers,
                 const std::string &body, HttpResponse *response);
    bool get(std::string url, HttpResponse *response);
    bool post(std::string url, const std::string &body, HttpResponse *response);

private:
    static void httpRequestDone(struct evhttp_request *req, void *ctx);
    struct evhttp_connection *getConnection(std::string host, uint16_t port);
    struct evhttp_connection *createConnection(std::string host, uint16_t port);

private:
    RA_LOG_DECLARE();
    friend class HttpClientTest;

    bool _persistentConn;
    int32_t _timeout;
    int32_t _retries;
    struct event_base *_eventBase;
    std::map<std::string, struct evhttp_connection *> _connectionPool;
};

RA_TYPEDEF_PTR(HttpClient);

RA_END_NAMESPACE(common);

#endif //RA_HTTPCLIENT_H
