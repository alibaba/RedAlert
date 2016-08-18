#ifndef RA_METRICFECHER_H
#define RA_METRICFECHER_H

#include <ra/common/Common.h>
#include <ra/common/Log.h>
#include <ra/fetcher/MetricNode.h>
#include <ra/fetcher/MetricData.h>
#include <ra/fetcher/MetricTreeUtil.h>

RA_BEGIN_NAMESPACE(fetcher);

typedef std::map<std::string, std::string> OptionMap;
static const std::string FETCHER_OPTION_NAME_KEY = "name";
static const std::string FETCHER_OPTION_TYPE_KEY = "type";
static const std::string FETCHER_OPTION_ADDR_KEY = "address";

class MetricFetcher
{
public:
    MetricFetcher() {
    }

    virtual ~MetricFetcher() {
    }

private:
    MetricFetcher(const MetricFetcher &);
    MetricFetcher& operator=(const MetricFetcher &);

public:

    virtual bool init(const OptionMap &options) = 0;

    virtual void close() = 0;

    virtual MetricNodePtr allocTree(int64_t start = -1, int64_t end = -1) = 0;  // start, end: microseconds

    virtual MetricNodePtr retrieve(
        const MetricNodePtr &root,
        int64_t start, int64_t end, int64_t step)                               // start, end, step: microseconds
    {
        if (root == NULL) return MetricNodePtr();
        MetricNodePtr clone = root->cloneTree();
        bool ok = MetricTreeUtil::traverseTree(clone,
                std::tr1::bind(&MetricFetcher::attachDataNode, this, std::tr1::placeholders::_1, start, end, step));
        if (ok) return clone;
        return MetricNodePtr();
    }

protected:
    virtual bool attachDataNode(const MetricPath& path, int64_t start, int64_t end, int64_t step) {
        return false;
    }
};

RA_TYPEDEF_PTR(MetricFetcher);

RA_END_NAMESPACE(fetcher);

#endif //RA_METRICFECHER_H
