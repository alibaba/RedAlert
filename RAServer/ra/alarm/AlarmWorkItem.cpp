#include <ra/alarm/AlarmWorkItem.h>

RA_USE_NAMESPACE(tree);
RA_USE_NAMESPACE(config);
using namespace std;
RA_BEGIN_NAMESPACE(alarm);
RA_LOG_SETUP(alarm, AlarmWorkItem);

AlarmWorkItem::AlarmWorkItem(AlarmManager* alarmManager, AlarmMsgPtr& alarmMsg)
    : _alarmMsg(alarmMsg)
    , _alarmManager(alarmManager)
{
}

AlarmWorkItem::~AlarmWorkItem() {
    
}

void AlarmWorkItem::process()
{
    assert(NULL != _alarmManager);
    assert(NULL != _alarmMsg);
    const string& alarmLevel = _alarmMsg->getAlarmLevel();
    if (alarmLevel == ALARM_LEVEL_ALIMONITOR) {
        _alarmManager->raiseAlimonitorAlarm(_alarmMsg);
    }
    else if(alarmLevel == ALARM_LEVEL_CURLMAIL){
        _alarmManager->raiseCurlMailAlarm(_alarmMsg);
    }
    else if(alarmLevel == ALARM_LEVEL_UCMT) {
        _alarmManager->raiseUcmtAlarm(_alarmMsg);
    }
    else {
        RA_LOG(ERROR, "unkown alarm level:%s, can not send alarm message", 
               alarmLevel.c_str());
    }
}

RA_END_NAMESPACE(alarm);

