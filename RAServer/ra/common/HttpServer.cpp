#include <sys/queue.h>
#include <event2/buffer.h>
#include <event2/keyvalq_struct.h>
#include <ra/util/Util.h>
#include <ra/common/CommonDefine.h>
#include <ra/common/Log.h>
#include <ra/common/HttpServer.h>
#include <ra/common/HttpClient.h>

RA_BEGIN_NAMESPACE(common);
RA_LOG_SETUP(common, HttpServer);
RA_USE_NAMESPACE(util);
using namespace std;

/// HttpRequestHandler

void HttpRequestHandler::sendResponse(HttpResponse *response, int status, const std::string& message, const std::string& body) {
    response->status = status;
    response->message.assign(message);
    if (body.empty()) {
        response->body.assign(message);
    } else {
        response->body.assign(body);
    }
}

/// InternalRequestHandler

class InternalRequestHandler {
public:
    InternalRequestHandler(HttpServer *server, HttpRequestHandler *userHandler)
            : _server(server), _userHandler(userHandler) { }
    virtual ~InternalRequestHandler() { }

    virtual void handle(struct evhttp_request *request);
    void exitLoop();

protected:
    static bool convertEvhttpRequest(struct evhttp_request* request, HttpRequest* httpRequest);
    static void sendResponse(struct evhttp_request* request, HttpResponse* httpResponse);
    static void sendResponse(struct evhttp_request *request, int status, const char *message, struct evbuffer *databuf = NULL);

protected:
    RA_LOG_DECLARE();
    HttpServer *_server;
    HttpRequestHandler *_userHandler;
};

RA_LOG_SETUP(common, InternalRequestHandler);

bool InternalRequestHandler::convertEvhttpRequest(struct evhttp_request* request, HttpRequest* httpRequest) {
    httpRequest->method = (enum HTTP_REQUEST_TYPE)evhttp_request_get_command(request);
    const struct evhttp_uri *uri = evhttp_request_get_evhttp_uri(request);
    if (uri == NULL) {
        LOG(WARNING) << "Uri is NULL";
        return false;
    }
    const char *path = evhttp_uri_get_path(uri);
    if (path) httpRequest->uri.path.assign(path);
    struct evkeyvalq arguments;
    TAILQ_INIT(&arguments);
    const char *query = evhttp_uri_get_query(uri);
    if (query) {
        if (evhttp_parse_query_str(query, &arguments) != 0) {
            LOG(WARNING) << "Cannot parse query '" << query << "'";
        }
    }
    struct evkeyval *argument;
    TAILQ_FOREACH(argument, &arguments, next) {
        string key(argument->key), value(argument->value);
        httpRequest->uri.arguments[key] = value;
    }
    evhttp_clear_headers(&arguments);
    struct evbuffer *body = evhttp_request_get_input_buffer(request);
    size_t bodyLen = evbuffer_get_length(body);
    char *buffer = (char *)malloc(bodyLen);
    if (buffer == NULL) {
        LOG(WARNING) << "Cannot allocate memory of size '" << bodyLen << "'";
        return false;
    }
    evbuffer_copyout(body, buffer, bodyLen);
    httpRequest->body.assign(buffer, bodyLen);
    free(buffer);
    return true;
}

void InternalRequestHandler::sendResponse(struct evhttp_request* request, HttpResponse* httpResponse) {
    map<string, string>::iterator iter;
    struct evkeyvalq *headers = evhttp_request_get_output_headers(request);
    for (iter = httpResponse->headers.begin(); iter != httpResponse->headers.end(); iter++) {
        evhttp_add_header(headers, iter->first.c_str(), iter->second.c_str());
    }
    struct evbuffer *responseBuffer = evbuffer_new();
    if (responseBuffer == NULL) {
        LOG(WARNING) << "Cannot create response buffer";
        sendResponse(request, HTTP_RESP_INTERNAL, "Internal Server Error");
        return;
    }
    evbuffer_add(responseBuffer, httpResponse->body.c_str(), httpResponse->body.size());
    sendResponse(request, httpResponse->status, httpResponse->message.c_str(), responseBuffer);
    evbuffer_free(responseBuffer);
}

void InternalRequestHandler::sendResponse(struct evhttp_request *request, int status, const char *message, struct evbuffer *databuf) {
    evhttp_send_reply(request, status, message, databuf);
}

void InternalRequestHandler::exitLoop() {
    event_base_loopexit(_server->_eventBase, NULL);
}

void InternalRequestHandler::handle(struct evhttp_request* request) {
    HttpRequest httpRequest;
    HttpResponse httpResponse;
    if (!convertEvhttpRequest(request, &httpRequest)) {
        sendResponse(request, HTTP_RESP_INTERNAL, "Internal Server Error");
        return;
    }
    if (_userHandler == NULL) {
        LOG(WARNING) << "User handler is NULL";
        sendResponse(request, HTTP_RESP_NOTFOUND, "User Handler is NULL");
        return;
    }
    _userHandler->process(&httpRequest, &httpResponse);
    sendResponse(request, &httpResponse);
}

/// StopServerHandler

