#include <ra/process/CyclePolicyChecker.h>
#include <ra/util/Util.h>

using namespace std;
RA_USE_NAMESPACE(tree);
RA_USE_NAMESPACE(config);
RA_USE_NAMESPACE(alarm);
RA_USE_NAMESPACE(util);

RA_BEGIN_NAMESPACE(process);

CyclePolicyChecker::CyclePolicyChecker() { 
}

CyclePolicyChecker::~CyclePolicyChecker() { 
}

bool CyclePolicyChecker::checkHost(const string& metric,
                                   const MetricDataItem& hostData,
                                   const vector<NormalizedMetricDataPtr>& metricDataVec,
                                   bool isTotal)
{
    assert(metricDataVec.size() > 1U);
    vector<double> historyValVec;
    vector<int32_t> missedCycle;
    for (size_t i = 1; i < metricDataVec.size(); ++i) {
        const NormalizedMetricDataPtr& historyNormalizedMetricData = metricDataVec[i];
        assert(NULL != historyNormalizedMetricData);
        MetricDataItem dataItem;
        bool flag = false;
        if (isTotal)
        {
            flag = historyNormalizedMetricData->getHostTotalData(metric,
                    dataItem);
        }
        else
        {
            flag = historyNormalizedMetricData->getMetricData(metric,
                    hostData.host, dataItem);
        }
        if (flag) {
            historyValVec.push_back(dataItem.value);
        } else {
            missedCycle.push_back(i);
        }
    }
    if (!missedCycle.empty()) {
        char buffer[1024];
        for (size_t i = 0; i < missedCycle.size(); ++i) {
            snprintf(buffer, sizeof(buffer), "%d ", missedCycle[i]);
        }
        if (isTotal) {
            LOG(WARNING) << "metric:" << metric <<", miss history cyclye " << buffer;
        } else {
            string hostIpStr;
            Util::IPToString(hostData.host, hostIpStr);
            LOG(WARNING) << "metric:" << metric << ", host:" 
			 << hostIpStr << ", miss history cyclye[" << buffer << "]";
        }
    }
    if(historyValVec.empty()) {
        return true;
    }
    return checkValue(metric, hostData, historyValVec);
}

bool CyclePolicyChecker::checkHostVal(const string& metric,
                                      const MetricDataItem& hostData,
                                      const vector<NormalizedMetricDataPtr>& metricDataVec)
{
    return checkHost(metric, hostData, metricDataVec, false);
}
    
bool CyclePolicyChecker::checkHostTotalVal(const string& metric,
        const MetricDataItem& hostTotalData,
        const vector<NormalizedMetricDataPtr>& metricDataVec)
{
    return checkHost(metric, hostTotalData, metricDataVec, true);
}
    
bool CyclePolicyChecker::checkMetricTotalVal(const MetricDataItem& metricTotalData,
        const vector<NormalizedMetricDataPtr>& metricDataVec)
{
    assert(metricDataVec.size() > 1U);
    assert(NULL != _policyItem);
    const string& metric = _policyItem->getMetric();
    vector<double> historyValVec;
    vector<int32_t> missedCycle;
    for (size_t i = 1; i < metricDataVec.size(); ++i) {
        const NormalizedMetricDataPtr& historyNormalizedMetricData = metricDataVec[i];
        assert(NULL != historyNormalizedMetricData);
        MetricDataItem dataItem;
        if (historyNormalizedMetricData->getMetricTotalData(dataItem)) {
            historyValVec.push_back(dataItem.value);
        }
        else {
            missedCycle.push_back(i);
        }
    }
    if (!missedCycle.empty()) {
        char buffer[1024];
        for (size_t i = 0; i < missedCycle.size(); ++i) {
            snprintf(buffer, sizeof(buffer), "%d ", missedCycle[i]);
        }
        LOG(WARNING) << "metric:" << metric << ", miss history cyclye[" << buffer << "]";
    }
    if(historyValVec.empty()) {
        return true;
    }
    return checkValue(metric, metricTotalData, historyValVec);
}


bool CyclePolicyChecker::checkValue(const string& metric,
                                    const MetricDataItem& dataItem, 
                                    const vector<double>& historyValVec)
{
    assert(NULL != _policyItem);
    if (_policyItem->isDiffTooMuch(dataItem.value, historyValVec)) {
        std::map<uint32_t, AbnormalVal>& abnormalHosts = _abnormalInfo[metric];
        assert(abnormalHosts.find(dataItem.host) == abnormalHosts.end());
        AbnormalVal abnormalVal;
        abnormalVal.value = dataItem.value;
        abnormalVal.historyValVec = historyValVec;
        abnormalHosts[dataItem.host] = abnormalVal;
        return false;
    }
    return true;
}

string CyclePolicyChecker::getAlarmMsgContent(time_t curTime)
{
    removeShieldAbnormalInfo(curTime, _abnormalInfo);
    string ret;
    genAbnormalStr(_abnormalInfo, ret);
    const string& baseAlarmContent = PolicyChecker::getAlarmMsgContent(curTime);
    if (!ret.empty() && !baseAlarmContent.empty()) {
        ret.append(";" + baseAlarmContent);
    }
    else {
        ret.append(baseAlarmContent);
    }
    return ret;    
}

string CyclePolicyChecker::getAlarmMsgPrefix() const
{
    assert(NULL != _policyItem);
    char buffer[1024];
    snprintf(buffer, sizeof(buffer), "[id:%u][%s][%s]", _policyItem->getId(), 
             POLICY_TYPE_CYCLE.c_str(), _policyItem->getCheckTypeStr().c_str());
    return buffer;
}

RA_END_NAMESPACE(process);

