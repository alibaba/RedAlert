#ifndef RA_LOAD_CONFIG_HANDLER_H
#define RA_LOAD_CONFIG_HANDLER_H

#include <ra/common/CommonDefine.h>
#include <ra/common/Log.h>
#include <ra/common/HttpServer.h>

RA_BEGIN_NAMESPACE(app);
class RaApp;
RA_END_NAMESPACE(app);

RA_BEGIN_NAMESPACE(service);

class LoadConfigHandler: public common::HttpRequestHandler {
public:
    LoadConfigHandler(app::RaApp *app);
    virtual ~LoadConfigHandler();

    virtual void process(const common::HttpRequest* request, common::HttpResponse* response);

private:
    bool doLoadConfig(const std::string& configPathStr);

private:
    RA_LOG_DECLARE();
    ra::app::RaApp *_app;
};

RA_END_NAMESPACE(service);

#endif /* RA_LOAD_CONFIG_HANDLER_H */

