#include <sstream>
#include <tr1/functional>
#include <ra/common/Common.h>
#include <ra/common/Log.h>
#include <ra/common/HttpClient.h>
#include <ra/fetcher/MetricFetcher.h>
#include <ra/fetcher/MetricDataNode.h>
#include <ra/fetcher/MetricTreeUtil.h>
#include <ra/test_util/SimpleMetricFetcher.h>

RA_BEGIN_NAMESPACE(test_util);
RA_USE_NAMESPACE(fetcher);
using namespace std;

SimpleMetricFetcher::SimpleMetricFetcher()
    : _treeDepth (3) {

}

SimpleMetricFetcher::~SimpleMetricFetcher() {

}

bool SimpleMetricFetcher::init(const OptionMap &options) {
    OptionMap::const_iterator iter = options.find("tree_depth");
    if (iter != options.end() && !iter->second.empty()) {
        stringstream ss(iter->second);
        ss >> _treeDepth;
    }
    return true;
}

void SimpleMetricFetcher::close() {
}

MetricNodePtr SimpleMetricFetcher::makeTree(uint32_t depth, uint32_t& count) {
    if (depth == 0) return MetricNodePtr();
    stringstream ss;
    ss << "node" << count++;
    MetricNodePtr root(new MetricNode(ss.str()));
    if (depth == 1) return root;
    MetricNodePtr left = makeTree(depth - 1, count);
    MetricNodePtr right = makeTree(depth - 1, count);
    root->addChild(left);
    root->addChild(right);
    return root;
}

MetricNodePtr SimpleMetricFetcher::allocTree(int64_t start, int64_t end) {
    uint32_t count = 1;
    return makeTree(_treeDepth, count);
}

bool SimpleMetricFetcher::attachDataNode(
    const MetricPath& path, int64_t start, int64_t end, int64_t step) {
    if (path.empty()) return true;
    MetricDataPtr data(new MetricData("localhost"));
    for (int64_t t = start; t < end; t += step) {
        data->times->push_back(t);
        data->values->push_back(t / 1e12);
    }
    MetricNodePtr dataNode(new MetricDataNode(data));
    const MetricNodePtr &parent = *path.rbegin();
    parent->addChild(dataNode);
    return true;
}

RA_END_NAMESPACE(test_util);
