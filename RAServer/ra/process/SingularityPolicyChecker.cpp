#include <ra/process/SingularityPolicyChecker.h>

using namespace std;
RA_USE_NAMESPACE(tree);
RA_USE_NAMESPACE(alarm);
RA_USE_NAMESPACE(config);

RA_BEGIN_NAMESPACE(process);

SingularityPolicyChecker::SingularityPolicyChecker() 
    : _avgValue(INVALID_METRIC_DATA)
{ 
}

SingularityPolicyChecker::~SingularityPolicyChecker() { 
}

bool SingularityPolicyChecker::checkHostVal(const string& metric, const MetricDataItem& hostData,
        const vector<NormalizedMetricDataPtr> &metricDataVec)
{
    assert(!metricDataVec.empty());
    const NormalizedMetricDataPtr& curMetricData = metricDataVec[0];
    assert(NULL != curMetricData);
    
    double avgValue = curMetricData->getAvgValue();
    assert(INVALID_METRIC_DATA != avgValue);

    if (INVALID_METRIC_DATA != _avgValue) {
        assert(avgValue == _avgValue);
    }
    else {
        _avgValue = avgValue;
    }
    
    assert(NULL != _policyItem);
    if (_policyItem->isDiffTooMuch(hostData.value, avgValue)) {
        std::map<uint32_t, double>& abnormalHosts = _abnormalInfo[metric];
        assert(abnormalHosts.find(hostData.host) == abnormalHosts.end());
        abnormalHosts[hostData.host] = hostData.value;
        return false;
    }
    return true;
}

string SingularityPolicyChecker::getAlarmMsgContent(time_t curTime)
{
    removeShieldAbnormalInfo(curTime, _abnormalInfo);
    string ret;
    genAbnormalStr(_abnormalInfo, ret);
    const string& baseAlarmContent = PolicyChecker::getAlarmMsgContent(curTime);
    if (!ret.empty()) {
        char buffer[128];
        assert(INVALID_METRIC_DATA != _avgValue);
        snprintf(buffer, sizeof(buffer), "avg:[%.2lf]", _avgValue);
        ret = buffer + ret;
        if (!baseAlarmContent.empty()) {
            ret.append(";");
        }
    }
    ret.append(baseAlarmContent);
    return ret;    
}

string SingularityPolicyChecker::getAlarmMsgPrefix() const
{
    assert(NULL != _policyItem);
    char buffer[1024];
    snprintf(buffer, sizeof(buffer), "[id:%u][%s]", _policyItem->getId(), 
             POLICY_TYPE_SINGULARITY.c_str());
    return buffer;
}


RA_END_NAMESPACE(process);

