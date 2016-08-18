#include <ra/process/ThresholdPolicyChecker.h>

using namespace std;
RA_USE_NAMESPACE(tree);
RA_USE_NAMESPACE(config);
RA_USE_NAMESPACE(alarm);
RA_USE_NAMESPACE(util);

RA_BEGIN_NAMESPACE(process);
RA_LOG_SETUP(process, ThresholdPolicyChecker);

ThresholdPolicyChecker::ThresholdPolicyChecker() { 
}

ThresholdPolicyChecker::~ThresholdPolicyChecker() { 
}

bool ThresholdPolicyChecker::checkHostVal(const string& metric, const MetricDataItem& hostData,
        const vector<NormalizedMetricDataPtr> &metricDataVec)
{
    return checkValue(metric, hostData);
}

bool ThresholdPolicyChecker::checkHostTotalVal(const string& metric, 
        const MetricDataItem& hostTotalData,
        const vector<NormalizedMetricDataPtr> &metricDataVec)
{
    return checkValue(metric, hostTotalData);
}

bool ThresholdPolicyChecker::checkMetricTotalVal(const MetricDataItem& metricTotalData,
        const vector<NormalizedMetricDataPtr> &metricDataVec)
{
    assert(NULL != _policyItem);
    const string& metric = _policyItem->getMetric();
    return checkValue(metric, metricTotalData);
}

bool ThresholdPolicyChecker::checkValue(const string& metric, 
        const MetricDataItem& dataItem)
{
    assert(NULL != _policyItem);
    if (dataItem.value > _policyItem->getUpBound() ||
        dataItem.value < _policyItem->getDownBound())
    {
        map<uint32_t, double>& abnormalHosts = _abnormalInfo[metric];
        assert(abnormalHosts.find(dataItem.host) == abnormalHosts.end());
        abnormalHosts[dataItem.host] = dataItem.value;
        return false;
    }
    return true;
}

string ThresholdPolicyChecker::getAlarmMsgContent(time_t curTime)
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

string ThresholdPolicyChecker::getAlarmMsgPrefix() const
{
    assert(NULL != _policyItem);
    char buffer[1024];
    snprintf(buffer, sizeof(buffer), "[id:%u][%s][%s][%.2lf,%.2lf]", _policyItem->getId(), 
             POLICY_TYPE_THRESHOLD.c_str(), _policyItem->getCheckTypeStr().c_str(),
             _policyItem->getDownBound(), _policyItem->getUpBound());
    return buffer;
}


RA_END_NAMESPACE(config);

