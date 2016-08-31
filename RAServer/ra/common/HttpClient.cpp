#include <ctime>
#include <sstream>
#include <sys/queue.h>
#include <jansson.h>
#include <event2/buffer.h>
#include <event2/keyvalq_struct.h>
#include <ra/common/HttpClient.h>

#ifdef DEBUG
#include <event2/http_struct.h>
#endif

RA_BEGIN_NAMESPACE(common);
RA_LOG_SETUP(common, HttpClient);
using namespace std;

std::string HttpUri::encode(const std::string& input) {
    string output;
    char *buffer = evhttp_uriencode(input.c_str(), input.size(), 0);
    if (buffer) output.assign(buffer);
    free(buffer);
    return output;
}

std::string HttpUri::decode(const std::string& input) {
    string output;
    size_t size = 0;
    char *buffer = evhttp_uridecode(input.c_str(), 0, &size);
    if (buffer) output.assign(buffer, size);
    free(buffer);
    return output;
}

string HttpUri::serialize() const {
    stringstream ss;
    ss << path;
    if (!arguments.empty()) ss << "?";
    map<string, string>::const_iterator iter;
    for (iter = arguments.begin(); iter != arguments.end(); iter++) {
        if (iter != arguments.begin()) ss << "&";
        string key = encode(iter->first);
        string value = encode(iter->second);
        if (!key.empty() && !value.empty()) {
            ss << key << "=" << value;
        }
    }
    return ss.str();
}

HttpClient::HttpClient(bool persistentConn, uint32_t timeout, uint32_t retries)
    : _persistentConn(persistentConn), _timeout(timeout), _retries(retries), _eventBase(NULL) {
}

HttpClient::~HttpClient() {
    map<string, struct evhttp_connection *>::iterator iter;
    for (iter = _connectionPool.begin(); iter != _connectionPool.end(); iter++) {
        evhttp_connection_free(iter->second);
    }
    _connectionPool.clear();
    event_base_free(_eventBase);
}

bool HttpClient::init() {
    _eventBase = event_base_new();
    if (_eventBase == NULL) {
        LOG(ERROR) << "Cannot create event base";
        return false;
    }
    return true;
}

evhttp_connection* HttpClient::createConnection(string host, uint16_t port) {
    if (_eventBase == NULL) {
        LOG(ERROR) << "Event base is NULL: client is not appropriately initialized";
        return NULL;
    }
    struct evhttp_connection *conn = evhttp_connection_base_new(_eventBase, NULL, host.c_str(), port);
    evhttp_connection_set_timeout(conn, _timeout);
    evhttp_connection_set_retries(conn, _retries);
    return conn;
}

struct evhttp_connection *HttpClient::getConnection(string host, uint16_t port) {
    if (!_persistentConn) {
        return createConnection(host, port);
    }
    char *keyBuffer = (char *)calloc(host.size() + 16U, 1U);
    snprintf(keyBuffer, host.size() + 16U, "%s:%d", host.c_str(), port);
    string key(keyBuffer);
    free(keyBuffer);
    struct evhttp_connection *conn;
    map<string, struct evhttp_connection *>::iterator iter = _connectionPool.find(key);
    if (iter == _connectionPool.end()) {
        conn = createConnection(host, port);
        if (conn == NULL) return NULL;
        _connectionPool[key] = conn;
    } else {
        conn = iter->second;
    }
    return conn;
}

void HttpClient::httpRequestDone(struct evhttp_request *req, void *ctx) {
    HttpResponse* response = (HttpResponse *)ctx;
    if (req == NULL) {
        LOG(WARNING) << "Request is NULL in callback, no idea what happens";
        response->status = HTTP_RESP_SERVUNAVAIL;
        return;
    }

    response->status = evhttp_request_get_response_code(req);
    struct evbuffer *body = evhttp_request_get_input_buffer(req);
    if (body) {
        size_t bodyLen = evbuffer_get_length(body);
        char *buffer = (char *)malloc(bodyLen);
        if (buffer == NULL) {
            LOG(WARNING) << "Cannot allocate memory for body buffer";
            response->status = HTTP_RESP_SERVUNAVAIL;
            return;
        }
        evbuffer_copyout(body, buffer, bodyLen);
        response->body.assign(buffer, bodyLen);
        free(buffer);
    }

    struct evkeyvalq *headers = evhttp_request_get_input_headers(req);
    if (headers) {
        struct evkeyval *header;
        TAILQ_FOREACH(header, headers, next) {
            string key(header->key), value(header->value);
            response->headers[key] = value;
        }
    }
}

