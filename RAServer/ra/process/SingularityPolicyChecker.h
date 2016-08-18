#ifndef RA_SINGULARITYPOLICYCHECKER_H
#define RA_SINGULARITYPOLICYCHECKER_H

#include <ra/common/Common.h>
#include <ra/common/Log.h>
#include <ra/process/PolicyCheckerTyped.h>

RA_BEGIN_NAMESPACE(process);

class SingularityPolicyChecker: public PolicyCheckerTyped<config::SingularityPolicyItem>
{
public:
    SingularityPolicyChecker();
    ~SingularityPolicyChecker();
private:
    SingularityPolicyChecker(const SingularityPolicyChecker &);
    SingularityPolicyChecker& operator=(const SingularityPolicyChecker &);

protected:
    std::string getAlarmMsgContent(time_t curTime);
    std::string getAlarmMsgPrefix() const;
    bool checkHostVal(const std::string& metric, const MetricDataItem& hostData,
                      const std::vector<NormalizedMetricDataPtr> &metricDataVec);

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
    friend class SingularityPolicyCheckerTest;

private:
    std::map<std::string, std::map<uint32_t, double> > _abnormalInfo;
    double _avgValue;

private:
    RA_LOG_DECLARE();
};

RA_TYPEDEF_PTR(SingularityPolicyChecker);

RA_END_NAMESPACE(process);

#endif //RA_SINGULARITYPOLICYCHECKER_H
