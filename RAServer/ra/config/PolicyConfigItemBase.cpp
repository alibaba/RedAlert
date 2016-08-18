#include <ra/util/Util.h>
#include <ra/config/PolicyConfigItemBase.h>

RA_BEGIN_NAMESPACE(config);
RA_LOG_SETUP(config, PolicyConfigItemBase);

using namespace std;
RA_USE_NAMESPACE(util);

bool PolicyFilterItem::fromJson(const common::JsonObjectPtr& json) {
    string hostVecStr;
    json->get("metric", metric, string("*"));
    json->get("host", hostVecStr, string("*"));
    const vector<string>& hostVec = Util::splitString(hostVecStr, ",");
    hostSet.insert(hostVec.begin(), hostVec.end());
    if (hostSet.find("*") != hostSet.end()) {
        hostSet.clear();
        hostSet.insert("*");
    }
    return true;
}

bool PolicyFilterItem::isMatched(const string& metric, uint32_t host) const
{
    if (!Util::isPatternMatch(this->metric, metric)) {
        return false;
    }
    std::string hostIp;
    Util::IPToString(host, hostIp);
    StringSet::const_iterator it = hostSet.begin();
    for (; it != hostSet.end(); ++it) {
        if (Util::isPatternMatch(*it, hostIp)){
            return true;
        }
    }
    return false;
}

bool PolicyFilterItem::isMatched(const string& metric) const
{
    if (Util::isPatternMatch(this->metric, metric) &&
        hostSet.find("*") != hostSet.end())
    {
        return true;
    }
    return false;
}


PolicyConfigItemBase::PolicyConfigItemBase():
    _fetchInterval(0),
    _minAlarmInterval(0),
    _trigger(PT_NONE),
    _id(0),
    _validTime(0),
    _checkType(CT_SINGLE)
{
}

PolicyConfigItemBase::~PolicyConfigItemBase()
{
}

bool PolicyConfigItemBase::fromJson(const common::JsonObjectPtr& json) {
    string checkType;
    json->get("alarmLevel", _alarmLevel, string("warning"));
    json->get("alarmGroup", _alarmGroup, string("ra"));
    json->get("checkType", checkType, CHECK_TYPE_SINGLE);
    json->get("fetchInterval_s", _fetchInterval, 15);
    json->get("minAlarmInterval_s", _minAlarmInterval, 60);

    if (checkType == CHECK_TYPE_SINGLE) {
        _checkType = CT_SINGLE;
    } else if (checkType == CHECK_TYPE_METRIC_TOTAL) {
        _checkType = CT_METRIC_TOTAL;
    } else if (checkType == CHECK_TYPE_HOST_TOTAL) {
        _checkType = CT_HOST_TOTAL;
    } else {
        _checkType = CT_NONE;
    }

    json->get("effectiveTime", _timeRanges);
    json->get("policyFilter", _policyFilterVec);

    if (_fetchInterval <= 0) {
        RA_LOG(ERROR, "fetch interval[%d] is negative, illegal, set to be default 15",
               _fetchInterval);
        _fetchInterval = 15;
    }
    RetrieveTimeRange retrieveTimeRange;
    retrieveTimeRange.start =  - _fetchInterval;
    retrieveTimeRange.end = 0;
    _retrieveTimeRangeVec.push_back(retrieveTimeRange);
    return true;
}

string PolicyConfigItemBase::getCheckTypeStr() const
{
    switch(_checkType) {
    case CT_SINGLE: 
        return CHECK_TYPE_SINGLE;
    case CT_METRIC_TOTAL: 
        return CHECK_TYPE_METRIC_TOTAL;
    case CT_HOST_TOTAL: 
        return CHECK_TYPE_HOST_TOTAL;
    default:
        return "unkown";
    }
}

bool PolicyConfigItemBase::isEffective(time_t curTime) const
{
    if (0 == _timeRanges.size()) {
        return true;
    }
    
    for (TimeRangeArray::const_iterator it = _timeRanges.begin();
         it != _timeRanges.end(); ++it) {
        if (it->isInRange(curTime)) {
            return true;
        }
    }
    
    return false;
}

bool PolicyConfigItemBase::isFiltered(const string& metric, uint32_t host) const
{
    vector<PolicyFilterItem>::const_iterator it = _policyFilterVec.begin();
    for (; it != _policyFilterVec.end(); ++it) {
        const PolicyFilterItem& policyFilterItem = *it;
        if (policyFilterItem.isMatched(metric, host)) {
            return true;
        }
    }
    return false;
}

bool PolicyConfigItemBase::isFiltered(const string& metric) const
{
    vector<PolicyFilterItem>::const_iterator it = _policyFilterVec.begin();
    for (; it != _policyFilterVec.end(); ++it) {
        const PolicyFilterItem& policyFilterItem = *it;
        if (policyFilterItem.isMatched(metric)) {
            return true;
        }
    }
    return false;
}

RA_END_NAMESPACE(ra);

