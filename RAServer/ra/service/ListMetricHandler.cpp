#include <tr1/memory>
#include <ra/common/Json.h>
#include <ra/common/HttpClient.h>
#include <ra/util/Util.h>
#include <ra/tree/TreeManager.h>
#include <ra/fetcher/MetricTreeUtil.h>
#include <ra/service/ListMetricHandler.h>

RA_BEGIN_NAMESPACE(service);
RA_USE_NAMESPACE(util);
RA_USE_NAMESPACE(common);
RA_USE_NAMESPACE(fetcher);
using namespace std;
using namespace std::tr1;

RA_LOG_SETUP(service, ListMetricHandler);

ListMetricHandler::ListMetricHandler(const tree::TreeManager *treeManager): _treeManager(treeManager) {
}

ListMetricHandler::~ListMetricHandler() {
}

void ListMetricHandler::process(const common::HttpRequest* request, common::HttpResponse* response) {
    if (request->method != HTTP_REQ_GET) {
        LOG(WARNING) << "Bad request method, expect GET";
        sendResponse(response, HTTP_RESP_BADMETHOD, "Bad request method, expect GET");
        return;
    }
    string path;
    const map<string, string>& arguments = request->uri.arguments;
    map<string, string>::const_iterator iter = arguments.find(HTTP_API_LIST_METRIC_PATH_KEY);
    if (iter != arguments.end()) path = iter->second;
    vector<string> labels = Util::splitString(path, HTTP_API_LIST_METRIC_PATH_SEP);
    labels.insert(labels.begin(), METRIC_ROOT_NODE_LABEL);
    fetcher::MetricNodePtr root = _treeManager->getMasterMetricTree();
    MetricNodePtr node = MetricTreeUtil::findNode(root, labels);
    if (node == NULL) {
        LOG(WARNING) << "Cannot find metric node for path " << path.c_str();
        sendResponse(response, HTTP_RESP_NOTFOUND, "Cannot find metric node");
        return;
    }
    const vector<MetricNodePtr>& children = node->getChildren();
    JsonArrayPtr array(new JsonArray());
    if (array == NULL) {
        LOG(WARNING) << "Cannot create json for response";
        sendResponse(response, HTTP_RESP_INTERNAL, "Cannot create json for response");
        return;
    }
    for (size_t i = 0; i < children.size(); i++) {
        const MetricNodePtr& child = children[i];
        JsonObjectPtr object(new JsonObject());
        if (object == NULL) {
            LOG(WARNING) << "Cannot create json for response";
            sendResponse(response, HTTP_RESP_INTERNAL, "Cannot create json for response");
            return;
        }
        object->insert(HTTP_API_LIST_LABEL_KEY, JsonStringPtr(new JsonString(child->getLabel())));
        object->insert(HTTP_API_LIST_IS_LEAF_KEY, JsonBooleanPtr(new JsonBoolean(child->isLeafNode())));
        array->append(object);
    }
    response->headers[HTTP_API_CONTENT_TYPE_KEY] = HTTP_API_JSON_CONTENT_TYPE;
    string responseBody = array->dump();
    VLOG(1) << "List path '" << path <<"': " << responseBody;
    sendResponse(response, HTTP_RESP_OK, "OK List Metrics", responseBody);
}

RA_END_NAMESPACE(service);
