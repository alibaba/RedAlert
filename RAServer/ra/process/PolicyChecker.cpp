#include <tr1/functional>
#include <ra/util/Util.h>
#include <ra/process/PolicyChecker.h>
#include <ra/fetcher/MetricFetcher.h>
#include <ra/fetcher/MetricTreeUtil.h>

using namespace std;

RA_USE_NAMESPACE(fetcher);
RA_USE_NAMESPACE(tree);
RA_USE_NAMESPACE(config);
RA_USE_NAMESPACE(util);
RA_USE_NAMESPACE(alarm);
RA_USE_NAMESPACE(config);

RA_BEGIN_NAMESPACE(process);

PolicyChecker::PolicyChecker() 
    : _hasNoMetric(false)
{ 
}

PolicyChecker::~PolicyChecker() { 
}

bool PolicyChecker::init(
    const tree::ProcessPackagePtr &processPackage)
{
    if (processPackage == NULL) {
        LOG(ERROR) << "Cannot initialize policy checker: process package is NULL";
        return false;
    }
    _processPackage = processPackage;
    _requestPackage = _processPackage->getRequestPackage();
    if (_requestPackage == NULL) {
        LOG(ERROR) << "Cannot initialize policy checker: request package is NULL";
        return false;
    }
    _configWrapper = _requestPackage->getConfig();
    if (_configWrapper == NULL) {
        LOG(ERROR) << "Cannot initialize policy checker: config wrapper is NULL";
        return false;
    }
    _alarmMsg.reset(new AlarmMsg(_processPackage, CODE_CRITICAL));
    if (_alarmMsg == NULL) {
        LOG(ERROR) << "Cannot initialize policy checker: alarm message is NULL";
        return false;
    }
    return true;
}

alarm::AlarmMsgPtr PolicyChecker::check()
{
    if (NULL == _processPackage) {
        LOG(ERROR) << "cannot do check without init PolicyChecker";
        return _alarmMsg;
    }
    vector<NormalizedMetricDataPtr> normalizedMetricDataVec;
    genNormalizedData(normalizedMetricDataVec);
    if (normalizedMetricDataVec.empty()) {
        LOG(WARNING) << "cannot find any metric data";
        return _alarmMsg;
    }
    
    assert(NULL != _processPackage);
    const RequestPackagePtr& requestPackage = _processPackage->getRequestPackage();
    assert(NULL != requestPackage);
    CheckType checkType = requestPackage->getCheckType();
    switch (checkType) {
    case CT_SINGLE:
        checkSingleHost(normalizedMetricDataVec);
        break;
    case CT_HOST_TOTAL:
        _fetchNoValidDataMap.clear();
        checkHostTotal(normalizedMetricDataVec);
        break;
    case CT_METRIC_TOTAL:
        _fetchNoValidDataMap.clear();
        checkMetricTotal(normalizedMetricDataVec);
        break;
    default:
        LOG(ERROR) << "unkown checkType, can not check";
        break;
    }
    time_t curTime = Util::currentTimeInSeconds();
    string alarmMsgStr = getAlarmMsgContent(curTime);
    if (!alarmMsgStr.empty()) {
        alarmMsgStr = getAlarmMsgPrefix() + alarmMsgStr;
    }
    _alarmMsg->setAlarmMsgStr(alarmMsgStr);
    return _alarmMsg;
}

void PolicyChecker::checkSingleHost(const vector<NormalizedMetricDataPtr> &metricDataVec)
{
    assert(metricDataVec.size() > 0);
    const NormalizedMetricDataPtr& currMetricData = metricDataVec[0];
    assert(NULL != currMetricData);
    set<MetricDataItem> dataItemSet;

    assert(NULL != _processPackage);
    const RequestPackagePtr& requestPackage = _processPackage->getRequestPackage();
    assert(NULL != requestPackage);
    const StringSet& metricSet = requestPackage->getMetricSet();
    if (metricSet.empty()) {
        _hasNoMetric = true;
        return;
    }
    StringSet::const_iterator metricIter = metricSet.begin();
    for (; metricIter != metricSet.end(); ++metricIter) {
        const string& metric = *metricIter;
        dataItemSet = currMetricData->getMetricData(metric);
        if (dataItemSet.empty()) {
            LOG(WARNING) << "metric:" << metric << " has no valid data";
            continue;
        }
        set<MetricDataItem>::const_iterator dataIter = dataItemSet.begin();
        for (; dataIter != dataItemSet.end(); ++dataIter) {
            const MetricDataItem& hostData = *dataIter;
            checkHostVal(metric, hostData, metricDataVec);
        }
    }
}

