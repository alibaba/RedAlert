#include <contrib/fetcher/RRDMetricFetcher/RRDMetricFetcher.h>
#include <contrib/fetcher/GraphiteMetricFetcher/GraphiteMetricFetcher.h>
#include <contrib/fetcher/DummyMetricFetcher/DummyMetricFetcher.h>
#include <contrib/fetcher/MetricFetcherFactory.h>

RA_BEGIN_NAMESPACE(fetcher);

MetricFetcherPtr MetricFetcherFactory::create(std::string type) {
    if (type == "RRDMetricFetcher") {
        return MetricFetcherPtr(new RRDMetricFetcher());
    }
    if (type == "GraphiteMetricFetcher") {
        return MetricFetcherPtr(new GraphiteMetricFetcher());
    }
    if (type == "DummyMetricFetcher") {
        return MetricFetcherPtr(new DummyMetricFetcher());
    }
    LOG(ERROR) << "Unknown fetcher type: " << type;
    return MetricFetcherPtr();
}

RA_END_NAMESPACE(fetcher);
