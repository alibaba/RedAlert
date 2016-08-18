#include <tr1/memory>
#include <ra/test/test.h>
#include <ra/common/Json.h>
#include <ra/common/HttpServer.h>
#include <ra/tree/TreeManager.h>
#include <ra/service/ListMetricHandler.h>
#include <ra/service/test/ListMetricHandlerTest.h>

RA_BEGIN_NAMESPACE(service);
RA_USE_NAMESPACE(common);
RA_USE_NAMESPACE(fetcher);
RA_USE_NAMESPACE(tree);
using namespace std;
using namespace std::tr1;

RA_LOG_SETUP(service, ListMetricHandlerTest);
CPPUNIT_TEST_SUITE_REGISTRATION(ListMetricHandlerTest);

ListMetricHandlerTest::ListMetricHandlerTest() {
}

ListMetricHandlerTest::~ListMetricHandlerTest() {
}

void ListMetricHandlerTest::setUp() {
    RA_LOG(INFO, "setUp!");
}

void ListMetricHandlerTest::tearDown() {
    RA_LOG(INFO, "tearDown!");
}

JsonArrayPtr ListMetricHandlerTest::doTestList(HttpClient& client, const string& url) {
    HttpResponse response;
    CPPUNIT_ASSERT(client.get(url, &response));
    CPPUNIT_ASSERT_EQUAL(HTTP_RESP_OK, response.status);
    RA_LOG(INFO, "GET '%s': %s", url.c_str(), response.body.c_str());
    JsonPtr json = Json::load(response.body);
    JsonArrayPtr array = dynamic_pointer_cast<JsonArray>(json);
    return array;
}

void ListMetricHandlerTest::testListMetric() {
    MetricNodePtr root(new MetricNode(METRIC_ROOT_NODE_LABEL));
    MetricNodePtr node1(new MetricNode("node1"));
    MetricNodePtr node2(new MetricNode("node2"));
    root->addChild(node1);
    root->addChild(node2);
    MetricNodePtr node3(new MetricNode("node3"));
    MetricNodePtr node4(new MetricNode("node4"));
    MetricNodePtr node5(new MetricNode("node5"));
    node1->addChild(node3);
    node1->addChild(node4);
    node3->addChild(node5);

    TreeManager treeManager;
    treeManager.setMasterMetricTree(root);
    ListMetricHandler handler(&treeManager);
    HttpServer server;
    CPPUNIT_ASSERT(server.init("127.0.0.1", 18082));
    CPPUNIT_ASSERT(server.registerHandler(HTTP_API_PATH_LIST_METRIC, &handler));
    CPPUNIT_ASSERT(server.start());

    HttpClient client;
    CPPUNIT_ASSERT(client.init());
    string url;
    JsonArrayPtr array;
    url = "http://127.0.0.1:18082" + HTTP_API_PATH_LIST_METRIC;
    array = doTestList(client, url);
    CPPUNIT_ASSERT(array);
    CPPUNIT_ASSERT_EQUAL((size_t)2U, array->size());

    url = "http://127.0.0.1:18082" + HTTP_API_PATH_LIST_METRIC + "?path=node1";
    array = doTestList(client, url);
    CPPUNIT_ASSERT(array);
    CPPUNIT_ASSERT_EQUAL((size_t)2U, array->size());

    url = "http://127.0.0.1:18082" + HTTP_API_PATH_LIST_METRIC + "?path=node1:node3";
    array = doTestList(client, url);
    CPPUNIT_ASSERT(array);
    CPPUNIT_ASSERT_EQUAL((size_t)1U, array->size());

    url = "http://127.0.0.1:18082" + HTTP_API_PATH_LIST_METRIC + "?path=node1:node3:node5";
    array = doTestList(client, url);
    CPPUNIT_ASSERT(array);
    CPPUNIT_ASSERT_EQUAL((size_t)0U, array->size());

    CPPUNIT_ASSERT(server.stop());
}

RA_END_NAMESPACE(service);
