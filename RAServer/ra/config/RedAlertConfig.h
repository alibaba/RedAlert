#ifndef RA_REDALERTCONFIG_H
#define RA_REDALERTCONFIG_H

#include <ra/common/Common.h>
#include <ra/common/Log.h>
#include <ra/config/HostItem.h>
#include <ra/util/ConsistentHash.h>
#include <string>
#include <vector>

RA_BEGIN_NAMESPACE(tree);
class TreeManagerTest;
RA_END_NAMESPACE(tree);

RA_BEGIN_NAMESPACE(process);
class FetcherManagerTest;
RA_END_NAMESPACE(process);


RA_BEGIN_NAMESPACE(config);

class RedAlertConfig
{
public:
    RedAlertConfig();
    ~RedAlertConfig();
private:
    RedAlertConfig(const RedAlertConfig &);
    RedAlertConfig& operator=(const RedAlertConfig &);
public:
    bool loadConfig(const std::string& configFilePath, const std::string& localAddress);
    bool isMyMetric(const std::string& metricPath) const;
    bool containThisServer() const {
        return _containThisServer;
    }
    void clear();
    size_t getRedAlertCount() const {
        return _hostVec.size();
    }
    const std::string& getServiceName() const {
        return _serviceName;
    }

private:
    void initHash();

private:
    typedef uint32_t (*HashMethod)(const char*, uint32_t);
    HashMethod _hashMethod;

    util::ConsistentHash _consistentHash;
    std::vector<HostItem> _hostVec;
    bool _containThisServer;
    std::string _serviceName;
    
private:
    friend class RedAlertConfigTest;
    friend class tree::TreeManagerTest;
    friend class process::FetcherManagerTest;
};

RA_TYPEDEF_PTR(RedAlertConfig);

RA_END_NAMESPACE(config);

#endif //RA_REDALERTCONFIG_H
