#ifndef RA_AVAILABILITYPOLICYCHECKER_H
#define RA_AVAILABILITYPOLICYCHECKER_H

#include <ra/common/Common.h>
#include <ra/common/Log.h>
#include <ra/process/PolicyCheckerTyped.h>

RA_BEGIN_NAMESPACE(process);

class AvailabilityPolicyChecker: public PolicyCheckerTyped<config::AvailabilityPolicyItem>
{
public:
    AvailabilityPolicyChecker();
    ~AvailabilityPolicyChecker();
private:
    AvailabilityPolicyChecker(const AvailabilityPolicyChecker &);
    AvailabilityPolicyChecker& operator=(const AvailabilityPolicyChecker &);

protected:
    std::string getAlarmMsgPrefix() const;
    std::string getAlarmMsgContent(time_t curTime);

    void checkSingleHost(const std::vector<NormalizedMetricDataPtr> &metricDataVec);

    bool checkHostVal(const std::string& metric, const MetricDataItem& hostData,
                      const std::vector<NormalizedMetricDataPtr> &metricDataVec)
    {
        assert(false);
        return true;
    }
    bool checkHostTotalVal(const std::string& metric, const MetricDataItem& hostTotalData,
                           const std::vector<NormalizedMetricDataPtr> &metricDataVec) 
    {
        assert(false);
        return true;
    }
    bool checkMetricTotalVal(const MetricDataItem& metricTotalData,
                             const std::vector<NormalizedMetricDataPtr> &metricDataVec)
    {
        assert(false);
        return true;
    }

private:
    friend class AvailabilityPolicyCheckerTest;

private:
    int32_t _hostCount;

private:
    RA_LOG_DECLARE();
};

RA_TYPEDEF_PTR(AvailabilityPolicyChecker);

RA_END_NAMESPACE(process);

#endif //RA_AVAILABILITYPOLICYCHECKER_H
