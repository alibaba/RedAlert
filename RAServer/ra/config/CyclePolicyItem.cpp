#include <ra/config/CyclePolicyItem.h>
#include <ra/util/Util.h>
#include <algorithm>

using namespace std;

RA_BEGIN_NAMESPACE(config);
RA_LOG_SETUP(config, CyclePolicyItem);

CyclePolicyItem::CyclePolicyItem():
        _cycleTime(86400)
{
    setTriggerType(PT_CYCLE);
}

CyclePolicyItem::~CyclePolicyItem()
{
}

bool CyclePolicyItem::isDiffTooMuch(double current, const vector<double>& historyValVec) const
{
    size_t size = historyValVec.size();
    if(size == 0){
        return false;
    }

    size_t diffTooMuchCount = 0;
    vector<double>::const_iterator it = historyValVec.begin();
    for(; it != historyValVec.end(); ++it) {
        if(DeviationPolicyItem::isDiffTooMuch(current, *it)){
            ++diffTooMuchCount;
        }
    }
    return diffTooMuchCount > size / 2;
}

bool CyclePolicyItem::fromJson(const common::JsonObjectPtr& json) {
    DeviationPolicyItem::fromJson(json);
    json->get("cycleTime_s", _cycleTime, 86400);
    json->get("cycleCount", _cycleCount, 3);

    if (_cycleTime <= 0) {
        LOG(ERROR) << "cycleTime[" << _cycleTime
		   << "] is negative, illegal, set to be default 86400";
        _cycleTime = 86400;
    }
    if (_cycleCount <= 0) {
        LOG(ERROR) << "cycleCount[" << _cycleCount 
		   << "] is negative, illegal, set to be default 3";
        _cycleCount = 3;
    }

    int32_t fetchInterval = getFetchInterval();
    assert(fetchInterval > 0);

    RetrieveTimeRange retrieveTimeRange;
    for (int32_t i = 1; i <= _cycleCount; ++i) {
        int64_t cycleTime =  - i * _cycleTime;
        retrieveTimeRange.start =  cycleTime - fetchInterval;
        retrieveTimeRange.end = cycleTime;
        _retrieveTimeRangeVec.push_back(retrieveTimeRange);
    }
    return true;
}

RA_END_NAMESPACE(ra);

