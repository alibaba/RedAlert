#include <ra/config/BootstrapConfig.h>
#include <ra/util/Util.h>

using namespace std;

RA_BEGIN_NAMESPACE(config);

BootstrapConfig::BootstrapConfig():
    _alarmThreadNum(1),
    _alarmQueueSize(1024),
    _checkerThreadNum(2),
    _checkerQueueSize(1024),
    _fetcherThreadNum(2),
    _fetcherQueueSize(1024),
    _retrieveMetricsPointCount(5),
    _reloadTreeIntervalSec(60),
    _interalMinAlarmInterval(60),
    _amonitorAgentPort(10086),
    _maxRandomLastRetriveTimeValueSec(DEFAULT_LAST_RETRIEVE_TIME_RANDOM_VALUE),
    _sendOkCodeIntervalSec(15)
{
}

BootstrapConfig::~BootstrapConfig() 
{
}

bool BootstrapConfig::fromJson(const common::JsonObjectPtr& json) {
    json->get("aliMonitorUrl", _aliMonitorUrl);
    json->get("alarmThreadNum", _alarmThreadNum, (size_t)1);
    json->get("alarmQueueSize", _alarmQueueSize, (size_t)1024);
    json->get("checkerThreadNum", _checkerThreadNum, (size_t)2);
    json->get("checkerQueueSize", _checkerQueueSize, (size_t)1024);
    json->get("fetcherThreadNum", _fetcherThreadNum, (size_t)2);
    json->get("fetcherQueueSize", _fetcherQueueSize, (size_t)1024);
    json->get("retrieveMetricsPointCount", _retrieveMetricsPointCount, 5U);
    json->get("maxRandomLastRetriveTimeValueSec", _maxRandomLastRetriveTimeValueSec,
              DEFAULT_LAST_RETRIEVE_TIME_RANDOM_VALUE);

    json->get("reloadTreeIntervalSec", _reloadTreeIntervalSec, 60);
    json->get("internalAlarmGroup", _internalAlarmGroup, string("shenma_ra_internal_alarm"));
    json->get("internalAlarmLevel", _internalAlarmLevel, ALARM_LEVEL_ALIMONITOR);
    json->get("interalMinAlarmInterval", _interalMinAlarmInterval, 60);
    json->get("amonitorAgentPort", _amonitorAgentPort, 10086);
    json->get("mailUser", _mailUser, string("foo@bar.com"));
    json->get("mailPassword", _mailPwd, string());
    json->get("mailServerUrl", _mailServerUrl, string());
    json->get("sendOkCodeIntervalSec", _sendOkCodeIntervalSec, _sendOkCodeIntervalSec);
    json->get("heartbeatHost", _heartbeatHost, string("127.0.0.1"));
    json->get("heartbeatPort", _heartbeatPort, HEARTBEAT_DEFAULT_PORT);
    return true;
}

RA_END_NAMESPACE(config);
