#ifndef RA_FAKEMETRICFETCHER_H
#define RA_FAKEMETRICFETCHER_H

#include <ra/common/Common.h>
#include <ra/common/Log.h>
#include <ra/fetcher/MetricFetcher.h>
#include <ra/test_util/SimpleMetricFetcher.h>

RA_BEGIN_NAMESPACE(test_util);

class FakeMetricFetcher: public SimpleMetricFetcher {
public:
    FakeMetricFetcher();
    virtual ~FakeMetricFetcher();

    virtual bool init(const fetcher::OptionMap& options);
    virtual void close();
    virtual fetcher::MetricNodePtr allocTree(int64_t start, int64_t end);

public:
    // for test
    bool initOK;
    bool allocOK;
    bool retrieveOK;

protected:
    virtual bool attachDataNode(const fetcher::MetricPath& path, int64_t start, int64_t end, int64_t step);
};

RA_TYPEDEF_PTR(FakeMetricFetcher);

RA_END_NAMESPACE(test_util);

#endif /* RA_FAKEMETRICFETCHER_H */

