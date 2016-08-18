#ifndef RA_DATA_SOURCE_CONFIG_H
#define RA_DATA_SOURCE_CONFIG_H

#include <string>
#include <ra/common/Common.h>
#include <ra/common/Log.h>
#include <ra/util/ThreadLock.h>
#include <ra/config/HostItem.h>
#include <ra/fetcher/MetricFetcher.h>

RA_BEGIN_NAMESPACE(tree);
class TreeManagerTest;
RA_END_NAMESPACE(tree);

RA_BEGIN_NAMESPACE(process);
class FetcherManagerTest;
class FetcherWorkItemTest;
RA_END_NAMESPACE(process);

RA_BEGIN_NAMESPACE(config);

class DataSourceConfig
{
public:
    DataSourceConfig();
    ~DataSourceConfig();
private:
    DataSourceConfig(const DataSourceConfig &);
    DataSourceConfig& operator=(const DataSourceConfig &);

public:
    bool loadConfig(const std::string& configFilePath);
    size_t getAmonitorCount() const {
        return _hostVec.size();
    }

    fetcher::MetricFetcherPtr getMetricFetcher(const std::string& spec) const;
    void getSpecs(StringSet& specSet) const;
    
    void clear();

private:
    typedef std::map<std::string, fetcher::MetricFetcherPtr> MetricFetcherMap;

    std::vector<HostItem> _hostVec;
    MetricFetcherMap _metricFetcherMap;
    mutable util::Mutex _mutex;

private:
    friend class DataSourceConfigTest;
    friend class tree::TreeManagerTest;
    friend class process::FetcherManagerTest;
    friend class process::FetcherWorkItemTest;
    
private:
    RA_LOG_DECLARE();
};

RA_TYPEDEF_PTR(DataSourceConfig);

RA_END_NAMESPACE(config);

#endif //RA_DATA_SOURCE_CONFIG_H
