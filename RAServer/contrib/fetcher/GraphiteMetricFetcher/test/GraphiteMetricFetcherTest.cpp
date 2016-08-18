#include <ra/fetcher/MetricDataNode.h>
#include <ra/common/HttpClient.h>
#include <ra/common/HttpServer.h>
#include <ra/util/Util.h>
#include <ra/test/test.h>
#include <cppunit/TestAssert.h>
#include "../GraphiteMetricFetcher.h"
#include "GraphiteMetricFetcherTest.h"

RA_BEGIN_NAMESPACE(fetcher);
RA_USE_NAMESPACE(common);
using namespace std;
RA_LOG_SETUP(fetcher, GraphiteMetricFetcherTest);

CPPUNIT_TEST_SUITE_REGISTRATION(GraphiteMetricFetcherTest);
class FakeHandler : public HttpRequestHandler 
{
public:
    FakeHandler(const string &path) {
        _path = path + "?";
    }

    void add(const string &str, const string &body) {
        _content[str] = body;
    }
    
    string getArgStr(const map<string,string> &arguments, const string &argument, bool &isFirst) {
        map<string,string>::const_iterator iter = arguments.find(argument);
        if (iter != arguments.end()) {
            if (isFirst) {
                isFirst = false;
                return argument + "=" + iter->second;
            }
            
            return string("&") + argument + "=" + iter->second;
        }
        return "";
    }

    string concat(map<string, string> arguments) {
        string ret;
        string argNames[] = {
            "target",
            "format",
            "from",
            "until",
            "query"
        };
        bool isFirst = true;
        for(size_t i = 0; i < sizeof(argNames)/sizeof(string); ++i) {
            ret += getArgStr(arguments, argNames[i], isFirst);
        }
        return ret;
    }
    virtual ~FakeHandler() {}
    virtual void process(const HttpRequest *request, HttpResponse *response) {
        if(request && response) {
            string path = concat(request->uri.arguments);
            //cout << "guocanwen-test. xxpath=" << path.c_str() << ": "<< _content[path] << endl;
            response->body = _content[path];
            response->status = HTTP_RESP_OK;
            response->message = "OK";
        }
    }
private:
    string _path;
    map<string,string> _content;
};

class HttpServerSyncWrapper{
public:
    HttpServerSyncWrapper(HttpServer *httpServer) {
        _httpServer = httpServer;
    }

    ~HttpServerSyncWrapper() {
        _httpServer->stop();
    }
    HttpServer *_httpServer;
};

GraphiteMetricFetcherTest::GraphiteMetricFetcherTest() { 
}

GraphiteMetricFetcherTest::~GraphiteMetricFetcherTest() { 
}

void GraphiteMetricFetcherTest::setUp() { 
    RA_LOG(DEBUG, "setUp!");
}

void GraphiteMetricFetcherTest::tearDown() { 
    RA_LOG(DEBUG, "tearDown!");
}

