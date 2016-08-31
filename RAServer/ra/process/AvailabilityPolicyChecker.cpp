#include <ra/process/AvailabilityPolicyChecker.h>

using namespace std;
RA_USE_NAMESPACE(tree);
RA_USE_NAMESPACE(alarm);
RA_USE_NAMESPACE(config);

RA_BEGIN_NAMESPACE(process);

AvailabilityPolicyChecker::AvailabilityPolicyChecker() 
    : _hostCount(-1)
{ 
}

AvailabilityPolicyChecker::~AvailabilityPolicyChecker() { 
}

void AvailabilityPolicyChecker::checkSingleHost(
    const vector<NormalizedMetricDataPtr> &metricDataVec)
{
    assert(metricDataVec.size() > 0);
    const NormalizedMetricDataPtr& currMetricData = metricDataVec[0];
    assert(NULL != currMetricData);

    const RequestPackagePtr& requestPackage = _processPackage->getRequestPackage();
    const StringSet& metricSet = requestPackage->getMetricSet();
    if (metricSet.empty()) {
        _hasNoMetric = true;
        return;
    }
    int32_t hostCount = currMetricData->getHostCount();
    assert(NULL != _policyItem);
    if (hostCount < _policyItem->getMinHostNum()) {
        _hostCount = hostCount;
    }
}

string AvailabilityPolicyChecker::getAlarmMsgContent(time_t curTime)
{
    string ret;
    if (_hostCount >= 0) {
        const std::string& group = _policyItem->getGroup();
        const std::string& metric = _policyItem->getMetric();
        if (!_configWrapper->isShielded(curTime, group, metric, 0)) {
            char buffer[1024];
            snprintf(buffer, sizeof(buffer), "%s:[%d]", metric.c_str(), _hostCount);
            ret = buffer;
        }
    }
    const string& baseAlarmContent = PolicyChecker::getAlarmMsgContent(curTime);
    if (!ret.empty() && !baseAlarmContent.empty()) {
        ret.append(";" + baseAlarmContent);
    }
    else {
        ret.append(baseAlarmContent);
    }
    return ret;
}

string AvailabilityPolicyChecker::getAlarmMsgPrefix() const
{
    assert(NULL != _policyItem);
    char buffer[1024];
    snprintf(buffer, sizeof(buffer), "[id:%u][%s][MinHostCount:%d]", _policyItem->getId(), 
             POLICY_TYPE_AVAILABILITY.c_str(), _policyItem->getMinHostNum());
    return buffer;

}

RA_END_NAMESPACE(process);

