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
    LOG(INFO) << "alarmLevel:[" << _alarmLevel << "], alarmGroup:["
	      << _alarmGroup << "], alarmMsg:[" << _alarmMsg 
	      <<"], description:[" << _description <<"]";
}

RA_END_NAMESPACE(common);

