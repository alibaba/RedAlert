#include <sstream>
#include <tr1/functional>
#include <ra/util/Util.h>
#include <ra/util/FileUtil.h>
#include <ra/fetcher/MetricDataNode.h>
#include <ra/fetcher/MetricTreeUtil.h>
#include "DummyMetricFetcher.h"

RA_BEGIN_NAMESPACE(fetcher);
using namespace std;
RA_USE_NAMESPACE(util);

DummyMetricFetcher::DummyMetricFetcher()
    : _treeDepth(3) {
}

DummyMetricFetcher::~DummyMetricFetcher() {
}

bool DummyMetricFetcher::init(const OptionMap &options) {
    OptionMap::const_iterator iter = options.find("tree_depth");
    if (iter != options.end() && !iter->second.empty()) {
        stringstream ss(iter->second);
        ss >> _treeDepth;
    }
    return true;
}

void DummyMetricFetcher::close() {
}

void DummyMetricFetcher::makeTree(MetricNodePtr &root, uint32_t depth, uint32_t& count) {
    if (depth == 0) return;
    stringstream label1, label2;
    label1 << "node" << count++;
    MetricNodePtr left(new MetricNode(label1.str()));
    label2 << "node" << count++;
    MetricNodePtr right(new MetricNode(label2.str()));
    makeTree(left, depth - 1, count);
    makeTree(right, depth - 1, count);
    root->addChild(left);
    root->addChild(right);
}

MetricNodePtr DummyMetricFetcher::allocTree(int64_t start, int64_t end) {
    MetricNodePtr root(new MetricNode(METRIC_ROOT_NODE_LABEL));
    uint32_t count = 1;
    makeTree(root, _treeDepth, count);
    return root;
}

bool DummyMetricFetcher::attachDataNode(
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

RA_END_NAMESPACE(fetcher);