void PolicyChecker::checkHostTotal(vector<NormalizedMetricDataPtr> &metricDataVec)
{
    vector<NormalizedMetricDataPtr>::iterator it = metricDataVec.begin();
    for (; it != metricDataVec.end(); ++it) {
        NormalizedMetricDataPtr& metricDataPtr = *it;
        metricDataPtr->genHostTotalData();
    }
    assert(metricDataVec.size() > 0);
    const NormalizedMetricDataPtr& currMetricData = metricDataVec[0];
    assert(NULL != currMetricData);

    assert(NULL != _processPackage);
    const RequestPackagePtr& requestPackage = _processPackage->getRequestPackage();
    assert(NULL != requestPackage);
    const StringSet& metricSet = requestPackage->getMetricSet();
    if (metricSet.empty()) {
        _hasNoMetric = true;
        return;
    }
    StringSet::const_iterator metricIter = metricSet.begin();
    for (; metricIter != metricSet.end(); ++metricIter) {
        const string& metric = *metricIter;
        MetricDataItem hostTotalData;
        if (!currMetricData->getHostTotalData(metric, hostTotalData)) {
            _missedMetric.insert(metric);
            continue;
        }
        checkHostTotalVal(metric, hostTotalData, metricDataVec);
    }
}

void PolicyChecker::checkMetricTotal(vector<NormalizedMetricDataPtr> &metricDataVec)
{
    vector<NormalizedMetricDataPtr>::iterator it = metricDataVec.begin();
    for (; it != metricDataVec.end(); ++it) {
        NormalizedMetricDataPtr& metricDataPtr = *it;
        metricDataPtr->genMetricTotalVal();
    }
    assert(metricDataVec.size() > 0);
    const NormalizedMetricDataPtr& currMetricData = metricDataVec[0];
    assert(NULL != currMetricData);

    assert(NULL != _processPackage);
    const RequestPackagePtr& requestPackage = _processPackage->getRequestPackage();
    assert(NULL != requestPackage);
    const StringSet& metricSet = requestPackage->getMetricSet();
    if (metricSet.empty()) {
        _hasNoMetric = true;
        return;
    }

    MetricDataItem metricTotalData;
    if (currMetricData->getMetricTotalData(metricTotalData)) {
        checkMetricTotalVal(metricTotalData, metricDataVec);
    }
    else {
        PolicyConfigItemBasePtr policyItemBase = requestPackage->getPolicyItem();
        assert(NULL != policyItemBase);
        const string& metric = policyItemBase->getMetric();
        _missedMetric.insert(metric);
    }
}

void PolicyChecker::genNormalizedData(vector<NormalizedMetricDataPtr>& normalizedMetricDataVec)
{
    assert(NULL != _processPackage);
    const vector<MetricNodePtr>& dataRoots = _processPackage->getMetricDataRoots();
    VLOG(1) << "Get " << dataRoots.size() <<" metric data roots";
    normalizedMetricDataVec.assign(dataRoots.size(), NormalizedMetricDataPtr());
    for (size_t i = 0; i < dataRoots.size(); ++i) {
        normalizedMetricDataVec[i].reset(new NormalizedMetricData());
        genNormalizedData(dataRoots[i], normalizedMetricDataVec[i], 0U == i);
    }
}

void PolicyChecker::genNormalizedData(
    const MetricNodePtr& dataRoot, NormalizedMetricDataPtr& normalizedMetricData,
    bool updateFetchNoValidData) {
    MetricTreeUtil::traverseTree(dataRoot, tr1::bind(&PolicyChecker::genNormalizedDataFromMetric,
            this, tr1::placeholders::_1, normalizedMetricData, updateFetchNoValidData));
}