class StopServerHandler: public InternalRequestHandler {
public:
    StopServerHandler(HttpServer *server)
            : InternalRequestHandler(server, NULL) { }

    virtual void handle(evhttp_request* request) {
        LOG(INFO) << "Get request to stop server";
        sendResponse(request, HTTP_RESP_OK, "OK Stop Server");
        exitLoop();
    }
};

/// NotFoundHandler

class NotFoundHandler: public InternalRequestHandler {
public:
    NotFoundHandler(HttpServer *server)
            : InternalRequestHandler(server, NULL) { }

    virtual void handle(evhttp_request* request) {
        LOG(INFO) << "Cannot find handler for uri '" << evhttp_request_get_uri(request)  <<"'";
        sendResponse(request, HTTP_RESP_NOTFOUND, "Not Found");
    }
};

/// HttpServer

HttpServer::HttpServer()
    : _eventBase(NULL), _httpServer(NULL), _isRunning(false) {
    _stopServerPath = "/" + Util::randomAlphabet(16);
}

HttpServer::~HttpServer() {
    stop();
    if (_httpServer) {
        evhttp_free(_httpServer);
        _httpServer = NULL;
    }
    if (_eventBase) {
        event_base_free(_eventBase);
        _eventBase = NULL;
    }
    vector<InternalRequestHandler *>::iterator iter;
    for (iter = _internalHandlers.begin(); iter != _internalHandlers.end(); iter++) {
        delete (*iter);
    }
    _internalHandlers.clear();
}

bool HttpServer::init(string host, uint16_t port) {
    _host = host;
    _port = port;
    _eventBase = event_base_new();
    if (_eventBase == NULL) {
        LOG(ERROR) << "Cannot create an event base";
        return false;
    }
    _httpServer = evhttp_new(_eventBase);
    if (_httpServer == NULL) {
        LOG(ERROR) << "Cannot create a HTTP server";
        return false;
    }
    if (evhttp_bind_socket(_httpServer, host.c_str(), port) != 0) {
        LOG(ERROR) << "Cannot bind address: "
		   << host <<":"<< port;
        return false;
    }
    InternalRequestHandler *stopServerHandler = new StopServerHandler(this);
    InternalRequestHandler *notFoundHandler = new NotFoundHandler(this);
    _internalHandlers.push_back(stopServerHandler);
    _internalHandlers.push_back(notFoundHandler);
    evhttp_set_gencb(_httpServer, HttpServer::handleRequest, notFoundHandler);
    if (evhttp_set_cb(_httpServer, _stopServerPath.c_str(), HttpServer::handleRequest, stopServerHandler) != 0) {
        LOG(ERROR) << "Cannot set up stop handler for http server";
        return false;
    }
    LOG(INFO) << "Successfully initialize http server '" << host << ":" << port <<"'";
    return true;
}

bool HttpServer::start() {
    if (pthread_create(&_thread, NULL, &HttpServer::eventLoop, this) != 0) {
        LOG(ERROR) << "Cannot start thread for http server";
        return false;
    }
    _isRunning = true;
    LOG(INFO) << "Successfully start http server '" << _host << ":" << _port <<"'";
    LOG(INFO) << "Secret stop path is 'http://"<< _host <<":" << _port <<  _stopServerPath;
    return true;
}

void *HttpServer::eventLoop(void* arg) {
    HttpServer *server = (HttpServer *)arg;
    event_base_dispatch(server->_eventBase);
    return NULL;
}

bool HttpServer::stop() {
    if (!_isRunning) return true;
    LOG(INFO) << "Try to stop http server '" << _host << ":" << _port <<"'";
    HttpClient client;
    if (!client.init()) {
        LOG(WARNING) << "Cannot initialize client for sending stop request";
        return false;
    }
    HttpRequest request(HTTP_REQ_GET, _host, _port, _stopServerPath);
    HttpResponse response;
    if (!client.request(&request, &response)) {
        LOG(WARNING) << "Cannot send stop request";
        return false;
    }
    if (response.status != HTTP_RESP_OK) {
        LOG(WARNING) << "Cannot stop http server";
        return false;
    }
    _isRunning = false;
    pthread_join(_thread, NULL);
    LOG(INFO) << "Successfully stop http server '" << _host <<": " << _port << "'";
    return true;
}

void HttpServer::handleRequest(struct evhttp_request* request, void* arg) {
    InternalRequestHandler *internalHandler = (InternalRequestHandler *)arg;
    if (internalHandler) internalHandler->handle(request);
}

bool HttpServer::registerHandler(string path, HttpRequestHandler* handler) {
    if (handler == NULL) {
        LOG(WARNING) << "Handler for path '" << path << "' is NULL";
        return false;
    }
    InternalRequestHandler *internalHandler = new InternalRequestHandler(this, handler);
    if (evhttp_set_cb(_httpServer, path.c_str(), HttpServer::handleRequest, internalHandler) != 0) {
        delete internalHandler;
        LOG(WARNING) << "Cannot register handler for path '" << path << "'";
        return false;
    }
    _internalHandlers.push_back(internalHandler);
    LOG(INFO) << "Successfully register handler for path '" << path << "'";
    return true;
}

RA_END_NAMESPACE(common);
