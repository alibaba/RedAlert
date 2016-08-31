#ifndef RA_THRESHOLDPOLICYCHECKER_H
#define RA_THRESHOLDPOLICYCHECKER_H

#include <ra/common/Common.h>
#include <ra/common/Log.h>
#include <ra/process/PolicyCheckerTyped.h>

RA_BEGIN_NAMESPACE(process);

class ThresholdPolicyChecker: public PolicyCheckerTyped<config::ThresholdPolicyItem>
{
public:
    ThresholdPolicyChecker();
    ~ThresholdPolicyChecker();
private:
    ThresholdPolicyChecker(const ThresholdPolicyChecker &);
    ThresholdPolicyChecker& operator=(const ThresholdPolicyChecker &);

protected:
    std::string getAlarmMsgContent(time_t curTime);
    std::string getAlarmMsgPrefix() const;
    bool checkHostVal(const std::string& metric, const MetricDataItem& hostData,
                      const std::vector<NormalizedMetricDataPtr> &metricDataVec);

    bool checkHostTotalVal(const std::string& metric, const MetricDataItem& hostTotalData,
                           const std::vector<NormalizedMetricDataPtr> &metricDataVec);

    bool checkMetricTotalVal(const MetricDataItem& metricTotalData,
                             const std::vector<NormalizedMetricDataPtr> &metricDataVec);

private:
    bool checkValue(const std::string& metric, const MetricDataItem& dataItem);
    

private:
    friend class ThresholdPolicyCheckerTest;

private:
    std::map<std::string, std::map<uint32_t, double> > _abnormalInfo;
};

RA_TYPEDEF_PTR(ThresholdPolicyChecker);

RA_END_NAMESPACE(process);

#endif //RA_THRESHOLDPOLICYCHECKER_H
