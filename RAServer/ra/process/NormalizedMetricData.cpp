#include <ra/process/NormalizedMetricData.h>
using namespace std;

RA_BEGIN_NAMESPACE(process);
RA_LOG_SETUP(process, NormalizedMetricData);

NormalizedMetricData::NormalizedMetricData() 
    : _sumValue(0.0)
    , _hostCount(0)
{
}

NormalizedMetricData::~NormalizedMetricData() { 
}

bool NormalizedMetricData::addMetricData(const string& metric, const MetricDataItem& item)
{
    if (item.value == INVALID_METRIC_DATA) {
        return false;
    }
    set<MetricDataItem>& metricDataItemSet = _data[metric];
    if (metricDataItemSet.find(item) != metricDataItemSet.end()) {
        return false;
    }
    metricDataItemSet.insert(item);
    _sumValue += item.value;
    ++_hostCount;
    return true;
}

double NormalizedMetricData::getAvgValue() const
{
    if (0 == _hostCount) {
        return INVALID_METRIC_DATA;
    }
    return _sumValue / _hostCount;
}

const set<MetricDataItem>& NormalizedMetricData::getMetricData(const string& metric) const
{
    static set<MetricDataItem> empty;
    map<string, set<MetricDataItem> >::const_iterator it = _data.find(metric);
    if (it != _data.end()) {
        return it->second;
    }
    return empty;
}

bool NormalizedMetricData::getMetricData(const string& metric, 
        uint32_t host, MetricDataItem& out) const
{
    map<string, set<MetricDataItem> >::const_iterator it = _data.find(metric);
    if (it != _data.end()) {
        const set<MetricDataItem>& dataSet = it->second;
        out.host = host;
        set<MetricDataItem>::const_iterator dataIter = dataSet.find(out);
        if (dataIter != dataSet.end()) {
            out = *dataIter;
            return true;
        }
    }
    return false;
}

bool NormalizedMetricData::getHostTotalData(const std::string& metric,
        MetricDataItem& out) const
{
    map<string, MetricDataItem>::const_iterator it = _hostTotalData.find(metric);
    if (it == _hostTotalData.end()) {
        return false;
    }
    out = it->second;
    return true;
}

bool NormalizedMetricData::getMetricTotalData(MetricDataItem& out) const
{
    if (NULL == _metricTotalData) {
        return false;
    }
    out = *_metricTotalData;
    return true;
}

void NormalizedMetricData::genHostTotalData() 
{
    _hostTotalData.clear();
    map<string, set<MetricDataItem> >::const_iterator metricIter = _data.begin();
    for (; metricIter != _data.end(); ++metricIter) {
        const string& metric = metricIter->first;
        const set<MetricDataItem>& dataSet = metricIter->second;
        assert(!dataSet.empty());
        vector<MetricDataItem> metricDataVec(dataSet.begin(), dataSet.end());

        double totalValue = 0.0;
        uint64_t avgTime = 0;
        getTotalValue(metricDataVec, totalValue, avgTime);
        assert(INVALID_METRIC_DATA != totalValue);

        MetricDataItem &dataItem = _hostTotalData[metric];
        dataItem.time = avgTime;
        dataItem.value = totalValue;
        dataItem.host = 0;
    }
}

void NormalizedMetricData::genMetricTotalVal() 
{
    if (empty()) {
        return;
    }
    vector<MetricDataItem> metricDataVec;
    metricDataVec.reserve(getHostCount());
    
    map<string, set<MetricDataItem> >::const_iterator metricIter = _data.begin();
    for (; metricIter != _data.end(); ++metricIter) {
        const set<MetricDataItem>& dataSet = metricIter->second;
        assert(!dataSet.empty());
        metricDataVec.insert(metricDataVec.end(), dataSet.begin(), dataSet.end());
    }
    double totalValue = 0.0;
    uint64_t avgTime = 0;
    getTotalValue(metricDataVec, totalValue, avgTime);
    assert(INVALID_METRIC_DATA != totalValue);

    _metricTotalData.reset(new MetricDataItem);
    _metricTotalData->time = avgTime;
    _metricTotalData->value = totalValue;
    _metricTotalData->host = 0;
}


uint64_t NormalizedMetricData::getTime(const TimeList& timeList)
{
    uint64_t time = 0;
    int32_t timeSize = timeList.size();
    if (0 == timeSize) {
        return time;
    }
    for (int32_t i = 0; i < timeSize; ++i) {
        time += timeList[i];
    }
    return time / timeSize;
}

double NormalizedMetricData::getValue(const ValueList& valueList)
{
    double totalValue = 0;
    int32_t validValueCount = 0;
    for (size_t i = 0; i < valueList.size(); ++i) {
        if (valueList[i] != INVALID_METRIC_DATA) {
            totalValue += valueList[i];
            ++validValueCount;
        }
    }
    if (0 == validValueCount) {
        return INVALID_METRIC_DATA;
    }
    return totalValue / validValueCount;
}

