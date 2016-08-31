#include <ra/config/TimeRange.h>
#include <time.h>
#include <glog/logging.h>

using namespace std;

RA_BEGIN_NAMESPACE(config);

TimeRange::TimeRange():
        _beginHour(0),
        _beginMin(0),
        _endHour(23),
        _endMin(59)
{
}

TimeRange::~TimeRange()
{ 
}

bool TimeRange::fromJson(const common::JsonObjectPtr& json) {
    string begin, end;
    json->get("begin", begin, string("00:00"));
    json->get("end", end, string("23:59"));

    int begHour, endHour, begMin, endMin;
    if (!formatTime(begin, begHour, begMin)
        || !formatTime(end, endHour, endMin)) {
        LOG(ERROR) << "invalid time range format, begin(" 
		   << begin << "), end(" << end << ")";
        return true;
    }

    if (begMin + begHour*60 > endMin + endHour*60) {
            LOG(ERROR) << "begintime(" << begin << ") should less than endtime("
                       << end << "), we give it fullday instead";
        return true;
    }

    _beginHour = begHour;
    _beginMin = begMin;
    _endHour = endHour;
    _endMin = endMin;
    return true;
}

bool TimeRange::isInRange(time_t time) const
{
    tm t;
    memset(&t, 0, sizeof(t));
    if (NULL == localtime_r(&time, &t)) {
        LOG(ERROR) << "invalid time_t" << time;
        return false;
    }

    int cmpMin = t.tm_hour*60+t.tm_min;
    int begMin = _beginHour*60+_beginMin;
    int endMin = _endHour*60+_endMin;
    return cmpMin <= endMin && cmpMin >= begMin;
}

bool TimeRange::formatTime(const std::string& timeStr, int& hour, int& min)
{
    tm t;
    memset(&t, 0, sizeof(t));
    char* res = strptime(timeStr.c_str(), "%H:%M", &t);
    if (NULL == res) {
        return false;
    }

    if (*res != '\0') {
        // some string remain unprocessed
        return false;
    }
    
    hour = t.tm_hour;
    min = t.tm_min;
    return true;
}

RA_END_NAMESPACE(config);