bool PolicyChecker::genNormalizedDataFromMetric(
    const MetricPath& metricPath, NormalizedMetricDataPtr& normalizedMetricData,
    bool updateFetchNoValidData)
{
    MetricDataPtr metricData;
    vector<string> metricPathStr;
    for (size_t i = 0; i < metricPath.size(); i++) {
        const MetricNodePtr &metricNode = metricPath[i];
        // skip root node
        if (i == 0 && metricNode->getLabel() == METRIC_ROOT_NODE_LABEL) {
            continue;
        }
        if (metricNode->isDataNode()) {
            metricData = metricNode->getData();
        } else {
            metricPathStr.push_back(metricNode->getLabel());
        }
    }
    string metricStr = Util::joinString(metricPathStr, METRIC_NODE_PATH_SEP);
    if (metricData == NULL) {
        LOG(WARNING) << "metric:" << metricStr << " has no valid data node";
        return true;
    }

    const string &hostStr = metricData->hostname;
    uint32_t hostIp = 0U;
    Util::StringToIP(hostStr, hostIp);
    if (isFiltered(metricStr, hostIp)) {
        VLOG(1) << "data of metric:" << metricStr << ", host:" << hostStr <<" is filted";
        return true;
    }
    double value = NormalizedMetricData::getValue(*metricData->values);
    if (value == INVALID_METRIC_DATA) {
        LOG(WARNING) << "metric:" << metricStr << ", host:" << hostStr << " has no valid data";
        if (updateFetchNoValidData) {
            _fetchNoValidDataMap[metricStr].insert(hostIp);
        }
        return true;
    }
    uint64_t time = NormalizedMetricData::getTime(*metricData->times);
    MetricDataItem item;
    item.host = hostIp;
    assert(time != 0);
    item.time = time;
    item.value = value;
    item.rawValue = metricData->values;
    item.rawTime = metricData->times;
    VLOG(1) << "Normalized metric data item, metric: '" << metricStr 
	    <<"', host: '" << hostStr <<"', value: " << value << ", time: " << time;
    if (!normalizedMetricData->addMetricData(metricStr, item)) {
        LOG(WARNING) <<  "metric[" << metricStr << "], host[" << hostStr <<"] is retrived by multi amonitor";
    }
    return true;
}

bool PolicyChecker::isFiltered(const string&metric, uint32_t host) const
{
    assert(NULL != _processPackage);
    const RequestPackagePtr& requestPackage = _processPackage->getRequestPackage();
    assert(NULL != requestPackage);
    PolicyConfigItemBasePtr policyItemBase = requestPackage->getPolicyItem();
    assert(NULL != policyItemBase);
    if (policyItemBase->isFiltered(metric, host)) {
        return true;
    }
    return false;
}

string PolicyChecker::getAlarmMsgContent(time_t curTime)
{
    assert(NULL != _processPackage);
    const RequestPackagePtr& requestPackage = _processPackage->getRequestPackage();
    assert(NULL != requestPackage);
    const config::ConfigWrapperPtr config = requestPackage->getConfig();
    assert(NULL != config);
    PolicyConfigItemBasePtr policyItemBase = requestPackage->getPolicyItem();
    assert(NULL != policyItemBase);
    const string& policyMetric = policyItemBase->getMetric();
    const string& group = policyItemBase->getGroup();

    string ret = "";
    if (_hasNoMetric) {
        uint32_t host = 0;
        if (config->isShielded(curTime, group, policyMetric, host)) {
            LOG(INFO) << policyMetric << ":has no matched metric, but is shielded!";
        }
        else {
            ret.append(policyMetric + ":has no matched metric");
        }
        return ret;
    }
    
    map<string, set<uint32_t> >::iterator metricIter =  _fetchNoValidDataMap.begin();
    LOG(ERROR) << "_fetchNoValidDataMap size=" << _fetchNoValidDataMap.size();
    while(metricIter != _fetchNoValidDataMap.end()) {
        const string& metric = metricIter->first;
        set<uint32_t>& hostSet = metricIter->second;
        vector<string> hostIpVec;
        set<uint32_t>::iterator hostIter = hostSet.begin();
        while(hostIter != hostSet.end()) {
            uint32_t host = *hostIter;
            if (config->isShielded(curTime, group, metric, host)) {
                LOG(INFO) << group << "." << metric << "." << host <<" is shielded";
                hostSet.erase(hostIter++);
            }
            else {
                string ipStr = Util::IPToString(host);
                hostIpVec.push_back(ipStr);
                ++hostIter;
            }
        }
        if (hostSet.empty()) {
            _fetchNoValidDataMap.erase(metricIter++);
        }
        else {
            ++metricIter;
            assert(!hostIpVec.empty());
            string tmpRet = metric + ":" + Util::joinString(hostIpVec, ",");
            if (!ret.empty()) {
                ret.append(";");
            }
            ret.append(tmpRet);
        }
    }
    if (!ret.empty()) {
        assert(_missedMetric.empty());
        return "Fetch no valid data:" + ret;
    }

    set<string>::iterator it = _missedMetric.begin();
    while (it != _missedMetric.end()) {
        const string& metric = *it;
        uint32_t host = 0;
        if (config->isShielded(curTime, group, metric, host)) {
            LOG(INFO) << group << "." << metric << " fetch no valid data, but is shielded";
            _missedMetric.erase(it++);
        }
        else {
            ++it;
        }
    }
    if (!_missedMetric.empty()) {
        vector<string> missedMetricVec(_missedMetric.begin(), _missedMetric.end());
        return "Fetch no valid data:[" + Util::joinString(missedMetricVec, ",") + "]";
    }
    return ret;
}

RA_END_NAMESPACE(process);

