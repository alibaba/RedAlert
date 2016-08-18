#ifndef RA_FAKEPOLICYCHECKER_H
#define RA_FAKEPOLICYCHECKER_H

#include <ra/common/Common.h>
#include <ra/common/Log.h>
#include <ra/alarm/AlarmMsg.h>
#include <ra/tree/ProcessPackage.h>
#include <ra/process/PolicyChecker.h>
#include <ra/process/NormalizedMetricData.h>

RA_BEGIN_NAMESPACE(test_util);

class FakePolicyChecker: public process::PolicyChecker
{
public:
    FakePolicyChecker();
    ~FakePolicyChecker();
private:
    FakePolicyChecker(const FakePolicyChecker &);
    FakePolicyChecker& operator=(const FakePolicyChecker &);

public:
    bool init(const tree::ProcessPackagePtr& processPackage);
    std::string getAlarmMsgPrefix() const { return ""; }
    void checkSingleHost(const std::vector<process::NormalizedMetricDataPtr> &metricDataVec);
    bool checkHostVal(const std::string& metric, const process::MetricDataItem& hostData,
                      const std::vector<process::NormalizedMetricDataPtr> &metricDataVec); 

    void checkHostTotal(std::vector<process::NormalizedMetricDataPtr> &metricDataVec);
    bool checkHostTotalVal(const std::string& metric, const process::MetricDataItem& hostTotalData,
                           const std::vector<process::NormalizedMetricDataPtr> &metricDataVec);
    
    void checkMetricTotal(std::vector<process::NormalizedMetricDataPtr> &metricDataVec);
    bool checkMetricTotalVal(const process::MetricDataItem& metricTotalData,
                             const std::vector<process::NormalizedMetricDataPtr> &metricDataVec);
    
public:
    int32_t checkSingleHostCount;
    std::vector<process::MetricDataItem> checkHostValueVec;
    
    int32_t checkHostTotalCount;
    std::vector<process::MetricDataItem> checkHostTotaltValueVec;

    int32_t checkMetricTotalCount;
    std::vector<process::MetricDataItem> checkMetricTotaltValueVec;

    int32_t totalCheckCount;

private:
    RA_LOG_DECLARE();
};

RA_TYPEDEF_PTR(FakePolicyChecker);

RA_END_NAMESPACE(test_util);

#endif //RA_FAKEPOLICYCHECKER_H
