#ifndef RA_BOOTSTRAPCONFIG_H
#define RA_BOOTSTRAPCONFIG_H

#include <ra/common/CommonDefine.h>
#include <ra/common/Log.h>
#include <ra/common/Json.h>

RA_BEGIN_NAMESPACE(config);

class BootstrapConfig
{
public:
    BootstrapConfig();
    ~BootstrapConfig();
public:

    bool fromJson(const common::JsonObjectPtr& json);

    const std::string& getAliMonitorUrl() const {
	return _aliMonitorUrl;
    }

    size_t getAlarmThreadNum() const {
        return _alarmThreadNum;
    }

    size_t getAlarmQueueSize() const {
        return _alarmQueueSize;
    }

    size_t getCheckerThreadNum() const {
        return _checkerThreadNum;
    }

    size_t getCheckerQueueSize() const {
        return _checkerQueueSize;
    }

    size_t getFetcherThreadNum() const {
        return _fetcherThreadNum;
    }

    size_t getFetcherQueueSize() const {
        return _fetcherQueueSize;
    }
    uint32_t getRetrieveMetricsPointCount() const {
        return _retrieveMetricsPointCount;
    }
    int32_t getMaxRandomLastRetriveTimeValueSec() const {
        return _maxRandomLastRetriveTimeValueSec;
    }

    int32_t getReloadTreeIntervalSec() const {
        return _reloadTreeIntervalSec;
    }

    const std::string& getInternalAlarmGroup() const
    {
        return _internalAlarmGroup;
    }

    const std::string& getInternalAlarmLevel() const
    {
        return _internalAlarmLevel;
    }
    
    int32_t getInteralMinAlarmInterval() const 
    {
        return _interalMinAlarmInterval;
    }

    int32_t getAmonitorAgentPort() const
    {
        return _amonitorAgentPort;
    }

    const std::string& getMailUser() const
    {
        return _mailUser;
    }

    const std::string& getMailPwd() const
    {
        return _mailPwd;
    }

    const std::string& getMailServerUrl() const
    {
        return _mailServerUrl;
    }

    int64_t getSendOkCodeIntervalSec() const
    {
        return _sendOkCodeIntervalSec;
    }

    const std::string& getHeartbeatHost() const
    {
        return _heartbeatHost;
    }

    uint32_t getHeartbeatPort() const
    {
        return _heartbeatPort;
    }

private:
    std::string _aliMonitorUrl;
    size_t _alarmThreadNum;
    size_t _alarmQueueSize;
    size_t _checkerThreadNum;
    size_t _checkerQueueSize;
    size_t _fetcherThreadNum;
    size_t _fetcherQueueSize;
    uint32_t _retrieveMetricsPointCount;
    
    int32_t _reloadTreeIntervalSec;
    std::string _internalAlarmGroup;
    std::string _internalAlarmLevel;
    int32_t _interalMinAlarmInterval;
    int32_t _amonitorAgentPort;
    std::string _mailUser;
    std::string _mailPwd;
    std::string _mailServerUrl;
    int32_t _maxRandomLastRetriveTimeValueSec;
    int64_t _sendOkCodeIntervalSec;
    std::string _heartbeatHost;
    uint32_t _heartbeatPort;

private:
    RA_LOG_DECLARE();
};

RA_TYPEDEF_PTR(BootstrapConfig);

RA_END_NAMESPACE(config);

#endif //RA_BOOTSTRAPCONFIG_H
