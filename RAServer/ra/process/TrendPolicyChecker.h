#ifndef RA_TRENDPOLICYCHECKER_H
#define RA_TRENDPOLICYCHECKER_H

#include <ra/common/Common.h>
#include <ra/common/Log.h>
#include <ra/process/PolicyCheckerTyped.h>

RA_BEGIN_NAMESPACE(process);

class TrendPolicyChecker: public PolicyCheckerTyped<config::TrendPolicyItem>
{
public:
    TrendPolicyChecker();
    ~TrendPolicyChecker();

private:
    TrendPolicyChecker(const TrendPolicyChecker &);
    TrendPolicyChecker& operator=(const TrendPolicyChecker &);

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
    struct AbnormalVal
    {
        double value;
        double forecastValue;
        std::string toString() const {
            char buffer[128];
            snprintf(buffer, sizeof(buffer), "[%.2lf,Forecast:%.2lf]", 
                     value, forecastValue);
            return buffer;
        }
    };

private:
    friend class TrendPolicyCheckerTest;

private:
    std::map<std::string, std::map<uint32_t, AbnormalVal> > _abnormalInfo;
};

RA_TYPEDEF_PTR(TrendPolicyChecker);

RA_END_NAMESPACE(process);

#endif //RA_TRENDPOLICYCHECKER_H