void GraphiteMetricFetcherTest::testSimpleProcess() {
    HttpServer httpServer;
    CPPUNIT_ASSERT(httpServer.init("0.0.0.0", 58020));
    const string content = "[{\"target\": \"test.random\", \"datapoints\": [[null, 1467862680], [null, 1467862740], [136.0, 1467862800], [256.0, 1467862860], [374.0, 1467862920], [494.0, 1467862980], [612.0, 1467863040], [732.0, 1467863100], [852.0, 1467863160], [970.0, 1467863220], [998.0, 1467863280], [null, 1467863340], [null, 1467863400]]}]";
    const string path1 = "/render";
    const string argStr = "target=test.random&format=json&from=1467862620&until=1467863400";
    FakeHandler fakeHandler1(path1);
    fakeHandler1.add(argStr, content);
    httpServer.registerHandler(path1, &fakeHandler1);

    const string path2 = "/metrics/find";
    FakeHandler fakeHandler2(path2);
    const string argStr1 = "query=*";
    const string content1 = "[{\"text\": \"test\", \"expandable\": 1, \"leaf\": 0, \"id\": \"test\", \"allowChildren\": 1}]";
    fakeHandler2.add(argStr1, content1);
    
    const string argStr2 = "query=test.*";
    const string content2 = "[{\"text\": \"random\", \"expandable\": 1, \"leaf\": 1, \"id\": \"carbon.agents\", \"allowChildren\": 1}]";
    fakeHandler2.add(argStr2, content2);
    httpServer.registerHandler(path2, &fakeHandler2);  

    HttpServerSyncWrapper hw(&httpServer);
    CPPUNIT_ASSERT(httpServer.start());

    GraphiteMetricFetcher fetcher;
    OptionMap option;
    option[ACCESS_HOST] = "http://127.0.0.1:58020/";
    option[BATCH_ALLOC_TREE] = "false";
    CPPUNIT_ASSERT(fetcher.init(option));

    MetricNodePtr rootNode = fetcher.allocTree(1,1);
    MetricNodePtr root = fetcher.retrieve(rootNode, 1467862620000000, 1467863400000000, 1);
    CPPUNIT_ASSERT(root != NULL);
    const vector<MetricNodePtr> nodes1 = root->getChildren(); 
    CPPUNIT_ASSERT_EQUAL(size_t(1), nodes1.size());
    CPPUNIT_ASSERT_EQUAL(string("test"), nodes1[0]->getLabel());
    const vector<MetricNodePtr> nodes2 = nodes1[0]->getChildren();
    CPPUNIT_ASSERT_EQUAL(size_t(1), nodes2.size()); 
    CPPUNIT_ASSERT_EQUAL(string("random"), nodes2[0]->getLabel());
    
    map<uint64_t, double> time2Value;
    time2Value[1467862800000000] = 136.0;
    time2Value[1467862860000000] = 256.0;
    time2Value[1467862920000000] = 374.0;
    time2Value[1467862980000000] = 494.0;
    time2Value[1467863040000000] = 612.0;
    time2Value[1467863100000000] = 732.0;
    time2Value[1467863160000000] = 852.0;
    time2Value[1467863220000000] = 970.0;
    time2Value[1467863280000000] = 998.0;

    const vector<MetricNodePtr> &children = nodes2[0]->getChildren();
    CPPUNIT_ASSERT_EQUAL(size_t(1), children.size());
    MetricDataNodePtr datanode = tr1::dynamic_pointer_cast<MetricDataNode>(children[0]);
    const MetricDataPtr xData = datanode->getData();
    for (size_t i = 0; i < xData->times->size(); ++i) {
        uint64_t time = (*(xData->times))[i];
        double value = (*(xData->values))[i];
        CPPUNIT_ASSERT_DOUBLES_EQUAL(time2Value[time], value, 0.01);
    }
}

