#ifndef RA_NORMALIZEDMETRICDATA_H
#define RA_NORMALIZEDMETRICDATA_H

#include <stdint.h>
#include <vector>
#include <ra/common/CommonDefine.h>
#include <ra/common/Log.h>
#include <ra/fetcher/MetricData.h>

RA_BEGIN_NAMESPACE(process);

//typedef ::google::protobuf::RepeatedField< ::google::protobuf::uint64 > TimeList;
//typedef ::google::protobuf::RepeatedField< double > ValueList;
typedef std::vector<uint64_t> TimeList;
typedef std::vector<double> ValueList;
RA_TYPEDEF_PTR(TimeList);
RA_TYPEDEF_PTR(ValueList);

struct MetricDataItem
{
    uint32_t host;
    uint64_t time;
    double value;
    fetcher::MetricData::ValueVecPtr rawValue;
    fetcher::MetricData::TimeVecPtr rawTime;

    MetricDataItem() { }

    bool operator < (const MetricDataItem& dataItem) const {
        return host < dataItem.host;
    }
    bool operator == (const MetricDataItem& dataItem) const {
        return host == dataItem.host
            && time == dataItem.time
            && value == dataItem.value;
    }
};

RA_TYPEDEF_PTR(MetricDataItem);

class NormalizedMetricData
{
public:
    NormalizedMetricData();
    ~NormalizedMetricData();

private:
    NormalizedMetricData(const NormalizedMetricData &);
    NormalizedMetricData& operator=(const NormalizedMetricData &);

public:
    static uint64_t getTime(const TimeList& timeList);
    static double getValue(const ValueList& valueList);
    
    void genHostTotalData();
    void genMetricTotalVal();

    bool hasData(const std::string& metric) const {
        return _data.find(metric) != _data.end();
    }
    double getAvgValue() const;
    size_t getHostCount() const {
        return _hostCount;
    }
    bool addMetricData(const std::string& metric, const MetricDataItem& item);
    bool getMetricData(const std::string& metric, uint32_t host, MetricDataItem& out) const;
    const std::set<MetricDataItem>& getMetricData(const std::string& metric) const;
    bool getHostTotalData(const std::string& metric, MetricDataItem& out) const;
    bool getMetricTotalData(MetricDataItem& out) const;

    size_t getMetricCount() const {
        return _data.size();
    }
    bool empty() const {
        return _data.empty();
    }

private:
    void getTotalValue(const std::vector<MetricDataItem> &metricDataVec,
                       double& totalValue, uint64_t& avgTime);

    void genSampleTimeList(uint64_t minTime, uint64_t maxTime, 
                           int32_t pointCount, TimeList& sampleTimeList) const;

    void genNormalizedTotalValue(const std::vector<MetricDataItem> &metricDataVec, 
            const TimeList& sampleTimeList, ValueList& totalValueList) const;

    void sampleValues(const MetricDataItem& dataItem, const TimeList& sampleTimeList, 
                      ValueList& avgValueList) const;

private:
    friend class NormalizedMetricDataTest;

private:
    std::map<std::string, std::set<MetricDataItem> > _data;
    std::map<std::string, MetricDataItem> _hostTotalData;
    MetricDataItemPtr _metricTotalData;
    double _sumValue;
    size_t _hostCount;
};

RA_TYPEDEF_PTR(NormalizedMetricData);

RA_END_NAMESPACE(process);

#endif //RA_NORMALIZEDMETRICDATA_H
