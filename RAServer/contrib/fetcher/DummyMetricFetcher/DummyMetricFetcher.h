#ifndef RA_DUMMYMETRICFETCHER_H
#define RA_DUMMYMETRICFETCHER_H

#include <ra/common/Common.h>
#include <ra/common/Log.h>
#include <ra/fetcher/MetricFetcher.h>

RA_BEGIN_NAMESPACE(fetcher);

class DummyMetricFetcher: public MetricFetcher {
public:
    DummyMetricFetcher();
    virtual ~DummyMetricFetcher();

    virtual bool init(const OptionMap &options);
    virtual void close();
    virtual MetricNodePtr allocTree(int64_t start, int64_t end);

protected:
    virtual bool attachDataNode(const MetricPath& path, int64_t start, int64_t end, int64_t step);

private:
    void makeTree(MetricNodePtr &root, uint32_t depth, uint32_t& count);

private:
    int32_t _treeDepth;
};

RA_TYPEDEF_PTR(DummyMetricFetcher);

RA_END_NAMESPACE(fetcher);

#endif /* RA_DUMMYMETRICFETCHER_H */

