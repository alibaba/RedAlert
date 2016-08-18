#ifndef RA_POLICYCONFIGPARSER_H
#define RA_POLICYCONFIGPARSER_H

#include <string>
#include <tr1/functional>
#include <ra/common/CommonDefine.h>
#include <ra/common/Log.h>
#include <ra/config/ThresholdPolicyItem.h>
#include <ra/config/DeviationPolicyItem.h>
#include <ra/config/AvailabilityPolicyItem.h>
#include <ra/config/TrendPolicyItem.h>
#include <ra/config/SingularityPolicyItem.h>
#include <ra/config/CyclePolicyItem.h>

RA_BEGIN_NAMESPACE(tree);
class TreeManagerTest;
RA_END_NAMESPACE(tree);

RA_BEGIN_NAMESPACE(config);

class PolicyConfig
{
public:
    PolicyConfig();
    ~PolicyConfig();

private:
    PolicyConfig(const PolicyConfig &);
    PolicyConfig& operator=(const PolicyConfig &);

public:
    void filterMetrics(const std::tr1::function<bool (const std::string&)>& isMyMetric);
    bool loadConfig(const std::string& configFilePath);
    bool isMonitored(const std::string& metric) const;
    void getPolicyItems(std::vector<PolicyConfigItemBasePtr>& policyItemVec) const {
        policyItemVec.assign(_policyItemVec.begin(), _policyItemVec.end());
    }
    size_t getPolicyCount() const {
        return _policyItemVec.size();
    }
    
    void clear();

private:
    PolicyConfigItemBasePtr createPolicyItem(const std::string& policyType) const;

private:
    std::vector<PolicyConfigItemBasePtr> _policyItemVec;

private:
    friend class PolicyConfigTest;
    friend class ConfigWrapperTest;
    friend class tree::TreeManagerTest;

private:
    RA_LOG_DECLARE();
};

RA_TYPEDEF_PTR(PolicyConfig);

RA_END_NAMESPACE(config);

#endif //RA_POLICYCONFIGPARSER_H