void GraphiteMetricFetcherTest::testParseSubMetrics() { 
    RA_LOG(DEBUG, "Begin Test!");
    const string content = "[{\"text\": \"cao\", \"expandable\": 1, \"leaf\": 0, \"id\": \"test.random.diceroll.metric.cao\", \"allowChildren\": 1}, {\"text\": \"xxx\", \"expandable\": 1, \"leaf\": 0, \"id\": \"test.random.diceroll.metric.xxx\", \"allowChildren\": 1}]";
    map<string, int> subMetrics;
    
    CPPUNIT_ASSERT(GraphiteMetricFetcher::parseSubMetrics(content, subMetrics));
    CPPUNIT_ASSERT_EQUAL(size_t(2),subMetrics.size());
    map<string, int>::iterator iter = subMetrics.find("cao");
    CPPUNIT_ASSERT(iter != subMetrics.end());
    CPPUNIT_ASSERT_EQUAL(0, iter->second);
    iter = subMetrics.find("xxx");
    CPPUNIT_ASSERT(iter != subMetrics.end());
    CPPUNIT_ASSERT_EQUAL(0, iter->second);

    const string content1 = "[{\"text\": \"cao\", \"expandable\": 1, \"leaf\": 1, \"id\": \"test.random.diceroll.metric.cao\", \"allowChildren\": 1}, {\"text\": \"xxx\", \"expandable\": 1, \"leaf\": 1, \"id\": \"test.random.diceroll.metric.xxx\", \"allowChildren\": 1}]";
    subMetrics.clear();
    
    CPPUNIT_ASSERT(GraphiteMetricFetcher::parseSubMetrics(content1, subMetrics));
    CPPUNIT_ASSERT_EQUAL(size_t(2),subMetrics.size());
    iter = subMetrics.find("cao");
    CPPUNIT_ASSERT(iter != subMetrics.end());
    CPPUNIT_ASSERT_EQUAL(1, iter->second);
    iter = subMetrics.find("xxx");
    CPPUNIT_ASSERT(iter != subMetrics.end());
    CPPUNIT_ASSERT_EQUAL(1, iter->second);

    const string content2 = "[{\"text\": I am not a json string";
    subMetrics.clear();
    CPPUNIT_ASSERT(!GraphiteMetricFetcher::parseSubMetrics(content2, subMetrics));

    const string content3 = "[]";
    subMetrics.clear();
    CPPUNIT_ASSERT(GraphiteMetricFetcher::parseSubMetrics(content3, subMetrics));
    CPPUNIT_ASSERT_EQUAL(size_t(0),subMetrics.size());

    const string content4 = "[{}]";
    subMetrics.clear();
    CPPUNIT_ASSERT(!GraphiteMetricFetcher::parseSubMetrics(content4, subMetrics));

    const string content5 = "[{\"expandable\": 1, \"leaf\": 1, \"id\": \"test.random.diceroll.metric.cao\", \"allowChildren\": 1}, {\"text\": \"xxx\", \"expandable\": 1, \"leaf\": 1, \"id\": \"test.random.diceroll.metric.xxx\", \"allowChildren\": 1}]";
    subMetrics.clear();
    CPPUNIT_ASSERT(!GraphiteMetricFetcher::parseSubMetrics(content4, subMetrics));
}

void GraphiteMetricFetcherTest::testParseSubMetricData() { 
    const string content = "[{\"target\": \"test.random.diceroll.metric.cao.tt\", \"datapoints\": [[null, 1467862680], [null, 1467862740], [136.0, 1467862800], [256.0, 1467862860], [374.0, 1467862920], [970.0, 1467863220], [998.0, 1467863280], [null, 1467863340], [null, 1467863400]]}]";
    const string metricPath = "test.random.diceroll.metric.cao.tt";
    map<uint64_t, double> metricData;
    CPPUNIT_ASSERT(GraphiteMetricFetcher::parseSubMetricData(content, metricData, metricPath)); 
    CPPUNIT_ASSERT_EQUAL((size_t)5, metricData.size());
    CPPUNIT_ASSERT_EQUAL(136.0, metricData[1467862800]); 
    CPPUNIT_ASSERT_EQUAL(256.0, metricData[1467862860]); 
    CPPUNIT_ASSERT_EQUAL(374.0, metricData[1467862920]); 
    CPPUNIT_ASSERT_EQUAL(970.0, metricData[1467863220]); 
    CPPUNIT_ASSERT_EQUAL(998.0, metricData[1467863280]); 

    const string content1 = "invalidate json content";
    metricData.clear();
    CPPUNIT_ASSERT(!GraphiteMetricFetcher::parseSubMetricData(content1, metricData, metricPath)); 
    const string content2 = "{\"x\":10}";
    metricData.clear();
    CPPUNIT_ASSERT(!GraphiteMetricFetcher::parseSubMetricData(content2, metricData, metricPath)); 
    
    const string content3 = "[{\"target\": \"not exist path\", \"datapoints\": [[null, 1467862680], [null, 1467862740], [136.0, 1467862800], [256.0, 1467862860], [374.0, 1467862920], [970.0, 1467863220], [998.0, 1467863280], [null, 1467863340], [null, 1467863400]]}]";
    metricData.clear();
    CPPUNIT_ASSERT(GraphiteMetricFetcher::parseSubMetricData(content3, metricData, metricPath)); 
    CPPUNIT_ASSERT_EQUAL((size_t)0, metricData.size());

    const string content4 = "[{\"target\": \"test.random.diceroll.metric.cao.tt\", \"has none datapoints\": [[null, 1467862680], [null, 1467862740], [136.0, 1467862800], [256.0, 1467862860], [374.0, 1467862920], [970.0, 1467863220], [998.0, 1467863280], [null, 1467863340], [null, 1467863400]]}]";
    metricData.clear();
    CPPUNIT_ASSERT(!GraphiteMetricFetcher::parseSubMetricData(content4, metricData, metricPath)); 
    
    const string content5 = "[{\"target\": \"test.random.diceroll.metric.cao.tt\", \"datapoints\": 10}]";
    metricData.clear();
    CPPUNIT_ASSERT(!GraphiteMetricFetcher::parseSubMetricData(content5, metricData, metricPath)); 

    const string content6 = "[]";
    metricData.clear();
    CPPUNIT_ASSERT(GraphiteMetricFetcher::parseSubMetricData(content6, metricData, metricPath)); 
    
    const string content7 = "[{\"target\": \"test.random.diceroll.metric.cao.tt\", \"datapoints\": []}]";
    metricData.clear();
    CPPUNIT_ASSERT(GraphiteMetricFetcher::parseSubMetricData(content7, metricData, metricPath));

    const string content8 = "[{\"target\": \"test.random.diceroll.metric.cao.tt\", \"datapoints\": [[1,2,3]]}]";
    metricData.clear();
    CPPUNIT_ASSERT(!GraphiteMetricFetcher::parseSubMetricData(content8, metricData, metricPath));
}

