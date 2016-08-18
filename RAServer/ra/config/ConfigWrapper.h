#ifndef RA_CONFIGWRAPPER_H
#define RA_CONFIGWRAPPER_H

#include <string>
#include <ra/common/Common.h>
#include <ra/common/Log.h>
#include <ra/config/PolicyConfig.h>
#include <ra/config/DataSourceConfig.h>
#include <ra/config/RedAlertConfig.h>
#include <ra/config/ShieldConfig.h>
#include <ra/fetcher/MetricFetcher.h>

RA_BEGIN_NAMESPACE(tree);
class TreeManagerTest;
RA_END_NAMESPACE(tree);

RA_BEGIN_NAMESPACE(service);
class HeartbeatClientTest;
class LoadConfigHandlerTest;
RA_END_NAMESPACE(service);

RA_BEGIN_NAMESPACE(process);
class PolicyCheckerTest;
class FetcherManagerTest;
class FetcherWorkItemTest;
RA_END_NAMESPACE(process);


RA_BEGIN_NAMESPACE(config);

class ConfigWrapper
{
public:
    ConfigWrapper();
    ~ConfigWrapper();
private:
    ConfigWrapper(const ConfigWrapper &);
    ConfigWrapper& operator=(const ConfigWrapper &);

public:
    int32_t getConfigVersion(){
        return _configVersion;
    } 
    bool loadConfig(const std::string& configRoot, int32_t configVersion,
                    const std::string& localAddress);

    double getSmoothingFactor() const {
        return _smoothingFactor;
    }
    double getTrendFactor() const {
        return _trendFactor;
    }
    void getAmonSpecs(StringSet& specSet) const {
        _dataSourceConfig.getSpecs(specSet);
    }
    bool isEmptyConfig() const {
        return !_redAlertConfig.containThisServer() ||
            _policyConfig.getPolicyCount() == 0;
    }
    bool isMonitored(const std::string& metric) const {
        return _policyConfig.isMonitored(metric);
    }
    
    bool isShielded(time_t curTime, const std::string& group, 
                    const std::string& metric, uint32_t host) const 
    {
        return _shieldConfig.isShielded(curTime, group, metric, host);
    }
    int32_t getConfigVersion() const {
        return _configVersion;
    }
    const std::string& getServiceName() const {
        return _redAlertConfig.getServiceName();
    }

    fetcher::MetricFetcherPtr getMetricFetcher(
            const std::string& spec) const
    {
        return _dataSourceConfig.getMetricFetcher(spec);
    }

    void getPolicyItems(std::vector<PolicyConfigItemBasePtr>& policyItemVec) const {
        return _policyConfig.getPolicyItems(policyItemVec);
    }
    
private:
    void clear();
    bool loadPairVals(const std::string& configFilePath);

private:
    PolicyConfig _policyConfig;
    DataSourceConfig _dataSourceConfig;
    RedAlertConfig _redAlertConfig;
    ShieldConfig _shieldConfig;
    
    double _smoothingFactor;
    double _trendFactor;

    int32_t _configVersion;

private:
    friend class ConfigWrapperTest;
    friend class tree::TreeManagerTest;
    friend class service::HeartbeatClientTest;
    friend class service::LoadConfigHandlerTest;
    friend class process::PolicyCheckerTest;
    friend class process::FetcherManagerTest;
    friend class process::FetcherWorkItemTest;

private:
    RA_LOG_DECLARE();
};

RA_TYPEDEF_PTR(ConfigWrapper);

RA_END_NAMESPACE(config);

#endif //RA_CONFIGWRAPPER_H
