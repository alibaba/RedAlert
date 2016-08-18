#include <errno.h>
#include <unistd.h>
#include <cppunit/TestAssert.h>
#include <ra/test/test.h>
#include <ra/test_util/SimpleMetricFetcher.h>
#include <ra/util/Util.h>
#include <ra/fetcher/MetricTreeUtil.h>
#include <ra/fetcher/test/MetricFetcherTest.h>

RA_BEGIN_NAMESPACE(fetcher);
RA_LOG_SETUP(fetcher, MetricFetcherTest);
RA_USE_NAMESPACE(test_util);
RA_USE_NAMESPACE(util);
using namespace std;

CPPUNIT_TEST_SUITE_REGISTRATION(MetricFetcherTest);

MetricFetcherTest::MetricFetcherTest() { 
}

MetricFetcherTest::~MetricFetcherTest() { 
}

void MetricFetcherTest::setUp() { 
    RA_LOG(INFO, "setUp!");
}

void MetricFetcherTest::tearDown() { 
    RA_LOG(INFO, "tearDown!");
}

void MetricFetcherTest::testAllocTree() { 
    MetricFetcherPtr fetcher(new SimpleMetricFetcher());
    MetricNodePtr root = fetcher->allocTree();
    CPPUNIT_ASSERT(root != NULL);
    cout << root->toDebugString() << endl;
}

void MetricFetcherTest::testCloneTree() {
    MetricFetcherPtr fetcher(new SimpleMetricFetcher());
    MetricNodePtr root = fetcher->allocTree();
    CPPUNIT_ASSERT(root != NULL);
    cout << root->toDebugString() << endl;
    MetricNodePtr clone = root->cloneTree();
    CPPUNIT_ASSERT(clone != NULL);
    cout << clone->toDebugString() << endl;
}

void MetricFetcherTest::testRetrieve() { 
    MetricFetcherPtr fetcher(new SimpleMetricFetcher());
    MetricNodePtr root = fetcher->allocTree();
    CPPUNIT_ASSERT(root != NULL);
    cout << root->toDebugString() << endl;
    int64_t now = Util::currentTimeInSeconds();
    MetricNodePtr metricTree = fetcher->retrieve(root, now - 100000000000000, now, 100000000000);
    CPPUNIT_ASSERT(metricTree != NULL);
    cout << metricTree->toDebugString() << endl;
}

bool MetricFetcherTest::printPath(const MetricPath& path) {
    vector<string> labels = MetricTreeUtil::pathToLabelVec(path);
    RA_LOG(INFO, "Path: %s", Util::joinString(labels, ".").c_str());
    return true;
}

void MetricFetcherTest::testTraverse() {
    OptionMap options;
    options["tree_depth"] = "5";
    MetricFetcherPtr fetcher(new SimpleMetricFetcher());
    CPPUNIT_ASSERT(fetcher->init(options));
    MetricNodePtr root = fetcher->allocTree();
    CPPUNIT_ASSERT(root != NULL);
    bool ok = MetricTreeUtil::traverseTree(root, std::tr1::bind(&MetricFetcherTest::printPath, this, std::tr1::placeholders::_1));
    CPPUNIT_ASSERT(ok);
}

RA_END_NAMESPACE(fetcher);

