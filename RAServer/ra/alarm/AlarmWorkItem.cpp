#include <ra/alarm/AlarmWorkItem.h>

RA_USE_NAMESPACE(tree);
RA_USE_NAMESPACE(config);
using namespace std;
RA_BEGIN_NAMESPACE(alarm);

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
        LOG(ERROR) << "unkown alarm level:" << alarmLevel << ", can not send alarm message";
    }
}

RA_END_NAMESPACE(alarm);

