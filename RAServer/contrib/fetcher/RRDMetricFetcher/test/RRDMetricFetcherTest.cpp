#include <tr1/memory>
#include <cppunit/TestAssert.h>
#include <contrib/filesystem/FileSystemFactory.h>
#include <ra/test/test.h>
#include <ra/common/CommonDefine.h>
#include <ra/util/FileUtil.h>
#include <ra/fetcher/MetricDataNode.h>
#include "../RRDMetricFetcher.h"
#include "RRDMetricFetcherTest.h"

using namespace std;
RA_BEGIN_NAMESPACE(fetcher);
RA_USE_NAMESPACE(fs);
RA_USE_NAMESPACE(util);
RA_LOG_SETUP(fetcher, RRDMetricFetcherTest);

CPPUNIT_TEST_SUITE_REGISTRATION(RRDMetricFetcherTest);

RRDMetricFetcherTest::RRDMetricFetcherTest() { 
}

RRDMetricFetcherTest::~RRDMetricFetcherTest() { 
}

void RRDMetricFetcherTest::setUp() { 
    RA_LOG(DEBUG, "setUp!");
    FileSystemPtr fs = FileSystemFactory::create("LocalFileSystem");
    CPPUNIT_ASSERT(FileUtil::init(fs));
}

void RRDMetricFetcherTest::tearDown() { 
    RA_LOG(DEBUG, "tearDown!");
}

void RRDMetricFetcherTest::testGetMetricName() {
    string name = "xxxxx";
    string metricFileName = name + RRD_FILE_SUFFIX;
    string metricName;
    CPPUNIT_ASSERT(RRDMetricFetcher::getMetricName(metricFileName, metricName));
    CPPUNIT_ASSERT_EQUAL(name, metricName);

    name = "ddd" + RRD_FILE_SUFFIX;
    metricFileName = name + RRD_FILE_SUFFIX;
    CPPUNIT_ASSERT(RRDMetricFetcher::getMetricName(metricFileName, metricName));
    CPPUNIT_ASSERT_EQUAL(name, metricName);

    metricFileName = "NoneRRD";
    CPPUNIT_ASSERT(!RRDMetricFetcher::getMetricName(metricFileName, metricName));
}

void RRDMetricFetcherTest::testCompareAndAddChild() {
    MetricNodePtr parent(new MetricNode("OK"));
    string cNode1 = "cNode1";
    MetricNodePtr saveParent = parent;
    RRDMetricFetcher::compareAndAddChild(parent, cNode1);
    CPPUNIT_ASSERT(NULL != parent);
    CPPUNIT_ASSERT(saveParent != parent);
    const vector<MetricNodePtr> childrenNodes1 = saveParent->getChildren();
    CPPUNIT_ASSERT_EQUAL(size_t(1), childrenNodes1.size());
    CPPUNIT_ASSERT_EQUAL(parent, childrenNodes1[0]);
    
    parent = saveParent;
    RRDMetricFetcher::compareAndAddChild(parent, cNode1);
    CPPUNIT_ASSERT(NULL != parent);
    CPPUNIT_ASSERT(saveParent != parent);
    const vector<MetricNodePtr> childrenNodes2 = saveParent->getChildren();
    CPPUNIT_ASSERT_EQUAL(size_t(1), childrenNodes2.size());
    CPPUNIT_ASSERT_EQUAL(parent, childrenNodes2[0]);

    parent = saveParent;
    string cNode2 = "cNode2";
    RRDMetricFetcher::compareAndAddChild(parent, cNode2);
    CPPUNIT_ASSERT(NULL != parent);
    CPPUNIT_ASSERT(saveParent != parent);
    const vector<MetricNodePtr> childrenNodes3 = saveParent->getChildren();
    CPPUNIT_ASSERT_EQUAL(size_t(2), childrenNodes3.size());
    CPPUNIT_ASSERT_EQUAL(parent, childrenNodes3[1]);
}

