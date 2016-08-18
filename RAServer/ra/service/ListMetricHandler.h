#ifndef RA_LIST_METRIC_HANDLER_H
#define RA_LIST_METRIC_HANDLER_H

#include <ra/common/CommonDefine.h>
#include <ra/common/Log.h>
#include <ra/common/HttpServer.h>

RA_BEGIN_NAMESPACE(service);

class ListMetricHandler: public common::HttpRequestHandler {
public:
    ListMetricHandler(const tree::TreeManager *treeManager);
    virtual ~ListMetricHandler();

    virtual void process(const common::HttpRequest* request, common::HttpResponse* response);

private:
    RA_LOG_DECLARE();
    const tree::TreeManager *_treeManager;
};

RA_END_NAMESPACE(service);

#endif /* RA_LIST_METRIC_HANDLER_H */