void GraphiteMetricFetcherTest::testGetDataURL() { 
    GraphiteMetricFetcher fetcher;
    OptionMap option;
    option[ACCESS_HOST] = "http://100.82.23.31:8080/";
    CPPUNIT_ASSERT(fetcher.init(option));
    const string metricPath = "test.random.diceroll.metric.cao.tt";
    string url;
    fetcher.getDataURL(metricPath, 0, 0, url);
    CPPUNIT_ASSERT_EQUAL(string("http://100.82.23.31:8080/render?target=test.random.diceroll.metric.cao.tt&format=json"), url);
    fetcher.getDataURL(metricPath, 100, 0, url);
    CPPUNIT_ASSERT_EQUAL(string("http://100.82.23.31:8080/render?target=test.random.diceroll.metric.cao.tt&format=json&from=100"), url);
    fetcher.getDataURL(metricPath, 0, 200, url);
    CPPUNIT_ASSERT_EQUAL(string("http://100.82.23.31:8080/render?target=test.random.diceroll.metric.cao.tt&format=json&until=200"), url);
    fetcher.getDataURL(metricPath, 100, 200, url);
    CPPUNIT_ASSERT_EQUAL(string("http://100.82.23.31:8080/render?target=test.random.diceroll.metric.cao.tt&format=json&from=100&until=200"), url);
}

void GraphiteMetricFetcherTest::testGetMetricsURL() { 
    GraphiteMetricFetcher fetcher;
    OptionMap option;
    option[ACCESS_HOST] = "http://100.82.23.31:8080/";
    CPPUNIT_ASSERT(fetcher.init(option));
    const string metricPath = "test.*";
    string url;
    fetcher.getMetricsURL(metricPath, url);
    CPPUNIT_ASSERT_EQUAL(string("http://100.82.23.31:8080/metrics/find?query=test.*"), url);
}

void GraphiteMetricFetcherTest::testGetAllMetricsURL(){
    GraphiteMetricFetcher fetcher;
    OptionMap option;
    option[ACCESS_HOST] = "http://100.82.23.31:8080/";
    CPPUNIT_ASSERT(fetcher.init(option));
    string url;
    fetcher.getAllMetricsURL(url);
    CPPUNIT_ASSERT_EQUAL(string("http://100.82.23.31:8080/metrics/index.json"), url);
}

