#ifndef RA_SHIELDCONFIG_H
#define RA_SHIELDCONFIG_H

#include <ra/common/Common.h>
#include <ra/common/Log.h>
#include <ra/util/Util.h>
#include <ra/util/ThreadLock.h>

RA_BEGIN_NAMESPACE(process);
class PolicyCheckerTest;
RA_END_NAMESPACE(process);

RA_BEGIN_NAMESPACE(config);

struct ShieldItem
{
    uint32_t id;
    std::string group;
    std::string metric;
    StringSet hostSet;
    time_t endTime;

    bool isMatched(const std::string& group,
                   const std::string& metric, uint32_t host) const
    {
        if (!util::Util::isPatternMatch(this->group, group) 
            ||!util::Util::isPatternMatch(this->metric, metric))
        {
            return false;
        }
        std::string hostIp;
        util::Util::IPToString(host, hostIp);
        StringSet::const_iterator it = hostSet.begin();
        for (; it != hostSet.end(); ++it) {
            if (util::Util::isPatternMatch(*it, hostIp)){
                return true;
            }
        }
        return false;
    }
};

class ShieldConfig
{
public:
    ShieldConfig();
    ~ShieldConfig();

private:
    ShieldConfig(const ShieldConfig &);
    ShieldConfig& operator=(const ShieldConfig &);

public:
    void clear();
    bool loadConfig(const std::string& configFilePath);

    bool isShielded(time_t curTime, const std::string& group, 
                    const std::string& metric, uint32_t host) const;
    size_t getShieldItemCount() const {
        return _shieldItemVec.size();
    }


private:
    time_t getMetricShieldTime(const std::string& metricKey) const;
    void setMetricShieldTime(const std::string& metricKey, time_t endTime) const;
            
private:
    std::vector<ShieldItem> _shieldItemVec;
    //cache shield end time for each metric
    mutable util::Mutex _cacheLock;
    mutable std::map<std::string, time_t>  _metricShieldTimeMap;

private:
    friend class ShieldConfigTest;
    friend class process::PolicyCheckerTest;
};

typedef std::vector<ShieldConfig> ShieldConfigList;

RA_TYPEDEF_PTR(ShieldConfig);

RA_END_NAMESPACE(config);

#endif //RA_SHIELDCONFIG_H
