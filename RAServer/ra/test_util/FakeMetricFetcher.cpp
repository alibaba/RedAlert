#include <ra/test_util/FakeMetricFetcher.h>

RA_BEGIN_NAMESPACE(test_util);
RA_USE_NAMESPACE(fetcher);
using namespace std;

FakeMetricFetcher::FakeMetricFetcher()
    : initOK(false), allocOK(false), retrieveOK(false) {
}

FakeMetricFetcher::~FakeMetricFetcher() {
}

bool FakeMetricFetcher::init(const OptionMap& options) {
    if (!initOK) return false;
    return SimpleMetricFetcher::init(options);
}

void FakeMetricFetcher::close() {
}

MetricNodePtr FakeMetricFetcher::allocTree(int64_t start, int64_t end) {
    if (!allocOK) return MetricNodePtr();
    return SimpleMetricFetcher::allocTree(start, end);
}

bool FakeMetricFetcher::attachDataNode(const MetricPath& path, int64_t start, int64_t end, int64_t step) {
    if (!retrieveOK) return false;
    return SimpleMetricFetcher::attachDataNode(path, start, end, step);
}

RA_END_NAMESPACE(test_util);
