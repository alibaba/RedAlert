#include <sstream>
#include <tr1/functional>
#define HAVE_STDINT_H
#include <rrd.h>
#include <rrd_client.h>
#include <ra/common/Common.h>
#include <ra/common/CommonDefine.h>
#include <ra/common/Log.h>
#include <ra/util/FileUtil.h>
#include <ra/util/Util.h>
#include <ra/fetcher/MetricTreeUtil.h>
#include <ra/fetcher/MetricDataNode.h>
#include "RRDMetricFetcher.h"

using namespace std;
RA_USE_NAMESPACE(util);
RA_BEGIN_NAMESPACE(fetcher);
RA_LOG_SETUP(fetcher, RRDMetricFetcher);

RRDMetricFetcher::RRDMetricFetcher() {
    _triesNum = 3;
}

RRDMetricFetcher::~RRDMetricFetcher() {
}

bool RRDMetricFetcher::init(const OptionMap &options) {
    _rrdRootDir = "";
    OptionMap::const_iterator it = options.find(RRD_ROOT);
    if (it != options.end()) {
        _rrdRootDir = it->second;
    }


    if (_rrdRootDir == "") {
        RA_LOG(ERROR, "Fail to init. Miss %s and %s", RRD_ROOT.c_str());
        return false;
    }

    return true;
}

void RRDMetricFetcher::close() {
}

MetricNodePtr RRDMetricFetcher::allocTree(int64_t start, int64_t end) {
    MetricNodePtr root(new MetricNode(METRIC_ROOT_NODE_LABEL));
    string path = "";
    bool ok = traverse(path, root);
    if (ok) {
        return root;
    }
    return MetricNodePtr(); 
}

bool RRDMetricFetcher::getMetricName(const string &metricFileName, string &metricName) {
    size_t suffixPos = metricFileName.rfind(RRD_FILE_SUFFIX);
    if (suffixPos == string::npos || suffixPos != (metricFileName.size() - RRD_FILE_SUFFIX.size())) {
        RA_LOG(WARN, "Invalidate filename [%s]", metricFileName.c_str()); 
        return false;
    }
    metricName = metricFileName.substr(0,suffixPos);    
    return true;
}

void RRDMetricFetcher::compareAndAddChild(MetricNodePtr &parent, const string &label) {
    const vector<MetricNodePtr> childrenNodes = parent->getChildren();
    for (vector<MetricNodePtr>::const_iterator it = childrenNodes.begin();
        it != childrenNodes.end();
        ++it) {
        if ((*it)->getLabel() == label) {
            parent = *it;
            return;
        }
    }
    MetricNodePtr newMetricNode(new MetricNode(label));
    parent->addChild(newMetricNode);
    parent = newMetricNode;    
}

bool RRDMetricFetcher::traverse(string &path, MetricNodePtr &root) const {
    string absolutePath = FileUtil::joinFilePath(_rrdRootDir, path);
    bool ok = FileUtil::isDir(absolutePath);
    if (ok) {
        vector<string> names;
        if (!FileUtil::listDir(absolutePath, names, false)) {  
            RA_LOG(ERROR, "Fail to listDir [%s]", absolutePath.c_str()); 
            return false;
        }

        for (size_t i = 0; i < names.size(); i++) {
            Util::rtrim(names[i], '/');
            if (names[i] == SUMMARY_INFO) {
                continue;
            }
            string xPath = FileUtil::joinFilePath(path, names[i]);
            traverse(xPath, root);
        }
        return true;
    } else {
        vector<string> elems = util::Util::splitString(path, "/");
        if (elems.size() <= (size_t)2) {
            RA_LOG(ERROR, "Invalidate Path=%s", path.c_str());
            return false;
        }
        string metricName;
        if (!getMetricName(elems[elems.size() - 1], metricName)) {
            RA_LOG(ERROR, "Invalidate Path=%s", path.c_str());
            return false;
        }

        MetricNodePtr parent = root;
        for (size_t i = 0; i < (elems.size() - 2); ++i) {
            compareAndAddChild(parent, elems[i]);
        }

        compareAndAddChild(parent, metricName);
        return true;
    }
    RA_LOG(ERROR, "It is impossible to come here");
    return false;
}

