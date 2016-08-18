#ifndef RA_CYCLEPOLICYCHECKER_H
#define RA_CYCLEPOLICYCHECKER_H

#include <sstream>
#include <ra/common/Common.h>
#include <ra/common/Log.h>
#include <ra/util/Util.h>
#include <ra/process/PolicyCheckerTyped.h>

RA_BEGIN_NAMESPACE(process);

class CyclePolicyChecker: public PolicyCheckerTyped<config::CyclePolicyItem>
{
public:
    CyclePolicyChecker();
    ~CyclePolicyChecker();
private:
    CyclePolicyChecker(const CyclePolicyChecker &);
    CyclePolicyChecker& operator=(const CyclePolicyChecker &);

protected:
    std::string getAlarmMsgContent(time_t curTime);
    std::string getAlarmMsgPrefix() const;
    bool checkHostVal(const std::string& metric,
                      const MetricDataItem& hostData,
                      const std::vector<NormalizedMetricDataPtr> &metricDataVec);

    bool checkHostTotalVal(const std::string& metric,
                           const MetricDataItem& hostTotalData,
                           const std::vector<NormalizedMetricDataPtr>& metricDataVec);
    
    bool checkMetricTotalVal(const MetricDataItem& metricTotalData,
                             const std::vector<NormalizedMetricDataPtr>& metricDataVec);

private:
    bool checkHost(const std::string& metric,
                   const MetricDataItem& data,
                   const std::vector<NormalizedMetricDataPtr>& metricDataVec,
                   bool isTotal);

    bool checkValue(const std::string& metric,
                    const MetricDataItem& dataItem, 
                    const std::vector<double>& historyValVec);

private:
    struct AbnormalVal
    {
        double value;
        std::vector<double> historyValVec;
        std::string toString() const {
            char buffer[64];
            snprintf(buffer, sizeof(buffer), "[%.2lf,History:", value);
            std::vector<std::string> historyValStr;
            for (size_t i = 0; i < historyValVec.size(); i++) {
                std::stringstream ss;
                ss << historyValVec[i];
                historyValStr.push_back(ss.str());
            }
            return buffer + util::Util::joinString(historyValStr, ",") + "]";
        }
    };

private:
    friend class CyclePolicyCheckerTest;

private:
    std::map<std::string, std::map<uint32_t, AbnormalVal> > _abnormalInfo;

private:
    RA_LOG_DECLARE();
};

RA_TYPEDEF_PTR(CyclePolicyChecker);

RA_END_NAMESPACE(process);

#endif //RA_CYCLEPOLICYCHECKER_H
