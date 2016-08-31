#include <ra/util/Util.h>
#include <ra/util/StringHash.h>
#include <ra/fetcher/MetricTreeUtil.h>
#include <ra/tree/MetricTree.h>

using namespace std;
RA_USE_NAMESPACE(util);
RA_USE_NAMESPACE(fetcher);

RA_BEGIN_NAMESPACE(tree);
RA_LOG_SETUP(tree, MetricTree);

MetricTree::MetricTree() { 
}

MetricTree::~MetricTree() { 
}

const StringSet& MetricTree::getSpecs(const string& metric) const
{
    static StringSet empty;
    map<string, StringSet>::const_iterator it = _metricSpecMap.find(metric);
    if (it != _metricSpecMap.end()) {
        return it->second;
    }
    return empty;
}

bool MetricTree::getMericPath(const string& metric, fetcher::MetricPath& path) const {
    map<string, MetricPath>::const_iterator it = _metricPathMap.find(metric);
    if (it != _metricPathMap.end()) {
        path = it->second;
        return true;
    }
    return false;
}

void MetricTree::getMetrics(const string& metricPattern, StringSet& metricSet) const
{
    metricSet.clear();
    map<string, StringSet>::const_iterator it = _metricSpecMap.begin();
    while(it != _metricSpecMap.end()) {
        const string& metric = it->first;
        if (Util::isPatternMatch(metricPattern, metric)) {
            metricSet.insert(metric);
        }
        ++it;
    }
}

void MetricTree::addPath(const fetcher::MetricPath& path, const std::string& spec)
{
    vector<string> labelVec = MetricTreeUtil::pathToLabelVec(path);
    // remove root node
    if (!labelVec.empty() && labelVec[0] == METRIC_ROOT_NODE_LABEL) {
        labelVec.erase(labelVec.begin());
    }
    string metric = Util::joinString(labelVec, METRIC_NODE_PATH_SEP);
    if (!metric.empty()) {
        _metricSpecMap[metric].insert(spec);
        updateMetricPath(metric, path);
    }
}

void MetricTree::updateMetricPath(const std::string& metric, const fetcher::MetricPath& path) {
    map<string, MetricPath>::iterator it = _metricPathMap.find(metric);
    if (it != _metricPathMap.end()) {
        const fetcher::MetricPath& oldPath = it->second;
        if (MetricTreeUtil::comparePath(oldPath, path) == 0) {
            LOG(WARNING) << "Find duplicate metric " << metric;
        }
    }
    _metricPathMap[metric] = path;
}

void MetricTree::filterTree(const tr1::function<bool (const string&)>& isMonitored,
                            int32_t& eraseCount, int32_t& keepCount)
{
    eraseCount = 0;
    keepCount = 0;
    map<string, StringSet>::iterator it = _metricSpecMap.begin();
    while (it != _metricSpecMap.end()) {
        const string& metric = it->first;
        if (!isMonitored(metric)) {
            _metricPathMap.erase(metric);
            _metricSpecMap.erase(it++);
            ++eraseCount;
        }
        else {
            ++it;
            ++keepCount;
        }
    }
    LOG(INFO) << "After filter tree, keep count[" 
	      << keepCount <<"], erase count[" << eraseCount <<"]";
}

uint64_t MetricTree::getHashValue() const
{
    string allMetricStr = "";
    static const size_t defaultMetricLen = 25;
    allMetricStr.reserve(_metricSpecMap.size() * defaultMetricLen);

    map<string, StringSet>::const_iterator it = _metricSpecMap.begin();
    for (; it != _metricSpecMap.end(); ++it) {
        const string& metric = it->first;
        allMetricStr.append(metric);
    }
    return StringHash::hashString64(allMetricStr.c_str());
}

RA_END_NAMESPACE(tree);