void GraphiteMetricFetcherTest::testInit() { 
    GraphiteMetricFetcher fetcher;
    OptionMap option;
    CPPUNIT_ASSERT(!fetcher.init(option));
    option[ACCESS_HOST] = "http://100.82.23.31:8080/";
    CPPUNIT_ASSERT(fetcher.init(option));
}

void GraphiteMetricFetcherTest::testTraverse() { 
    HttpServer httpServer;
    CPPUNIT_ASSERT(httpServer.init("0.0.0.0", 58020));
    HttpServerSyncWrapper hw(&httpServer);
    
    const string path = "/metrics/find";
    FakeHandler fakeHandler(path);

    const string argStr1 = "query=*";
    const string content1 = "[{\"text\": \"carbon\", \"expandable\": 1, \"leaf\": 0, \"id\": \"carbon\", \"allowChildren\": 1}, {\"text\": \"test\", \"expandable\": 1, \"leaf\": 0, \"id\": \"test\", \"allowChildren\": 1}]";
    fakeHandler.add(argStr1, content1);
    
    const string argStr2 = "query=carbon.*";
    const string content2 = "[{\"text\": \"agents\", \"expandable\": 1, \"leaf\": 0, \"id\": \"carbon.agents\", \"allowChildren\": 1}]";
    fakeHandler.add(argStr2, content2);

    const string argStr3 = "query=carbon.agents.*";
    const string content3 = "[{\"text\": \"e18e04254_et15sqa-a\", \"expandable\": 1, \"leaf\": 1, \"id\": \"carbon.agents.e18e04254_et15sqa-a\", \"allowChildren\": 1}]";
    fakeHandler.add(argStr3, content3);

    const string argStr4 = "query=test.*";
    const string content4 = "[{\"text\": \"random\", \"expandable\": 1, \"leaf\": 1, \"id\": \"test.random\", \"allowChildren\": 1}]";
    fakeHandler.add(argStr4, content4);
    
    httpServer.registerHandler(path, &fakeHandler);
    CPPUNIT_ASSERT(httpServer.start());
    MetricNodePtr root(new MetricNode(METRIC_ROOT_NODE_LABEL));
    string xpath; 

    GraphiteMetricFetcher fetcher;
    OptionMap option;
    option[ACCESS_HOST] = "http://127.0.0.1:58020/";
    CPPUNIT_ASSERT(fetcher.init(option));
    CPPUNIT_ASSERT(fetcher.traverse(xpath, root));
    RA_LOG(ERROR, "guocanwen-test. roottree=%s", root->toDebugString().c_str());
    const vector<MetricNodePtr> rootNodes = root->getChildren();
    CPPUNIT_ASSERT_EQUAL(size_t(2), rootNodes.size());
    for(size_t i = 0; i < rootNodes.size(); i++) { 
        if (rootNodes[i]->getLabel() == "carbon") {
            const vector<MetricNodePtr> nodes = rootNodes[i]->getChildren(); 
            CPPUNIT_ASSERT_EQUAL(size_t(1), nodes.size());
            CPPUNIT_ASSERT_EQUAL(string("agents"), nodes[0]->getLabel());
            const vector<MetricNodePtr> nodes_c = nodes[i]->getChildren();
            CPPUNIT_ASSERT_EQUAL(size_t(1), nodes_c.size()); 
            CPPUNIT_ASSERT_EQUAL(string("e18e04254_et15sqa-a"), nodes_c[0]->getLabel());
        } else if (rootNodes[i]->getLabel() == "test") {
            const vector<MetricNodePtr> nodes = rootNodes[i]->getChildren(); 
            CPPUNIT_ASSERT_EQUAL(size_t(1), nodes.size());
            CPPUNIT_ASSERT_EQUAL(string("random"), nodes[0]->getLabel());
        } else {
            CPPUNIT_ASSERT(false);
        }
    }
}

