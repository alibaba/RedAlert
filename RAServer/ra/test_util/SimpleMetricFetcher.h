#ifndef RA_SIMPLEMETRICFETCHER_H
#define RA_SIMPLEMETRICFETCHER_H

#include <ra/common/Common.h>
#include <ra/common/Log.h>
#include <ra/fetcher/MetricFetcher.h>

RA_BEGIN_NAMESPACE(test_util);

class SimpleMetricFetcher: public fetcher::MetricFetcher
{
public:
    SimpleMetricFetcher();
    virtual ~SimpleMetricFetcher();

    virtual bool init(const fetcher::OptionMap &options);
    virtual void close();
    virtual fetcher::MetricNodePtr allocTree(int64_t start, int64_t end);

protected:
    fetcher::MetricNodePtr makeTree(uint32_t depth, uint32_t& count);
    virtual bool attachDataNode(const fetcher::MetricPath& path, int64_t start, int64_t end, int64_t step);

private:
    int32_t _treeDepth;
};

RA_TYPEDEF_PTR(SimpleMetricFetcher);

RA_END_NAMESPACE(test_util);

#endif //RA_SIMPLEMETRICFETCHER_H
