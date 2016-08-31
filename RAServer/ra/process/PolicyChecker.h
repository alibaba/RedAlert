#ifndef RA_POLICYCHECKER_H
#define RA_POLICYCHECKER_H

#include <ra/common/Common.h>
#include <ra/common/Log.h>
#include <ra/alarm/AlarmMsg.h>
#include <ra/tree/ProcessPackage.h>
#include <ra/process/NormalizedMetricData.h>
#include <ra/util/Util.h>

RA_BEGIN_NAMESPACE(process);

class PolicyChecker
{
public:
    PolicyChecker();
    virtual ~PolicyChecker();

private:
    PolicyChecker(const PolicyChecker &);
    PolicyChecker& operator=(const PolicyChecker &);

public:
    virtual bool init(const tree::ProcessPackagePtr& processPackage);
    alarm::AlarmMsgPtr check();

protected:
    virtual std::string getAlarmMsgContent(time_t curTime);
    virtual std::string getAlarmMsgPrefix() const = 0;

    virtual void checkSingleHost(const std::vector<NormalizedMetricDataPtr> &metricDataVec);
    virtual bool checkHostVal(const std::string& metric,
                              const MetricDataItem& hostData,
                              const std::vector<NormalizedMetricDataPtr> &metricDataVec) = 0;

    virtual void checkHostTotal(std::vector<NormalizedMetricDataPtr> &metricDataVec);
    virtual bool checkHostTotalVal(const std::string& metric,
                                   const MetricDataItem& hostTotalData,
                                   const std::vector<NormalizedMetricDataPtr> &metricDataVec) = 0;

    virtual void checkMetricTotal(std::vector<NormalizedMetricDataPtr> &metricDataVec);
    virtual bool checkMetricTotalVal(const MetricDataItem& metricTotalData,
            const std::vector<NormalizedMetricDataPtr> &metricDataVec) =0;

    template<typename T>
    void removeShieldAbnormalInfo(time_t curTime, 
            std::map<std::string, std::map<uint32_t, T> >& abnormalInfo) const;
    template<typename T>
    void genAbnormalStr(const std::map<std::string, std::map<uint32_t, T> >& abnormalInfo, std::string& abnormalStr) const;
    std::string abnormalValueToStr(const double& abnormalValue) const
    {
        char buffer[32];
        snprintf(buffer, sizeof(buffer), "[%.2lf]", abnormalValue);
        return buffer;
    }
    template<typename AbnormalVal>
    std::string abnormalValueToStr(const AbnormalVal& abnormalValue) const
    {
        return abnormalValue.toString();
    }
    
private:
    void genNormalizedData(std::vector<process::NormalizedMetricDataPtr>& normalizedMetricDataVec);
    void genNormalizedData(const fetcher::MetricNodePtr& dataRoot,
                           process::NormalizedMetricDataPtr& normalizedMetricDataVec,
                           bool updateFetchNoValidData = true);
    bool genNormalizedDataFromMetric(const fetcher::MetricPath& metricPath,
                                     NormalizedMetricDataPtr& normalizedMetricData,
                                     bool updateFetchNoValidData = true);

    bool isFiltered(const std::string&metric, uint32_t host) const;

    bool isMetricMissed(const std::string& metric) {
        return _missedMetric.find(metric) != _missedMetric.end();
    }

protected:
    tree::ProcessPackagePtr _processPackage;
    tree::RequestPackagePtr _requestPackage;
    config::ConfigWrapperPtr _configWrapper;
    bool _hasNoMetric;
    alarm::AlarmMsgPtr _alarmMsg;
private:
    std::map<std::string, std::set<uint32_t> > _fetchNoValidDataMap;
    std::set<std::string> _missedMetric;

private:
    friend class PolicyCheckerTest;
    friend class AvailabilityPolicyCheckerTest;
    friend class CyclePolicyCheckerTest;
    friend class SingularityPolicyCheckerTest;
    friend class ThresholdPolicyCheckerTest;
    friend class TrendPolicyCheckerTest;
};

template<typename T>
void PolicyChecker::removeShieldAbnormalInfo(time_t curTime,
        std::map<std::string, std::map<uint32_t, T> >& abnormalInfo) const
{
    assert(NULL != _processPackage);
    const tree::RequestPackagePtr& requestPackage = _processPackage->getRequestPackage();
    assert(NULL != requestPackage);
    const config::ConfigWrapperPtr config = requestPackage->getConfig();
    assert(NULL != config);
    config::PolicyConfigItemBasePtr policyItem = requestPackage->getPolicyItem();
    assert(NULL != policyItem);
    const std::string& group = policyItem->getGroup();

    typename std::map<std::string, std::map<uint32_t, T> >::iterator metricIter = 
        abnormalInfo.begin();
    while(metricIter != abnormalInfo.end()) {
        const std::string& metric = metricIter->first;
        std::map<uint32_t, T>& hostMap = metricIter->second;
        typename std::map<uint32_t, T>::iterator hostIter = hostMap.begin();
        while(hostIter != hostMap.end()) {
            uint32_t host = hostIter->first;
            if (config->isShielded(curTime, group, metric, host)) {
                LOG(INFO) << group << "." << metric <<"." << metric << " is shielded";
                hostMap.erase(hostIter++);
            }
            else {
                ++hostIter;
            }
        }
        if (hostMap.empty()) {
            abnormalInfo.erase(metricIter++);
        }
        else {
            ++metricIter;
        }
    }
}

template<typename T>
void PolicyChecker::genAbnormalStr(
    const std::map<std::string, std::map<uint32_t, T> >& abnormalInfo,
    std::string& abnormalStr) const
{
    abnormalStr = "";
    typename std::map<std::string, std::map<uint32_t, T> >::const_iterator metricIter = abnormalInfo.begin();
    for(; metricIter != abnormalInfo.end(); ++metricIter) {
        const std::string& metric = metricIter->first;
        if (!abnormalStr.empty()) {
            abnormalStr.append("," + metric + ":");
        }
        else {
            abnormalStr.append(metric + ":");
        }
        const std::map<uint32_t, T>& hostMap = metricIter->second;
        typename std::map<uint32_t, T>::const_iterator hostIter = hostMap.begin();
        for(; hostIter != hostMap.end(); ++hostIter) {
            uint32_t host = hostIter->first;
            const T& abnormalValue = hostIter->second;
            if (host != 0) {
                std::string ipStr = util::Util::IPToString(host);
                abnormalStr.append(ipStr);
            }
            const std::string& abnormalValueStr = abnormalValueToStr(abnormalValue);
            abnormalStr.append(abnormalValueStr);
        }
    }
}

RA_TYPEDEF_PTR(PolicyChecker);

RA_END_NAMESPACE(process);

#endif //RA_POLICYCHECKER_H
