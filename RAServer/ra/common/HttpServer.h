#ifndef RA_HTTP_SERVER_H
#define RA_HTTP_SERVER_H

#include <map>
#include <string>
#include <pthread.h>
#include <event2/event.h>
#include <event2/http.h>
#include <ra/common/CommonDefine.h>
#include <ra/common/Log.h>
#include <ra/common/HttpClient.h>

RA_BEGIN_NAMESPACE(common);

class HttpRequestHandler {
public:
    virtual ~HttpRequestHandler() { }
    virtual void process(const HttpRequest *request, HttpResponse *response) = 0;
    void sendResponse(HttpResponse *response, int status, const std::string& message, const std::string& body = std::string());
};

class InternalRequestHandler;

class HttpServer {
public:
    HttpServer();
    virtual ~HttpServer();

    bool init(std::string host, uint16_t port);
    bool start();
    bool stop();
    bool registerHandler(std::string path, HttpRequestHandler *handler);

private:
    static void *eventLoop(void *arg);
    static void handleRequest(struct evhttp_request *request, void *arg);

private:
    friend class InternalRequestHandler;
    RA_LOG_DECLARE();

    pthread_t _thread;
    std::string _host;
    uint16_t _port;
    struct event_base *_eventBase;
    struct evhttp *_httpServer;
    std::string _stopServerPath;
    bool _isRunning;
    std::vector<InternalRequestHandler *> _internalHandlers;
};

RA_TYPEDEF_PTR(HttpServer);

RA_END_NAMESPACE(common);

#endif /* RA_HTTP_SERVER_H */

