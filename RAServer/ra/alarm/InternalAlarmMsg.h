#ifndef RA_INTERNALALARMMSG_H
#define RA_INTERNALALARMMSG_H

#include <ra/common/Common.h>
#include <ra/common/Log.h>
#include <ra/alarm/AlarmMsg.h>

RA_BEGIN_NAMESPACE(alarm);

class InternalAlarmMsg: public AlarmMsg
{
public:
    InternalAlarmMsg(const std::string& alarmGroup, 
                     const std::string& alarmLevel, 
                     int32_t interalMinAlarmInterval,
                     const config::ConfigWrapperPtr& config,
                     MsgCode code);
    ~InternalAlarmMsg();

private:
    InternalAlarmMsg(const InternalAlarmMsg &);
    InternalAlarmMsg& operator=(const InternalAlarmMsg &);

public:
    /*override*/ AlarmMsgKey getKey() const {
        AlarmMsgKey key;
        key.id = INTERNAL_ALARM_POLICY_ID;
        key.metric = INTERNAL_ALARM_METRIC;
        return key;
    }

    /*override*/ int64_t getMinAlarmInterval() const {
        return _interalMinAlarmInterval;
    }
    /*override*/ const std::string& getAlarmGroup() const {
        return _alarmGroup;
    }
    /*override*/ const std::string& getAlarmLevel() const {
        return _alarmLevel;
    }
    /*override*/ void setPackageLastAlarmTimeSec(int64_t lastAlarmTime)
    {
        return;
    }
    /*override*/ std::string getDescription() const {
        if (_code == CODE_OK) {
            return AlarmMsg::getDescription();
        }
        return "internal error";
    }

protected:
    /*override*/ std::string getAlarmMsgPrefix() const {
        assert(NULL != _config);
        char buffer[512];
        snprintf(buffer, sizeof(buffer), "%s:%d#", 
                 _config->getServiceName().c_str(), _config->getConfigVersion());
        return buffer;
    }

private:
    std::string _alarmGroup;
    std::string _alarmLevel;
    int32_t _interalMinAlarmInterval;
    config::ConfigWrapperPtr _config;

private:
    RA_LOG_DECLARE();
};

RA_TYPEDEF_PTR(InternalAlarmMsg);

RA_END_NAMESPACE(alarm);

#endif //RA_INTERNALALARMMSG_H
