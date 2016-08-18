#ifndef RA_METRICDATA_H
#define RA_METRICDATA_H

#include <stdint.h>
#include <vector>
#include <limits>
#include <tr1/memory>
#include <ra/common/Common.h>
#include <ra/common/Log.h>

RA_BEGIN_NAMESPACE(fetcher);

class MetricData
{
public:
    typedef std::vector<uint64_t> TimeVec;
    typedef std::vector<double> ValueVec;
    RA_TYPEDEF_PTR(TimeVec);
    RA_TYPEDEF_PTR(ValueVec);

public:
    MetricData(std::string hostname_ = "UNSPECIFIED") : hostname(hostname_), times(new TimeVec()), values(new ValueVec()) { }

    MetricData(std::string hostname_, TimeVecPtr times_, ValueVecPtr values_)
        : hostname(hostname_), times(times_), values(values_) { }

    uint64_t startTime() const {
        uint64_t start = 0;
        if (!times->empty()) {
            start = *times->begin();
        }
        return start;
    }

    uint64_t endTime() const {
        uint64_t end = 0;
        if (!times->empty()) {
            end = *times->rbegin();
        }
        return end;
    }

    double avgValue() const {
        double sum = 0.0;
        for (ValueVec::const_iterator iter = values->begin(); iter != values->end(); iter++) {
            sum += *iter;
        }
        return sum / values->size();
    }

    double maxValue() const {
        double maxVal = -std::numeric_limits<double>::infinity();
        for (ValueVec::const_iterator iter = values->begin(); iter != values->end(); iter++) {
            maxVal = std::max(maxVal, *iter);
        }
        return maxVal;
    }

    double minValue() const {
        double minVal = std::numeric_limits<double>::infinity();
        for (ValueVec::const_iterator iter = values->begin(); iter != values->end(); iter++) {
            minVal = std::min(minVal, *iter);
        }
        return minVal;
    }

public:
    std::string hostname;
    TimeVecPtr times;
    ValueVecPtr values;
};

RA_TYPEDEF_PTR(MetricData);

RA_END_NAMESPACE(fetcher);

#endif /* RA_METRICDATA_H */

