#include <sstream>
#include <jansson.h>
#include <ra/util/Util.h>
#include <ra/fetcher/MetricDataNode.h>
#include <ra/fetcher/MetricTreeUtil.h>
#include "JsonMemoryWrapper.h"
#include "GraphiteMetricFetcher.h"

RA_BEGIN_NAMESPACE(fetcher);
RA_USE_NAMESPACE(common); 
using namespace std;

RA_LOG_SETUP(fetcher, GraphiteMetricFetcher);

GraphiteMetricFetcher::GraphiteMetricFetcher(): _httpClient(true,3), _isBatchAllocTree(false) {
}

GraphiteMetricFetcher::~GraphiteMetricFetcher() { 
}

bool GraphiteMetricFetcher::init(const OptionMap &options){
    OptionMap::const_iterator it = options.find(BATCH_ALLOC_TREE);
    if (it != options.end()) {
        _isBatchAllocTree = ((it->second == "true") || (it->second == "True") || (it->second == "TRUE"));
    }

    it = options.find(ACCESS_HOST);
    if (it != options.end()) {
        _accessHost = it->second;
        util::Util::rtrim(_accessHost, '/');
        return _httpClient.init();
    }

    return false;
}

void GraphiteMetricFetcher::getMetricsURL(const string &metricPath, string &url) {
    url = _accessHost + "/metrics/find?query=" + metricPath;
}

void GraphiteMetricFetcher::getAllMetricsURL(string &url) {
    url = _accessHost + "/metrics/index.json";
}

void GraphiteMetricFetcher::getDataURL(const string &metricPath, int64_t start, 
                                       int64_t end, string &url) {
    stringstream ss;
    ss <<  _accessHost <<  "/render?target=" << metricPath << "&format=json";
    if (start > 0) {
        ss << "&from=" << start;
    }
    if (end >0) {
        ss << "&until=" << end;
    }
    url = ss.str();
}

bool GraphiteMetricFetcher::getSubMetrics(const std::string &parentMetricPath, map<string, int> &subMetrics) {
    string url;
    getMetricsURL(parentMetricPath, url);
    HttpResponse response;
    if (!_httpClient.get(url, &response)) {
        RA_LOG(ERROR, "http client get [%s] fail", url.c_str());
        return false;
    }

    if (response.status != HTTP_RESP_OK ) {
        RA_LOG(ERROR, "response status[%d] is not ok. url=[%s]", response.status, url.c_str());
        return false;
    }

    if (!parseSubMetrics(response.body, subMetrics)) {
        RA_LOG(ERROR, "parseSubMetrics [%s] fail, url=[%s]", response.body.c_str(),url.c_str());
        return false;
    }
    return true;
}

bool GraphiteMetricFetcher::getAllMetrics(set<string> &metrics) {
    string url;
    getAllMetricsURL(url);
    HttpResponse response;
    RA_LOG(ERROR, "http client get [%s]", url.c_str());
    if (!_httpClient.get(url, &response)) {
        RA_LOG(ERROR, "http client get [%s] fail", url.c_str());
        return false;
    }

    if (response.status != HTTP_RESP_OK ) {
        RA_LOG(ERROR, "response status[%d] is not ok. url=[%s]", response.status, url.c_str());
        return false;
    }
    
    if (!parseAllMetrics(response.body, metrics)) {
        RA_LOG(ERROR, "parseSubMetricData [%s] fail, url=[%s]", response.body.c_str(), url.c_str());
        return false;
    }
    return true;
}

bool GraphiteMetricFetcher::getData(const string &metricPath, int64_t start, int64_t end, map<uint64_t, double> &data) {
    string url;
    getDataURL(metricPath, start, end, url);
    RA_LOG(DEBUG, "Http request url: %s", url.c_str());

    HttpResponse response;
    if (!_httpClient.get(url, &response)) {
        RA_LOG(ERROR, "http client get [%s] fail", url.c_str());
        return false;
    }
    RA_LOG(DEBUG, "Response status: %d, message: %s, body: %s", response.status, response.message.c_str(), response.body.c_str());

    if (response.status != HTTP_RESP_OK ) {
        RA_LOG(ERROR, "response status[%d] is not ok. url=[%s]", response.status, url.c_str());
        return false;
    }
    
    if (!parseSubMetricData(response.body, data, metricPath)) {
        RA_LOG(ERROR, "parseSubMetricData [%s] fail, url=[%s]", response.body.c_str(), url.c_str());
        return false;
    }

    return true;
}