MetricNodePtr RRDMetricFetcher::retrieve(
        const MetricNodePtr &root, int64_t start, int64_t end, int64_t step) {
    if (root == NULL) {
        return MetricNodePtr();
    }
    map<string, std::vector<std::string> > hostNameBuf;
    MetricNodePtr clone = root->cloneTree();
    bool ok = MetricTreeUtil::traverseTree(clone,
            tr1::bind(&RRDMetricFetcher::readRRDData, this, tr1::placeholders::_1, start, end, step, tr1::ref(hostNameBuf)));
    if (ok) {
        return clone;
    }
    return MetricNodePtr();
}

bool RRDMetricFetcher::readRRDData(
        const MetricPath& path, int64_t start, int64_t end, int64_t step,
        map< string, vector<string> >& hostNameBuf) {
    if (path.empty()) {
        return true;
    }
    string rPathBeforeHost;
    //The first node is not used!
    for(size_t i = 1; i < path.size() - 1; i++) {
        rPathBeforeHost = rPathBeforeHost + "/" + path[i]->getLabel();
    }
    map<string, vector<string> >::const_iterator bufIter = hostNameBuf.find(rPathBeforeHost);
    if (bufIter == hostNameBuf.end()) {
            string absolutePath = FileUtil::joinFilePath(_rrdRootDir, rPathBeforeHost);
            bool ok = FileUtil::isDir(absolutePath);
            if (ok) {
                vector<string> names;
                if (!FileUtil::listDir(absolutePath, names, false)) {
                    RA_LOG(WARN, "List %s failed!", absolutePath.c_str());
                    return true;
                }
                for(size_t j = 0; j < names.size(); ++j) {
                    Util::rtrim(names[j], '/');
                    if (names[j] == "" || names[j] == SUMMARY_INFO || names[j].find(".") == 0) {
                        continue;
                    }

                    hostNameBuf[rPathBeforeHost].push_back(names[j]);;
                }
            } else {
                return true;
            }
    }
    const vector<string> hosts = hostNameBuf[rPathBeforeHost];
    const MetricNodePtr &parent = *path.rbegin();
    const string &metricName = path[path.size() - 1]->getLabel();
    for (vector<string>::const_iterator iter = hosts.begin();
         iter != hosts.end();
         ++iter) {
        string realPath = _rrdRootDir + "/" + rPathBeforeHost + "/" 
                          + *iter + "/" + metricName + RRD_FILE_SUFFIX;
        if (!FileUtil::localFileExist(realPath)) {
            continue;
        }

        MetricDataPtr data(new MetricData());
        data->hostname = *iter;

        getMetricValue(realPath, (time_t)start, (time_t)end, step, data);
        MetricNodePtr dataNode(new MetricDataNode(data));
        parent->addChild(dataNode);

    }
    return true;
}

bool RRDMetricFetcher::getMetricValue(string& rrdPath,
                                      time_t start, time_t end, uint64_t step,
                                      MetricDataPtr data) {
    const char *args[7];
    args[0] = "fetch";
    args[1] = rrdPath.c_str();
    args[2] = "AVERAGE";
    args[3] = "-s";
    sprintf(startBuf, "%ld", start);
    args[4] = startBuf;
    args[5] = "-e";
    sprintf(endBuf, "%ld", end);
    args[6] = endBuf;
    // args[3] = "-r";
    // args[4] = "10";
    unsigned long ds_cnt = 1;
    char** ds_namv;
    rrd_value_t *values = NULL;
    time_t rStart;
    time_t rEnd;
    int ret = rrd_fetch(7, (char **)args,
                        &rStart, &rEnd, (long unsigned int*)&step,
                        &ds_cnt, &ds_namv, &values);

    if (values != NULL) {
        rrd_value_t *valuecursor = values;
        for(time_t tt = rStart + step; tt <= rEnd; tt += step) {
            for(unsigned long i = 0; i < ds_cnt; i++) {
                string dsName(ds_namv[i]);
                if(dsName == "sum") {
                    data->times->push_back(tt);
                    data->values->push_back(*(valuecursor++));
                }
            }
        }
        for(unsigned long i = 0; i < ds_cnt; i++) {
            free(ds_namv[i]);
        }

        free(values);
        free(ds_namv);
    }

    if (ret != 0) {
        RA_LOG(ERROR, "rrd_fetch failed. rrdPath=%s, errorString=%s", rrdPath.c_str(), rrd_get_error());
        return false;
    }
    return true;
}

RA_END_NAMESPACE(fetcher);
