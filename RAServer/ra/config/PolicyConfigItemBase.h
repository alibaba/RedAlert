#ifndef RA_POLICYCONFIGITEMBASE_H
#define RA_POLICYCONFIGITEMBASE_H

#include <ra/common/CommonDefine.h>
#include <ra/common/Log.h>
#include <ra/common/Json.h>
#include <ra/config/TimeRange.h>

RA_BEGIN_NAMESPACE(tree);
class TreeManagerTest;
RA_END_NAMESPACE(tree);

RA_BEGIN_NAMESPACE(test_util);
class TestUtil;
RA_END_NAMESPACE(test_util);

RA_BEGIN_NAMESPACE(process);
class PolicyCheckerTest;
RA_END_NAMESPACE(process);

RA_BEGIN_NAMESPACE(config);

struct PolicyFilterItem
{
    std::string metric;
    StringSet hostSet;

    bool fromJson(const common::JsonObjectPtr& json);
    bool isMatched(const std::string& metric, uint32_t host) const;
    bool isMatched(const std::string& metric) const;
};

struct RetrieveTimeRange
{
    int32_t start;
    int32_t end;
};

enum PolicyTrigger {
    PT_NONE = 0,
    PT_THRESHOLD,
    PT_TREND,
    PT_AVAILABILITY,
    PT_SINGULARITY,
    PT_CYCLE,
};

enum CheckType {
    CT_NONE = 0,
    CT_SINGLE,
    CT_METRIC_TOTAL,
    CT_HOST_TOTAL,
};

class PolicyConfigItemBase
{
public:
    typedef std::vector<TimeRange> TimeRangeArray;
public:
    PolicyConfigItemBase();
    virtual ~PolicyConfigItemBase();

public:
    bool isFiltered(const std::string& metric, uint32_t host) const;
    bool isFiltered(const std::string& metric) const;
    void setValidTime(time_t time) {
        _validTime = time;
    }
    time_t getValidTime() const {
        return _validTime;
    }
    void getRetrieveTimeRange(std::vector<RetrieveTimeRange>& retrieveTimeRangeVec) const {
        retrieveTimeRangeVec.assign(_retrieveTimeRangeVec.begin(), _retrieveTimeRangeVec.end());
    }

    virtual bool fromJson(const common::JsonObjectPtr& json);

    const std::string& getMetric() const {
        return _metric;
    }
    const std::string& getGroup() const {
        return _group;
    }
    const std::string& getAlarmLevel() const {
        return _alarmLevel;
    }
    const std::string& getAlarmGroup() const {
        return _alarmGroup;
    }
    CheckType getCheckType() const{
        return _checkType;
    }
    std::string getCheckTypeStr() const;
    int32_t getFetchInterval() const {
        return _fetchInterval;
    }
    int32_t getMinAlarmInterval() const {
        return _minAlarmInterval;
    }
    PolicyTrigger getTriggerType() const {
        return _trigger;
    }
    uint32_t getId() const
    {
        return _id;
    }

    void setMinAlarmInterval(int interval) {
        _minAlarmInterval = interval;
    }
    void setMetric(const std::string& metric) {
        _metric = metric;
    }
    void setGroup(const std::string& group) {
        _group = group;
    }
    void setAlarmGroup(const std::string& alarmGroup) {
        _alarmGroup = alarmGroup;
    }
    void setCheckType(CheckType checkType) {
        _checkType = checkType;
    }
    void setFetchInterval(int interval) {
        _fetchInterval = interval;
    }
    void setTriggerType(PolicyTrigger t) {
        _trigger = t;
    }
    void setAlarmLevel(const std::string& level) {
        _alarmLevel = level;
    }
    void setId(uint32_t id)
    {
        _id = id;
    }

    bool isEffective(time_t curTime) const;

private:
    std::string _group;
    std::string _metric;
    std::string _alarmLevel;
    std::string _alarmGroup;
    int32_t _fetchInterval;
    int32_t _minAlarmInterval;
    PolicyTrigger _trigger;
    uint32_t _id;
    TimeRangeArray _timeRanges;

    time_t _validTime;
    std::vector<PolicyFilterItem> _policyFilterVec;

protected:
    CheckType _checkType;
    //unit:second
    std::vector<RetrieveTimeRange> _retrieveTimeRangeVec; 

private:
    friend class PolicyConfigItemBaseTest;
    friend class tree::TreeManagerTest;
    friend class test_util::TestUtil;
    friend class process::PolicyCheckerTest;
};

RA_TYPEDEF_PTR(PolicyConfigItemBase);

RA_END_NAMESPACE(ra);

#endif //RA_POLICYCONFIGITEMBASE_H