bool GraphiteMetricFetcher::parseAllMetrics(const string &content, set<string> &metrics) {
    json_error_t error;
    json_t *json = json_loads(content.c_str(), 0, &error);
    _wrap_json_memeory(json);
    if (!json) {
        RA_LOG(ERROR, "Fail to parse %s", content.c_str());
        return false;
    }

    if (!json_is_array(json)) {
        RA_LOG(ERROR, "Invalidate json content %s", content.c_str());
        return false;
    }

    for (size_t i = 0; i < json_array_size(json); ++i) {
        json_t *item = json_array_get(json, i);
        if (!item) {
            RA_LOG(ERROR, "Invalidate json content %s", content.c_str());
            return false;
        }
        if (!json_is_string(item)) {
            RA_LOG(ERROR, "Invalidate json content %s", content.c_str());
            return false;
        }
        metrics.insert(json_string_value(item));
    }
    return true;
}

bool GraphiteMetricFetcher::parseSubMetrics(const string &content, map<string, int> &subMetrics) {
    json_error_t error;
    json_t *json = json_loads(content.c_str(), 0, &error);
    _wrap_json_memeory(json);
    if (!json) {
        RA_LOG(ERROR, "Fail to parse %s", content.c_str());
        return false;
    }

    if (!json_is_array(json)) {
        RA_LOG(ERROR, "Invalidate json content %s", content.c_str());
        return false;
    }

    for (size_t i = 0; i < json_array_size(json); ++i) {
        json_t *item = json_array_get(json, i);
        if (!item) {
            RA_LOG(ERROR, "Invalidate json content %s", content.c_str());
            return false;
        }
        if (!json_is_object(item)) {
            RA_LOG(ERROR, "Invalidate json content %s", content.c_str());
            return false;
        }
        json_t *text = json_object_get(item, "text");
        if (!text || !json_is_string(text)) {
            RA_LOG(ERROR, "Invalidate json content %s", content.c_str());
            return false;
        }
        string mertricNode(json_string_value(text));
        json_t *leaf = json_object_get(item, "leaf");
        if (!leaf || !json_is_number(leaf)) {
            RA_LOG(ERROR, "Invalidate json content %s", content.c_str());
            return false;
        }
        subMetrics[mertricNode] = json_integer_value(leaf);
    }
    return true;
}

bool GraphiteMetricFetcher::parseSubMetricData(const string &content, map<uint64_t, double> &metricData, const string &metricPath) {
    json_error_t error;
    json_t *json = json_loads(content.c_str(), 0, &error);
    _wrap_json_memeory(json);
    if (!json) {
        RA_LOG(ERROR, "Fail to parse %s", content.c_str());
        return false;
    }

    if (!json_is_array(json)) {
        RA_LOG(ERROR, "Invalidate json content %s", content.c_str());
        return false;
    }

    for (size_t i = 0; i < json_array_size(json); ++i) {
        json_t *item = json_array_get(json, i);
        if (!item) {
            RA_LOG(ERROR, "Invalidate json content %s", content.c_str());
            return false;
        }
        if (!json_is_object(item)) {
            RA_LOG(ERROR, "Invalidate json content %s", content.c_str());
            return false;
        }
        json_t *target = json_object_get(item, "target");
        if (!target || !json_is_string(target)) {
            continue;
        }
        
        if (metricPath != json_string_value(target)) {
            continue;
        }
        
        json_t *datapoints = json_object_get(item, "datapoints");
        if (!datapoints || !json_is_array(datapoints)) {
            RA_LOG(ERROR, "Invalidate json content %s", content.c_str());
            return false;
        }
        for (size_t j = 0; j < json_array_size(datapoints); ++j) {
            json_t *dataPointItem =  json_array_get(datapoints, j);
            if (!dataPointItem || !json_is_array(dataPointItem)) {
                RA_LOG(ERROR, "Invalidate json content %s", content.c_str());
                return false;
            }
            
            if (json_array_size(dataPointItem) != 2) {
                RA_LOG(ERROR, "Invalidate json content %s", content.c_str());
                return false;
            }
            json_t *value = json_array_get(dataPointItem, 0);
            if (!value || json_is_null(value) || !json_is_number(value)) {
                continue;
            }

            json_t *time = json_array_get(dataPointItem, 1);
            if (!time || !json_is_number(time)) {
                continue;
            }

            metricData[(uint64_t)json_integer_value(time)] = json_real_value(value);
        }
    }

    return true;
}

void GraphiteMetricFetcher::close() {
}

MetricNodePtr GraphiteMetricFetcher::allocTree(int64_t start, int64_t end) {
    MetricNodePtr root(new MetricNode(METRIC_ROOT_NODE_LABEL));
    bool isAllocOk = false;
    if (_isBatchAllocTree) {
        isAllocOk = batchAllocTree(root);
    } else {
        isAllocOk = traverse(string(), root);
    }

    if (isAllocOk) {
        return root;
    }
    return MetricNodePtr();
}