void GraphiteMetricFetcherTest::testParseAllMetrics() {
    set<string> metrics;
    const string content = "[\"big\", \"big.big.big\", \"big.big.big.care\", \"care\", \"care.big\", \"care.big.big.forever\", \"zheng.big.care\", \"zheng.big.care.care.forever\", \"zheng.big.care.zheng\", \"zheng.big.ff.care\", \"zheng.big.ff.ff\", \"zheng.big.zheng\", \"zheng.big.zheng.big\", \"zheng.care.big.forever.ff\", \"zheng.care.ff\", \"zheng.care.forever.ff\", \"zheng.care.zheng.zheng.zheng\", \"zheng.ff\",\"zheng.ff.forever\", \"zheng.ff.forever.zheng\", \"zheng.ff.zheng\", \"zheng.ff.zheng.forever\", \"zheng.forever\", \"zheng.forever.ff\", \"zheng.forever.ff.ff\"]";
    CPPUNIT_ASSERT(GraphiteMetricFetcher::parseAllMetrics(content, metrics));
    set<string> expectedSet;
    expectedSet.insert("big");
    expectedSet.insert("big.big.big");
    expectedSet.insert("big.big.big.care");
    expectedSet.insert("care");
    expectedSet.insert("care.big");
    expectedSet.insert("care.big.big.forever");
    expectedSet.insert("zheng.big.care");
    expectedSet.insert("zheng.big.care.care.forever");
    expectedSet.insert("zheng.big.care.zheng");
    expectedSet.insert("zheng.big.ff.care");
    expectedSet.insert("zheng.big.ff.ff");
    expectedSet.insert("zheng.big.zheng");
    expectedSet.insert("zheng.big.zheng.big");
    expectedSet.insert("zheng.care.big.forever.ff");
    expectedSet.insert("zheng.care.ff");
    expectedSet.insert("zheng.care.forever.ff");
    expectedSet.insert("zheng.care.zheng.zheng.zheng");
    expectedSet.insert("zheng.ff");
    expectedSet.insert("zheng.ff.forever");
    expectedSet.insert("zheng.ff.forever.zheng");
    expectedSet.insert("zheng.ff.zheng");
    expectedSet.insert("zheng.ff.zheng.forever");
    expectedSet.insert("zheng.forever");
    expectedSet.insert("zheng.forever.ff");
    expectedSet.insert("zheng.forever.ff.ff");
    CPPUNIT_ASSERT_EQUAL(expectedSet.size(), metrics.size());
    for(set<string>::const_iterator iter = metrics.begin();
        iter != metrics.end();
        ++iter) {
        set<string>::const_iterator xIter = expectedSet.find(*iter);
        CPPUNIT_ASSERT(xIter != expectedSet.end());
    }
}

void GraphiteMetricFetcherTest::testMakeTree() {
    set<string> metrics;
    metrics.insert("a");
    metrics.insert("a.b");
    metrics.insert("a.b.c");
    metrics.insert("x");
    metrics.insert("x.y");
    MetricNodePtr root(new MetricNode(METRIC_ROOT_NODE_LABEL));
    CPPUNIT_ASSERT(GraphiteMetricFetcher::makeTree(metrics, root));
    const vector<MetricNodePtr> level1 = root->getChildren();
    CPPUNIT_ASSERT_EQUAL(size_t(2), level1.size());
    for (size_t i = 0;  i < level1.size(); ++i) {
        if (level1[i]->getLabel() == "a") {
            const vector<MetricNodePtr> level2 = level1[i]->getChildren();
            CPPUNIT_ASSERT_EQUAL(size_t(1), level2.size());
            CPPUNIT_ASSERT_EQUAL(string("b"), level2[0]->getLabel());
            const vector<MetricNodePtr> level3 = level2[i]->getChildren();
            CPPUNIT_ASSERT_EQUAL(size_t(1), level3.size());
            CPPUNIT_ASSERT_EQUAL(string("c"), level3[0]->getLabel());
        } else if (level1[i]->getLabel() == "x") {
            const vector<MetricNodePtr> level2 = level1[i]->getChildren();
            CPPUNIT_ASSERT_EQUAL(size_t(1), level2.size());
            CPPUNIT_ASSERT_EQUAL(string("y"), level2[0]->getLabel());
        } else {
            CPPUNIT_ASSERT(false);
        }
    }
}


