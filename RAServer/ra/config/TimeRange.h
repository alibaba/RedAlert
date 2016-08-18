#ifndef RA_TIMERANGE_H
#define RA_TIMERANGE_H

#include <ra/common/CommonDefine.h>
#include <ra/common/Log.h>
#include <ra/common/Json.h>

RA_BEGIN_NAMESPACE(config);

class TimeRange
{
public:
    TimeRange();
    ~TimeRange();
public:
    bool fromJson(const common::JsonObjectPtr& json);
    bool isInRange(time_t t) const;

private:
    bool formatTime(const std::string& timeStr, int& hour, int& min);
    
private:
    friend class TimeRangeTest;
    friend class PolicyConfigItemBaseTest;

private:
    int _beginHour;
    int _beginMin;
    int _endHour;
    int _endMin;
private:
    RA_LOG_DECLARE();
};

RA_TYPEDEF_PTR(TimeRange);

RA_END_NAMESPACE(config);

#endif //RA_TIMERANGE_H