bool GraphiteMetricFetcher::makeTree(const set<string> &metrics, MetricNodePtr &metricNode) {
    if (metricNode == NULL) {
        metricNode.reset(new MetricNode(METRIC_ROOT_NODE_LABEL));
    }

    for (set<string>::iterator iter = metrics.begin(); iter != metrics.end(); ++iter) {
        MetricNodePtr curParent = metricNode;
        vector<string> elems = util::Util::splitString(*iter, ".");
        for(size_t i = 0; i < elems.size(); ++i) {
            const vector<MetricNodePtr> childrenNodes = curParent->getChildren();
            vector<MetricNodePtr>::const_iterator cIter = childrenNodes.begin();
            for (; cIter != childrenNodes.end(); ++cIter) {
                if ((*cIter)->getLabel() == elems[i]) {
                    curParent = *cIter;
                    break;
                }
            }
            if (cIter == childrenNodes.end()) {
                MetricNodePtr newMetricNode(new MetricNode(elems[i]));
                curParent->addChild(newMetricNode);
                curParent = newMetricNode;
            }
        }
    }
    return true;
}
 
bool GraphiteMetricFetcher::batchAllocTree(MetricNodePtr &metricNode) {
    set<string> metrics;
    if (!getAllMetrics(metrics)) {
        RA_LOG(ERROR, "Fail to get all metrics");
        return false;
    }
    return makeTree(metrics, metricNode);
} 

bool GraphiteMetricFetcher::traverse(const string &parent, MetricNodePtr &metricNode) {
    string url;
    string metricPath;
    if (parent == "") {
        metricPath = "*";
    } else {
        metricPath = parent + ".*";
    }
    map<string, int> subMetrics;
    bool isSucc = getSubMetrics(metricPath, subMetrics);
    if (!isSucc) {
        RA_LOG(ERROR, "Fail to get sub-metrics [%s]", metricPath.c_str());
        return false;
    }
    map<string, int>::const_iterator iter = subMetrics.begin();
    for (;iter != subMetrics.end(); ++iter) {
        MetricNodePtr newMetricNode(new MetricNode(iter->first));
        metricNode->addChild(newMetricNode);
        if (iter->second != 1) {
            string sPath = "";
            if (parent == "") {
                sPath = iter->first;
            } else {
                sPath = parent + "." + iter->first;
            }

            if (!traverse(sPath, newMetricNode)) {
                RA_LOG(ERROR, "Traverse path[%s] failed", sPath.c_str());
                return false;
            }
        }
    }
    return true;
}

MetricNodePtr GraphiteMetricFetcher::retrieve(const MetricNodePtr &root, int64_t start, int64_t end, int64_t step) {
    if (root == NULL) {
        return MetricNodePtr();
    }

    MetricNodePtr clone = root->cloneTree();
    bool ok = MetricTreeUtil::traverseTree(clone,
            tr1::bind(&GraphiteMetricFetcher::readGraphiteData, this, tr1::placeholders::_1, start, end, step));
    if (ok) {
        return clone;
    }
    return MetricNodePtr(); 
}

bool GraphiteMetricFetcher::readGraphiteData(
        const MetricPath& path, int64_t start, int64_t end, int64_t step) {
    if (path.size() < 2) {
        return true;
    }
    start /= 1000000;
    end /= 1000000;
    step /= 1000000;
    string fullMetricPath = path[1]->getLabel();
    for (size_t i = 2; i < path.size(); i++) {
        fullMetricPath = fullMetricPath + "." + path[i]->getLabel();
    }
    
    map<uint64_t, double> oData;
    bool isSucc = getData(fullMetricPath, start, end, oData);
    if (!isSucc) {
        RA_LOG(ERROR, "Fail to getData from [%s]", fullMetricPath.c_str());
        return false;
    }
    
    const MetricNodePtr &parent = *path.rbegin();
    MetricDataPtr data(new MetricData());
    for (map<uint64_t, double>::iterator iter = oData.begin();
         iter != oData.end();
         ++iter) {
        data->times->push_back(iter->first * 1000000);
        data->values->push_back(iter->second);
    }
    MetricNodePtr dataNode(new MetricDataNode(data));
    parent->addChild(dataNode); 
    RA_LOG(DEBUG, "fullMetricPath=%s, start=%ld, end=%ld, dataCount=%zu", fullMetricPath.c_str(), start, end, data->times->size());
    return true;
}

RA_END_NAMESPACE(fetcher);