void GraphiteMetricFetcherTest::testRetrieve() {
    HttpServer httpServer;
    CPPUNIT_ASSERT(httpServer.init("0.0.0.0", 58020));
    const string content = "[{\"target\": \"test.random\", \"datapoints\": [[null, 1467862680], [null, 1467862740], [136.0, 1467862800], [256.0, 1467862860], [374.0, 1467862920], [494.0, 1467862980], [612.0, 1467863040], [732.0, 1467863100], [852.0, 1467863160], [970.0, 1467863220], [998.0, 1467863280], [null, 1467863340], [null, 1467863400]]}]";
    const string path = "/render";
    const string argStr = "target=test.random&format=json&from=1467862620&until=1467863400";
    FakeHandler fakeHandler(path);
    fakeHandler.add(argStr, content);
    httpServer.registerHandler(path, &fakeHandler);
    HttpServerSyncWrapper hw(&httpServer);
    CPPUNIT_ASSERT(httpServer.start());

    MetricNodePtr rootNode(new MetricNode(METRIC_ROOT_NODE_LABEL));
    MetricNodePtr testNode(new MetricNode("test"));
    rootNode->addChild(testNode);
    MetricNodePtr randomNode(new MetricNode("random"));
    testNode->addChild(randomNode);
    GraphiteMetricFetcher fetcher;
    OptionMap option;
    option[ACCESS_HOST] = "http://127.0.0.1:58020/";
    CPPUNIT_ASSERT(fetcher.init(option));

    MetricNodePtr root = fetcher.retrieve(rootNode, 1467862620000000, 1467863400000000, 1);
    CPPUNIT_ASSERT(root != NULL);
    const vector<MetricNodePtr> nodes1 = root->getChildren(); 
    CPPUNIT_ASSERT_EQUAL(size_t(1), nodes1.size());
    CPPUNIT_ASSERT_EQUAL(string("test"), nodes1[0]->getLabel());
    const vector<MetricNodePtr> nodes2 = nodes1[0]->getChildren();
    CPPUNIT_ASSERT_EQUAL(size_t(1), nodes2.size()); 
    CPPUNIT_ASSERT_EQUAL(string("random"), nodes2[0]->getLabel());
    
    map<uint64_t, double> time2Value;
    time2Value[1467862800000000] = 136.0;
    time2Value[1467862860000000] = 256.0;
    time2Value[1467862920000000] = 374.0;
    time2Value[1467862980000000] = 494.0;
    time2Value[1467863040000000] = 612.0;
    time2Value[1467863100000000] = 732.0;
    time2Value[1467863160000000] = 852.0;
    time2Value[1467863220000000] = 970.0;
    time2Value[1467863280000000] = 998.0;

    const vector<MetricNodePtr> &children = nodes2[0]->getChildren();
    CPPUNIT_ASSERT_EQUAL(size_t(1), children.size());
    MetricDataNodePtr datanode = tr1::dynamic_pointer_cast<MetricDataNode>(children[0]);
    const MetricDataPtr xData = datanode->getData();
    for (size_t i = 0; i < xData->times->size(); ++i) {
        uint64_t time = (*(xData->times))[i];
        double value = (*(xData->values))[i];
        CPPUNIT_ASSERT_DOUBLES_EQUAL(time2Value[time], value, 0.01);
    }
}

