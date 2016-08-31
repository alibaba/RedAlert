#ifndef RA_METRIC_FETCHER_FACTORY_H
#define RA_METRIC_FETCHER_FACTORY_H

#include <ra/common/Common.h>
#include <ra/common/Log.h>
#include <ra/fetcher/MetricFetcher.h>

RA_BEGIN_NAMESPACE(fetcher);

class MetricFetcherFactory {
public:
    static MetricFetcherPtr create(std::string type);

private:
    MetricFetcherFactory(const MetricFetcherFactory &);
    MetricFetcherFactory& operator=(const MetricFetcherFactory &);
};

RA_END_NAMESPACE(fetcher);

#endif //RA_METRIC_FETCHER_FACTORY_H
