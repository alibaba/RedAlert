#include <ra/alarm/InternalAlarmMsg.h>

using namespace std;
RA_USE_NAMESPACE(config);
RA_USE_NAMESPACE(tree);

RA_BEGIN_NAMESPACE(alarm);

InternalAlarmMsg::InternalAlarmMsg(const string& alarmGroup, 
        const string& alarmLevel, int32_t interalMinAlarmInterval,
        const ConfigWrapperPtr& config, MsgCode code)
    : AlarmMsg(ProcessPackagePtr(), code)
    , _alarmGroup(alarmGroup)
    , _alarmLevel(alarmLevel)
    , _interalMinAlarmInterval(interalMinAlarmInterval)
    , _config(config)
{
    
}

InternalAlarmMsg::~InternalAlarmMsg() { 
}

RA_END_NAMESPACE(alarm);

