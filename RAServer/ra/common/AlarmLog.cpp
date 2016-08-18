#include <ra/common/AlarmLog.h>

RA_BEGIN_NAMESPACE(common);
RA_LOG_SETUP(common, AlarmLog);

AlarmLog::AlarmLog() 
{ 
}

AlarmLog::~AlarmLog() { 
    writeLog();
}

void AlarmLog::writeLog()
{
    RA_LOG(INFO, "alarmLevel:[%s], alarmGroup:[%s], alarmMsg:[%s], description:[%s]",
           _alarmLevel.c_str(), _alarmGroup.c_str(), _alarmMsg.c_str(), _description.c_str());
}

RA_END_NAMESPACE(common);

