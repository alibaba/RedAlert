#include <ra/process/TrendPolicyChecker.h>
#include <ra/process/TrendData.h>

using namespace std;
RA_USE_NAMESPACE(tree);
RA_USE_NAMESPACE(config);
RA_USE_NAMESPACE(alarm);

RA_BEGIN_NAMESPACE(process);

TrendPolicyChecker::TrendPolicyChecker() { 
}

TrendPolicyChecker::~TrendPolicyChecker() { 
}

bool TrendPolicyChecker::checkHostVal(const string& metric, const MetricDataItem& hostData,
        const vector<NormalizedMetricDataPtr> &metricDataVec)
{
    return checkValue(metric, hostData);
}

bool TrendPolicyChecker::checkHostTotalVal(const string& metric, 
        const MetricDataItem& hostTotalData,
        const vector<NormalizedMetricDataPtr> &metricDataVec)
{
    return checkValue(metric, hostTotalData);
}

bool TrendPolicyChecker::checkMetricTotalVal(const MetricDataItem& metricTotalData,
        const vector<NormalizedMetricDataPtr> &metricDataVec)
{
    assert(NULL != _policyItem);
    const string& metric = _policyItem->getMetric();
    return checkValue(metric, metricTotalData);
}

bool TrendPolicyChecker::checkValue(const string& metric, 
        const MetricDataItem& dataItem)
{
    TrendDataPtr trendData = _processPackage->getTrendData();
    if (trendData == NULL) {
        LOG(WARNING) << "Cannot check value: trend data is NULL";
        return false;
    }
    double forecastValue = 0.0;
    TrendKey key;
    key.host = dataItem.host;
    key.metric = metric;
    VLOG(1) << "metric:" << metric << ", host:" <<dataItem.host 
	    <<", value: " << dataItem.value << ", time:" << dataItem.time;
    if (trendData->getForecastVal(key, dataItem.time, forecastValue)) {
        VLOG(1) << "forcastValue:" << forecastValue;
        assert(NULL != _policyItem);
        if (_policyItem->isDiffTooMuch(dataItem.value, forecastValue)) {
            std::map<uint32_t, AbnormalVal>& abnormalHosts = _abnormalInfo[metric];
            assert(abnormalHosts.find(dataItem.host) == abnormalHosts.end());
            AbnormalVal abnormalVal;
            abnormalVal.value = dataItem.value;
            abnormalVal.forecastValue = forecastValue;
            abnormalHosts[dataItem.host] = abnormalVal;
            trendData->feedTrend(key, dataItem.time, dataItem.value);
            return false;
        }
    }
    trendData->feedTrend(key, dataItem.time, dataItem.value);
    return true;
}

string TrendPolicyChecker::getAlarmMsgContent(time_t curTime)
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

std::string TrendPolicyChecker::getAlarmMsgPrefix() const
{
    assert(NULL != _policyItem);
    char buffer[1024];
    snprintf(buffer, sizeof(buffer), "[id:%u][%s][%s]", _policyItem->getId(), 
             POLICY_TYPE_TREND.c_str(), _policyItem->getCheckTypeStr().c_str());
    return buffer;
}

RA_END_NAMESPACE(process);
