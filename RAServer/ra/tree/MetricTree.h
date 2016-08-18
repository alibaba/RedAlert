#ifndef RA_METRICTREE_H
#define RA_METRICTREE_H

#include <tr1/functional>
#include <ra/common/CommonDefine.h>
#include <ra/common/Log.h>
#include <ra/fetcher/MetricNode.h>

RA_BEGIN_NAMESPACE(tree);

class MetricTree
{
public:
    MetricTree();
    ~MetricTree();
private:
    MetricTree(const MetricTree &);
    MetricTree& operator=(const MetricTree &);

public:
    size_t getTreeSize() const {
        return _metricSpecMap.size();
    }
    const StringSet& getSpecs(const std::string& metric) const;
    bool getMericPath(const std::string& metric, fetcher::MetricPath& path) const;
    void getMetrics(const std::string& metricPattern, StringSet& metricSet) const;
    void addPath(const fetcher::MetricPath& path, const std::string& spec);
    void filterTree(const std::tr1::function<bool (const std::string&)>& isMonitored, 
                    int32_t& eraseCount, int32_t& keepCount);
    uint64_t getHashValue() const;
    void setMetricRootMap(const fetcher::MetricRootMap& metricRootMap);

private:
    void updateMetricPath(const std::string& metric, const fetcher::MetricPath& path);
    
private:
    // metricPath -> amonitor specs set
    std::map<std::string, StringSet> _metricSpecMap;
    std::map<std::string, fetcher::MetricPath> _metricPathMap;

private:
    friend class MetricTreeTest;

private:
    RA_LOG_DECLARE();
};

RA_TYPEDEF_PTR(MetricTree);

RA_END_NAMESPACE(tree);

#endif //RA_METRICTREE_H
