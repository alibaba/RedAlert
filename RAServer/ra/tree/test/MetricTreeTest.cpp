#include <cppunit/TestAssert.h>
#include <ra/test/test.h>
#include <ra/util/Util.h>
#include <ra/util/StringHash.h>
#include <ra/fetcher/MetricNode.h>
#include <ra/tree/test/MetricTreeTest.h>

using namespace std;
RA_USE_NAMESPACE(util);
RA_USE_NAMESPACE(fetcher);

RA_BEGIN_NAMESPACE(tree);

CPPUNIT_TEST_SUITE_REGISTRATION(MetricTreeTest);

MetricTreeTest::MetricTreeTest() { 
}

MetricTreeTest::~MetricTreeTest() { 
}

void MetricTreeTest::setUp() { 
}

void MetricTreeTest::tearDown() { 
}

static string pattern = "";
bool isMonitored(const string& str)
{
    return Util::isPatternMatch(pattern, str);
}

void MetricTreeTest::testSimpleProcess() { 
    MetricTree tree;
    CPPUNIT_ASSERT_EQUAL((size_t)0, tree._metricSpecMap.size());

    MetricNodePtr node1(new MetricNode("node1"));
    MetricNodePtr node2(new MetricNode("node2"));
    MetricNodePtr node3(new MetricNode("node3"));
    MetricNodePtr node4(new MetricNode("node4"));
    MetricPath path1, path2;
    path1.push_back(node1);
    path1.push_back(node2);
    path1.push_back(node3);

    tree.addPath(path1, "spec1");
    CPPUNIT_ASSERT_EQUAL((size_t)1, tree._metricSpecMap.size());

    path2.assign(path1.begin(), path1.end());
    path2.pop_back();
    path2.push_back(node4);
    tree.addPath(path2, "spec2");
    CPPUNIT_ASSERT_EQUAL((size_t)2, tree._metricSpecMap.size());

    tree.addPath(path2, "spec3");
    CPPUNIT_ASSERT_EQUAL((size_t)2, tree._metricSpecMap.size());

    StringSet specSet = tree.getSpecs("node1.node2.node3");
    CPPUNIT_ASSERT_EQUAL((size_t)1, specSet.size());
    CPPUNIT_ASSERT(specSet.count("spec1") == 1);

    specSet = tree.getSpecs("node1.node2.node4");
    CPPUNIT_ASSERT_EQUAL((size_t)2, specSet.size());
    CPPUNIT_ASSERT(specSet.count("spec2") == 1);
    CPPUNIT_ASSERT(specSet.count("spec3") == 1);

    specSet = tree.getSpecs("pathXXXX");
    CPPUNIT_ASSERT_EQUAL((size_t)0, specSet.size());

    StringSet metricSet;
    tree.getMetrics("*n?de*", metricSet);
    CPPUNIT_ASSERT_EQUAL((size_t)2, metricSet.size());
    CPPUNIT_ASSERT_EQUAL((size_t)1, metricSet.count("node1.node2.node3"));
    CPPUNIT_ASSERT_EQUAL((size_t)1, metricSet.count("node1.node2.node4"));

    tree.getMetrics("node1.node2.node3", metricSet);
    CPPUNIT_ASSERT_EQUAL((size_t)1, metricSet.size());
    CPPUNIT_ASSERT_EQUAL((size_t)1, metricSet.count("node1.node2.node3"));

    tree.getMetrics("pathXXXX", metricSet);
    CPPUNIT_ASSERT_EQUAL((size_t)0, metricSet.size());

    pattern = "*";
    int32_t eraseCount = 0; 
    int32_t keepCount = 0;
    tree.filterTree(isMonitored, eraseCount, keepCount);
    CPPUNIT_ASSERT_EQUAL((size_t)2, tree._metricSpecMap.size());

    pattern = "*n?*e3*";
    tree.filterTree(isMonitored, eraseCount, keepCount);
    CPPUNIT_ASSERT_EQUAL((size_t)1, tree._metricSpecMap.size());

    pattern = "XXXXXXX";
    tree.filterTree(isMonitored, eraseCount, keepCount);
    CPPUNIT_ASSERT_EQUAL((size_t)0, tree._metricSpecMap.size());
}

void MetricTreeTest::testGetHashValue()
{
    MetricNodePtr node1(new MetricNode("node1"));
    MetricNodePtr node2(new MetricNode("node2"));
    MetricNodePtr node3(new MetricNode("node3"));
    MetricNodePtr node4(new MetricNode("node4"));
    MetricNodePtr node5(new MetricNode("node5"));
    MetricNodePtr node6(new MetricNode("node6"));
    MetricPath path;
    path.push_back(node1);
    path.push_back(node2);
    path.push_back(node3);

    MetricTree tree;
    tree.addPath(path, "spec1");
    path.pop_back();
    path.push_back(node4);
    tree.addPath(path, "spec2");
    path.pop_back();
    path.push_back(node5);
    tree.addPath(path, "spec3");

    uint64_t expecHashVal = StringHash::hashString64("node1.node2.node3node1.node2.node4node1.node2.node5");
    CPPUNIT_ASSERT_EQUAL(expecHashVal, tree.getHashValue());

    path.pop_back();
    path.push_back(node6);
    tree.addPath(path, "spec1");
    CPPUNIT_ASSERT(expecHashVal != tree.getHashValue());
}

RA_END_NAMESPACE(tree);

