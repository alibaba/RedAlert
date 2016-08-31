#ifndef RA_ALARMWORKITEM_H
#define RA_ALARMWORKITEM_H

#include <ra/common/Common.h>
#include <ra/common/Log.h>
#include <ra/util/ThreadPool.h>
#include <ra/alarm/AlarmManager.h>
#include <ra/alarm/AlarmMsg.h>

RA_BEGIN_NAMESPACE(alarm);

class AlarmWorkItem: public util::WorkItem
{
public:
    AlarmWorkItem(AlarmManager* alarmManager, AlarmMsgPtr& alarmMsg);
    ~AlarmWorkItem();

private:
    AlarmWorkItem(const AlarmWorkItem &);
    AlarmWorkItem& operator=(const AlarmWorkItem &);

public:
    /*override*/ void process();

private:
    AlarmMsgPtr _alarmMsg;
    AlarmManager* _alarmManager;
};

RA_TYPEDEF_PTR(AlarmWorkItem);

RA_END_NAMESPACE(alarm);

#endif //RA_ALARMWORKITEM_H