void RRDMetricFetcherTest::testTraverse() {
    RRDMetricFetcher fetcher;
    fetcher._rrdRootDir = TEST_DATA_PATH "/rrd_data";
    MetricNodePtr root(new MetricNode(METRIC_ROOT_NODE_LABEL));
    string path = "";
    CPPUNIT_ASSERT(fetcher.traverse(path, root));
    const vector<MetricNodePtr> rootNodes = root->getChildren();
    CPPUNIT_ASSERT_EQUAL(size_t(2), rootNodes.size());
    for(size_t i = 0; i < rootNodes.size(); i++) {
        if (rootNodes[i]->getLabel() == "cluster1") {
            vector<string> labels;
            labels.push_back("cpu_user");
            labels.push_back("pkts_out");
            labels.push_back("proc_run");
            labels.push_back("pkts_in");
            checkChildrenLabel(rootNodes[i], labels);
        } else if (rootNodes[i]->getLabel() == "grid1") {
            vector<string> labels;
            labels.push_back("cluster1");
            labels.push_back("cluster2");
            checkChildrenLabel(rootNodes[i], labels);
            const vector<MetricNodePtr> clusterNodes = rootNodes[i]->getChildren(); 
            for (size_t j = 0; j < clusterNodes.size(); j++) {
                if (clusterNodes[j]->getLabel() == "cluster1") {
                    vector<string> labels;
                    labels.push_back("cpu_idle"); 
                    labels.push_back("pkts_in"); 
                    labels.push_back("proc_run"); 
                    labels.push_back("mem_cached"); 
                    checkChildrenLabel(clusterNodes[j], labels); 
                } else if (clusterNodes[j]->getLabel() == "cluster2") {
                    vector<string> labels; 
                    labels.push_back("cpu_idle"); 
                    checkChildrenLabel(clusterNodes[j], labels);
                } else {
                    CPPUNIT_ASSERT(false); 
                }
            }
        } else {
            CPPUNIT_ASSERT(false);
        }
    }
}

void RRDMetricFetcherTest::testRetrieve() {
    RRDMetricFetcher fetcher;
    fetcher._rrdRootDir = TEST_DATA_PATH "/rrd_data";
    MetricNodePtr root(new MetricNode(METRIC_ROOT_NODE_LABEL));
    string path = "";
    CPPUNIT_ASSERT(fetcher.traverse(path, root));
    MetricNodePtr xx = fetcher.retrieve(root, 1465897575,1465897575 + 360,1);
}

void RRDMetricFetcherTest::testReadRRDData() {
    RRDMetricFetcher fetcher;
    fetcher._rrdRootDir = TEST_DATA_PATH "/rrd_data";

    MetricPath path;
    path.push_back(MetricNodePtr(new MetricNode(METRIC_ROOT_NODE_LABEL)));
    path.push_back(MetricNodePtr(new MetricNode("grid1")));
    path.push_back(MetricNodePtr(new MetricNode("cluster1")));
    MetricNodePtr metricNode(new MetricNode("proc_run"));
    path.push_back(MetricNodePtr(metricNode));
    
    map<string, vector<string> > hostNameBuf;
    CPPUNIT_ASSERT(fetcher.readRRDData(path, 0, 0, 0, hostNameBuf));
    CPPUNIT_ASSERT_EQUAL((size_t)1, hostNameBuf.size());
    map<string, vector<string> >::iterator it = hostNameBuf.begin();
    CPPUNIT_ASSERT_EQUAL(string("/grid1/cluster1"), it->first);
    vector<string> &hosts = it->second;
    CPPUNIT_ASSERT_EQUAL((size_t)3, hosts.size());
    for(size_t i = 0; i < hosts.size(); i++) {
        CPPUNIT_ASSERT(hosts[i] == string("host1") 
                       || hosts[i] == string("host2") 
                       || hosts[i] == string("host3"));
    }
    
    const std::vector<MetricNodePtr>& hostsNode = metricNode->getChildren();

    CPPUNIT_ASSERT_EQUAL((size_t)3, hostsNode.size());
    for(size_t i = 0; i < hostsNode.size(); i++) {
        MetricDataNodePtr datanode = tr1::dynamic_pointer_cast<MetricDataNode>(hostsNode[i]);
        CPPUNIT_ASSERT(datanode->getData()->hostname == string("host1") 
                       || datanode->getData()->hostname == string("host2") 
                       || datanode->getData()->hostname == string("host3"));
    }

    MetricPath path1;
    path1.push_back(MetricNodePtr(new MetricNode(METRIC_ROOT_NODE_LABEL)));
    path1.push_back(MetricNodePtr(new MetricNode("grid1")));
    path1.push_back(MetricNodePtr(new MetricNode("cluster1")));
    MetricNodePtr metricNode1(new MetricNode("mem_cached"));
    path1.push_back(MetricNodePtr(metricNode1));

    CPPUNIT_ASSERT(fetcher.readRRDData(path1, 0, 0, 0, hostNameBuf));
    CPPUNIT_ASSERT_EQUAL((size_t)1, hostNameBuf.size());
    it = hostNameBuf.begin();
    CPPUNIT_ASSERT_EQUAL(string("/grid1/cluster1"), it->first);
    vector<string> &hosts1 = it->second;
    CPPUNIT_ASSERT_EQUAL((size_t)3, hosts1.size());
    for(size_t i = 0; i < hosts1.size(); i++) {
        CPPUNIT_ASSERT(hosts1[i] == string("host1") 
                       || hosts1[i] == string("host2") 
                       || hosts1[i] == string("host3"));
    }
    
    const std::vector<MetricNodePtr>& hostsNode1 = metricNode1->getChildren();
    CPPUNIT_ASSERT_EQUAL((size_t)1, hostsNode1.size());
    MetricDataNodePtr datanode = tr1::dynamic_pointer_cast<MetricDataNode>(hostsNode1[0]);
    CPPUNIT_ASSERT(datanode->getData()->hostname == string("host3"));
}

