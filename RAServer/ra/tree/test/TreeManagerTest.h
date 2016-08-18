#ifndef RA_TREEMANAGERTEST_H
#define RA_TREEMANAGERTEST_H

#include <ra/common/Common.h>
#include <ra/common/Log.h>
#include <ra/tree/TreeManager.h>
#include <cppunit/extensions/HelperMacros.h>

RA_BEGIN_NAMESPACE(tree);

class TreeManagerTest : public CppUnit::TestFixture {
    CPPUNIT_TEST_SUITE(TreeManagerTest);
    CPPUNIT_TEST(testSimpleProcess);
    CPPUNIT_TEST(testStartStop);
    CPPUNIT_TEST(testGetMetricTree);
    CPPUNIT_TEST(testMergePath);
    CPPUNIT_TEST(testMergeTree);
    CPPUNIT_TEST(testIsNeedReloadTree);
    CPPUNIT_TEST(testGenMetricSubtree);
    CPPUNIT_TEST(testGenOneRequestPackage);
    CPPUNIT_TEST(testGenRequestPackages);
    CPPUNIT_TEST(testGenProcessPackages);
    CPPUNIT_TEST(testWorkLoop);

    CPPUNIT_TEST_SUITE_END();
public:
    TreeManagerTest();
    ~TreeManagerTest();
public:
    void setUp();
    void tearDown();
    void testSimpleProcess();
    void testStartStop();

    void testGetMetricTree();
    void testMergePath();
    void testMergeTree();

    void testIsNeedReloadTree();
    void testGenMetricSubtree();
    void testGenOneRequestPackage();
    void testGenRequestPackages();
    void testGenProcessPackages();
    void testWorkLoop();

private:
    void generateDefaultTree(MetricTree& tree, fetcher::MetricNodePtr &root) const;

private:
    RA_LOG_DECLARE();
};

RA_END_NAMESPACE(tree);

#endif //RA_TREEMANAGERTEST_H