void GraphiteMetricFetcherTest::testReadGraphiteData() {
    HttpServer httpServer;
    CPPUNIT_ASSERT(httpServer.init("0.0.0.0", 58020));
    const string content = "[{\"target\": \"test.random.diceroll.metric.cao.tt\", \"datapoints\": [[null, 1467862680], [null, 1467862740], [136.0, 1467862800], [256.0, 1467862860], [374.0, 1467862920], [494.0, 1467862980], [612.0, 1467863040], [732.0, 1467863100], [852.0, 1467863160], [970.0, 1467863220], [998.0, 1467863280], [null, 1467863340], [null, 1467863400]]}]";
    const string path = "/render";
    const string argStr = "target=test.random.diceroll.metric.cao.tt&format=json&from=1467862620&until=1467863400";
    FakeHandler fakeHandler(path);
    fakeHandler.add(argStr, content);
    httpServer.registerHandler(path, &fakeHandler);
    HttpServerSyncWrapper hw(&httpServer);
    CPPUNIT_ASSERT(httpServer.start());
    MetricPath metricPath;
    metricPath.push_back(MetricNodePtr(new MetricNode(METRIC_ROOT_NODE_LABEL)));
    metricPath.push_back(MetricNodePtr(new MetricNode("test")));
    metricPath.push_back(MetricNodePtr(new MetricNode("random")));
    metricPath.push_back(MetricNodePtr(new MetricNode("diceroll")));
    metricPath.push_back(MetricNodePtr(new MetricNode("metric")));
    metricPath.push_back(MetricNodePtr(new MetricNode("cao")));
    MetricNodePtr metricNode(new MetricNode("tt"));
    metricPath.push_back(MetricNodePtr(metricNode));
    
    GraphiteMetricFetcher fetcher;
    OptionMap option;
    option[ACCESS_HOST] = "http://127.0.0.1:58020/";
    CPPUNIT_ASSERT(fetcher.init(option));
    CPPUNIT_ASSERT(fetcher.readGraphiteData(metricPath, 1467862620000000, 1467863400000000, 1));
    map<uint64_t, double> time2Value;
    time2Value[1467862800000000] = 136.0;
    time2Value[1467862860000000] = 256.0;
    time2Value[1467862920000000] = 374.0;
    time2Value[1467862980000000] = 494.0;
    time2Value[1467863040000000] = 612.0;
    time2Value[1467863100000000] = 732.0;
    time2Value[1467863160000000] = 852.0;
    time2Value[1467863220000000] = 970.0;
    time2Value[1467863280000000] = 998.0;

    const vector<MetricNodePtr> &children = metricNode->getChildren();
    CPPUNIT_ASSERT_EQUAL(size_t(1), children.size());
    MetricDataNodePtr datanode = tr1::dynamic_pointer_cast<MetricDataNode>(children[0]);
    const MetricDataPtr xData = datanode->getData();
    for (size_t i = 0; i < xData->times->size(); ++i) {
        uint64_t time = (*(xData->times))[i];
        double value = (*(xData->values))[i];
        CPPUNIT_ASSERT_DOUBLES_EQUAL(time2Value[time], value, 0.01);
    }
}

void GraphiteMetricFetcherTest::testMain() {
    GraphiteMetricFetcher fetcher;
    OptionMap option;
    option[ACCESS_HOST] = "http://100.82.23.31:8080/";
    CPPUNIT_ASSERT(fetcher.init(option));
    MetricNodePtr rootNode = fetcher.allocTree(1,1);
    CPPUNIT_ASSERT(rootNode != NULL);
    int64_t now = util::Util::currentTimeInSeconds();
    MetricNodePtr root = fetcher.retrieve(rootNode, 1000000*(now - 600), 1000000*now, 1);
    //MetricNodePtr root = fetcher.retrieve(rootNode, 0, 0, 1);
    CPPUNIT_ASSERT(root != NULL);
    RA_LOG(ERROR, "guocanwen-test #######rootNode=%s", rootNode->toDebugString().c_str());
    RA_LOG(ERROR, "guocanwen-test #######root=%s", root->toDebugString().c_str());
}
RA_END_NAMESPACE(fetcher);