bool HttpClient::request(const HttpRequest* req, HttpResponse* response) {
    stringstream url;
    url << "http://" << req->host << ":" << req->port << req->uri.serialize();
    return request(req->method, url.str(), req->headers, req->body, response);
}

bool HttpClient::request(enum HTTP_REQUEST_TYPE method, string url,
                         const map<string, string> &headers, const string& body,
                         HttpResponse* response) {
    string scratch;
    const char *host = NULL;
    int port = -1;
    const char *path = NULL;
    const char *query = NULL;
    struct evhttp_uri *evhttpUri = NULL;
    struct evhttp_connection *conn = NULL;
    struct evhttp_request *evhttpRequest = NULL;
    struct evkeyvalq *evHeaders = NULL;
    map<string, string>::const_iterator iter;

    if (response == NULL) {
        LOG(WARNING) << "Invalid argument: http response is NULL";
        goto fail;
    }
    evhttpUri = evhttp_uri_parse(url.c_str());
    if (evhttpUri == NULL) {
        LOG(WARNING) << "Invalid argument: cannot parse url " << url;
        goto fail;
    }
    host = evhttp_uri_get_host(evhttpUri);
    if (host == NULL) {
        LOG(ERROR) << "Invalid argument: no host found in url " << url;
        goto fail;
    }
    port = evhttp_uri_get_port(evhttpUri);
    if (port == -1) port = 80;
    conn = getConnection(host, port);
    if (conn == NULL) {
        LOG(ERROR) << "Cannot get connection to '" <<host <<":" <<port <<"'";
        goto fail;
    }

    evhttpRequest = evhttp_request_new(HttpClient::httpRequestDone, (void *)response);
    if (evhttpRequest == NULL) {
        LOG(ERROR) << "Cannot create http request";
        goto fail;
    }
    evHeaders = evhttp_request_get_output_headers(evhttpRequest);
    scratch.clear();
    scratch.resize(sizeof(host) + 32U);
    snprintf(&scratch[0], sizeof(host) + 32U, "%s:%d", host, port);
    evhttp_add_header(evHeaders, "Host", scratch.c_str());
    if (!_persistentConn) {
        evhttp_add_header(evHeaders, "Connection", "close");
    }
    if (!body.empty()) {
        char lenBuffer[64];
        snprintf(lenBuffer, sizeof(lenBuffer), "%zu", body.size());
        evhttp_add_header(evHeaders, "Content-Length", lenBuffer);
        struct evbuffer *bodyBuffer = evhttp_request_get_output_buffer(evhttpRequest);
        evbuffer_add(bodyBuffer, body.c_str(), body.size());
    }
    for (iter = headers.begin(); iter != headers.end(); iter++) {
        evhttp_add_header(evHeaders, iter->first.c_str(), iter->second.c_str());
    }
    path = evhttp_uri_get_path(evhttpUri);
    query = evhttp_uri_get_query(evhttpUri);
    scratch.clear();
    if (path) scratch.append(path);
    if (query) scratch.append(string("?") + query);
    if (evhttp_make_request(conn, evhttpRequest, (enum evhttp_cmd_type)method, scratch.c_str()) != 0) {
        LOG(ERROR) << "Cannot send http request to '" << host << ":" <<port << "'";
        goto fail;
    }
    event_base_dispatch(_eventBase);
    evhttp_uri_free(evhttpUri);
    return true;

fail:
    if (evhttpUri) evhttp_uri_free(evhttpUri);
    if (evhttpRequest) evhttp_request_free(evhttpRequest);
    return false;
}

bool HttpClient::get(string url, HttpResponse* response) {
    return request(HTTP_REQ_GET, url, map<string, string>(), "", response);
}

bool HttpClient::post(string url, const string& body, HttpResponse* response) {
    return request(HTTP_REQ_POST, url, map<string, string>(), body, response);
}

RA_END_NAMESPACE(common);
