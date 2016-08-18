#ifndef RA_HEARTBEAT_CLIENT_H
#define RA_HEARTBEAT_CLIENT_H

#include <pthread.h>
#include <ra/util/ThreadLock.h>
#include <ra/common/CommonDefine.h>
#include <ra/common/Log.h>
#include <ra/common/HttpClient.h>

RA_BEGIN_NAMESPACE(app);
class RaApp;
RA_END_NAMESPACE(app);

RA_BEGIN_NAMESPACE(service);

class HeartbeatClient {
public:
    HeartbeatClient(app::RaApp *app, std::string remoteHost, uint16_t remotePort, std::string localAddress,
                    uint32_t heartbeatInterval = HEARTBEAT_DEFAULT_INTERVAL);   // heartbeatInterval: seconds
    virtual ~HeartbeatClient();

    bool init();
    bool start();
    bool stop();
    // hostIds: IP address -> host id
    void getAliveHostIds(std::map<std::string, std::string>& hostIds) const {
        util::ScopedLock lock(_lockHostIds);
        hostIds = _aliveHostIds;
    }

private:
    static void *workLoop(void *arg);
    std::string createRequestBody();
    bool parseResponseBody(const std::string& body, std::map<std::string, std::string>& hostIds);

    void setAliveHostIds(const std::map<std::string, std::string>& hostIds) {
        util::ScopedLock lock(_lockHostIds);
        _aliveHostIds = hostIds;
    }

private:
    RA_LOG_DECLARE();
    pthread_t _thread;
    common::HttpClient _client;
    app::RaApp *_app;
    std::string _remoteHost;
    uint16_t _remotePort;
    uint16_t _localPort;
    std::string _localAddress;
    uint32_t _heartbeatInterval;
    bool _running;
    mutable util::Mutex _lockHostIds;
    std::map<std::string, std::string> _aliveHostIds;
};

RA_END_NAMESPACE(service);

#endif /* RA_HEARTBEAT_CLIENT_H */

