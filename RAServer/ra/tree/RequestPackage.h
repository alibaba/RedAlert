#ifndef RA_REQUESTPACKAGE_H
#define RA_REQUESTPACKAGE_H

#include <ra/common/CommonDefine.h>
#include <ra/common/Log.h>
#include <ra/config/ConfigWrapper.h>
#include <ra/fetcher/MetricNode.h>

RA_BEGIN_NAMESPACE(tree);

class RequestPackage
{
public:
    RequestPackage(const config::ConfigWrapperPtr& config, 
                   const config::PolicyConfigItemBasePtr& policyItem);
    ~RequestPackage();

private:
    RequestPackage(const RequestPackage &);
    RequestPackage& operator=(const RequestPackage &);

public:
    const config::ConfigWrapperPtr& getConfig() const {
        return _config;
    }

    const config::PolicyConfigItemBasePtr& getPolicyItem() const {
        return _policyItem;
    }

    config::CheckType getCheckType() const {
        if (NULL == _policyItem) {
            return config::CT_NONE;
        }
        return _policyItem->getCheckType();
    }

    config::PolicyTrigger getTriggerType() const {
        if (NULL == _policyItem) {
            return config::PT_NONE;
        }
        return _policyItem->getTriggerType();
    }

    void getRetrieveTimeRange(std::vector<config::RetrieveTimeRange>& retrieveTimeRangeVec) const;

    void swapMetricSet(StringSet& metricSet) {
        _metricSet.swap(metricSet);
    }

    const StringSet& getMetricSet() const {
        return _metricSet;
    }

    void updateMetricRootMap(const std::string& spec,
                             const fetcher::MetricNodePtr& rootPtr)
    {
        _metricRootMap[spec] = rootPtr;
    }

    void getSpecs(StringSet& specSet) const;

    fetcher::MetricNodePtr getMetricRoot(const std::string& spec) const;
    
private:
    const config::ConfigWrapperPtr _config;
    const config::PolicyConfigItemBasePtr _policyItem;
    std::vector<config::RetrieveTimeRange> _retrieveTimeRangeVec;
    fetcher::MetricRootMap _metricRootMap;
    StringSet _metricSet;

private:
    RA_LOG_DECLARE();
};

RA_TYPEDEF_PTR(RequestPackage);

RA_END_NAMESPACE(tree);

#endif //RA_REQUESTPACKAGE_H