void RRDMetricFetcherTest::testGetMetricValue() {
    RRDMetricFetcher fetcher;
    fetcher._rrdRootDir = TEST_DATA_PATH "/rrd_data";
    MetricDataPtr data(new MetricData());
    string path = "notExists";
    CPPUNIT_ASSERT(!fetcher.getMetricValue(path, 1465897575, 1465897575 + 360, 1, data));
    path = fetcher._rrdRootDir + "/grid1/cluster1/host1/pkts_in.rrd";
    CPPUNIT_ASSERT(fetcher.getMetricValue(path, 1465897575, 1465897575 + 360, 1, data));
    map<uint64_t, double> time2Value;
    time2Value[1465897590] = 10616.290000;
    time2Value[1465897605] = 10616.290000;
    time2Value[1465897620] = 10333.236667;
    time2Value[1465897635] = 10289.690000;
    time2Value[1465897650] = 9943.731333;
    time2Value[1465897665] = 9919.020000;
    time2Value[1465897680] = 9919.020000;
    time2Value[1465897695] = 10414.848667;
    time2Value[1465897710] = 10491.130000;
    time2Value[1465897725] = 10491.130000;
    time2Value[1465897740] = 10204.027333;
    time2Value[1465897755] = 10183.520000;
    time2Value[1465897770] = 9954.832667;
    time2Value[1465897785] = 9919.650000;
    time2Value[1465897800] = 9919.650000;
    time2Value[1465897815] = 13106.526000;
    time2Value[1465897830] = 13334.160000;
    time2Value[1465897845] = 13334.160000;
    time2Value[1465897860] = 15531.058667;
    time2Value[1465897875] = 15687.980000;
    time2Value[1465897890] = 13508.564667;
    time2Value[1465897905] = 13173.270000;
    time2Value[1465897920] = 13173.270000;
    time2Value[1465897935] = 10197.864667;
    time2Value[1465897950] = 9740.110000;
    for (size_t i = 0; i < data->times->size(); ++i) {
        uint64_t time = (*(data->times))[i];
        double value = (*(data->values))[i];
        CPPUNIT_ASSERT_DOUBLES_EQUAL(time2Value[time], value, 0.01);
    }
}

void RRDMetricFetcherTest::checkChildrenLabel(const MetricNodePtr &parent, const vector<string> & labels) {
    const vector<MetricNodePtr> nodes = parent->getChildren();
    CPPUNIT_ASSERT_EQUAL(labels.size(), nodes.size());
    for (size_t i = 0; i < labels.size(); i++) {
        size_t j = 0;
        for(; j < nodes.size(); j++) {
            if (labels[i] == nodes[j]->getLabel()) {
                break;
            }
        }
        CPPUNIT_ASSERT(j != nodes.size());
    }

    for (size_t j = 0; j < nodes.size(); j++) {
        size_t i = 0;
        for (; i < labels.size(); i++) {
            if (labels[i] == nodes[j]->getLabel()) {
                break;
            }
        }
        CPPUNIT_ASSERT(i != labels.size());
    }
}

RA_END_NAMESPACE(fetcher);