void NormalizedMetricData::getTotalValue(const vector<MetricDataItem> &metricDataVec,
        double& totalValue, uint64_t& avgTime)
{
    uint64_t minTime = std::numeric_limits<uint64_t>::max();
    uint64_t maxTime = 0;
    size_t maxPointCount = 0;
    assert(!metricDataVec.empty());
    vector<MetricDataItem>::const_iterator it = metricDataVec.begin();
    for (; it != metricDataVec.end(); ++it) {
        const MetricDataItem& item = *it;
        assert(NULL != item.rawValue);
        assert(NULL != item.rawTime);
        size_t pointCount = item.rawValue->size();
        assert(pointCount > 0);
        assert(item.rawTime->size() == pointCount);
        uint64_t first = item.rawTime->at(0);
        uint64_t last = item.rawTime->at(pointCount - 1);
        minTime = min(minTime, first);
        maxTime = max(maxTime, last);
        if (pointCount > maxPointCount) {
            maxPointCount = pointCount;
        }
    }
    assert(minTime <= maxTime);
    assert(maxPointCount >= 1);
    if ((uint64_t)maxPointCount > (maxTime - minTime) / MICROSECONDS) {
        maxPointCount = (maxTime - minTime) / MICROSECONDS;
        if (0 == maxPointCount) {
            maxPointCount = 1;
        }
    }
    TimeList sampleTimeList;
    genSampleTimeList(minTime, maxTime, maxPointCount, sampleTimeList);
    ValueList totalValueList;
    genNormalizedTotalValue(metricDataVec, sampleTimeList, totalValueList);
    avgTime = getTime(sampleTimeList);
    totalValue = getValue(totalValueList);
}

void NormalizedMetricData::genSampleTimeList(uint64_t minTime, uint64_t maxTime, 
        int32_t pointCount, TimeList& sampleTimeList) const
{
    sampleTimeList.clear();
    assert(minTime <= maxTime);
    assert(pointCount >= 1);
    uint64_t interval = (maxTime - minTime) / pointCount;
    sampleTimeList.reserve(pointCount);
    for (int32_t i = 1; i <= pointCount; ++i) {
        uint64_t time = minTime + i * interval;
        sampleTimeList.push_back(time);
    }
    sampleTimeList[pointCount - 1] = maxTime;
}

void NormalizedMetricData::genNormalizedTotalValue(const vector<MetricDataItem> &metricDataVec, 
        const TimeList& sampleTimeList, ValueList& totalValueList) const
{
    totalValueList.clear();
    size_t samplePointCount = sampleTimeList.size();
    assert(samplePointCount > 0);
    totalValueList.reserve(samplePointCount);
    for (size_t i = 0; i < samplePointCount; ++i) {
        totalValueList.push_back(INVALID_METRIC_DATA);
    }

    vector<MetricDataItem>::const_iterator it = metricDataVec.begin();
    for (; it != metricDataVec.end(); ++it) {
        const MetricDataItem& item = *it;
        ValueList avgValueList;
        sampleValues(item, sampleTimeList, avgValueList);
        assert(avgValueList.size() == samplePointCount);
        for (size_t i = 0; i < samplePointCount; ++i) {
            if (INVALID_METRIC_DATA == avgValueList[i]) {
                continue;
            }
            double totalValue = totalValueList[i];
            if (INVALID_METRIC_DATA == totalValue) {
                totalValue = 0.0;
            }
            totalValue += avgValueList[i];
            totalValueList[i] = totalValue;
        }
    }
}

void NormalizedMetricData::sampleValues(const MetricDataItem& dataItem, 
        const TimeList& sampleTimeList, ValueList& avgValueList) const
{
    avgValueList.clear();
    int32_t samplePointCount = sampleTimeList.size();
    assert(samplePointCount > 0);
    avgValueList.reserve(samplePointCount);
    assert(NULL != dataItem.rawValue);
    size_t pointCount = dataItem.rawValue->size();
    assert(pointCount > 0);
    assert(NULL != dataItem.rawTime);
    assert(dataItem.rawTime->size() == pointCount);

    size_t rawTimeIndex = 0;
    for (int32_t i = 0; i < samplePointCount; ++i) {
        uint64_t sampleTime = sampleTimeList[i];
        double totalValue = 0.0;
        int32_t sampleCount = 0;
        while (rawTimeIndex < pointCount && 
               dataItem.rawTime->at(rawTimeIndex) <= sampleTime)
        {
            double val = dataItem.rawValue->at(rawTimeIndex++);
            if (INVALID_METRIC_DATA == val) {
                continue;
            }
            totalValue += val;
            ++sampleCount;
        }
        if (sampleCount == 0) {
            avgValueList.push_back(INVALID_METRIC_DATA);
        }
        else {
            avgValueList.push_back(totalValue / sampleCount);
        }
    }
}

RA_END_NAMESPACE(process);
