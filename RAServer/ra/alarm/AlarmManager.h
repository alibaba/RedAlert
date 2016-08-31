#ifndef RA_ALARMMANAGER_H
#define RA_ALARMMANAGER_H

#include <ra/common/Common.h>
#include <ra/common/Log.h>
#include <ra/util/ThreadLock.h>
#include <ra/util/ThreadPool.h>
#include <ra/alarm/AlarmMsg.h>
#include <ra/tree/ProcessPackage.h>

RA_BEGIN_NAMESPACE(alarm);

class AlarmManager
{
public:
    AlarmManager();
    ~AlarmManager();
private:
    AlarmManager(const AlarmManager &);
    AlarmManager& operator=(const AlarmManager &);

public:
    bool init(size_t threadNum, size_t queueSize, 
              const std::string& alimonitorUrl,
              const std::string& mailUser, 
              const std::string& mailPwd, 
              const std::string& mailServer);
    bool start();
    void stop();
    
    size_t getQueueItemCount() const;
    bool pushAlarmMessage(AlarmMsgPtr& alarmMsg);
    
    void raiseCurlMailAlarm(AlarmMsgPtr& alarmMsg);
    void raiseAlimonitorAlarm(AlarmMsgPtr& alarmMsg);
    void raiseUcmtAlarm(AlarmMsgPtr& alarmMsg);
    bool doUcmtAlarm(const std::string& alarmGroup, const std::string& alarmMsgStr, 
                     int32_t code, const std::string& hostName="");
    
private:
    void clearExpiredRecordTime(int64_t currTimeSec);

private:
    bool _started;
    size_t _threadNum;
    size_t _queueSize;
    std::string _alimonitorUrl;
    std::string _mailUser;
    std::string _mailPwd;
    std::string _mailServer;

    util::ThreadPool *_alarmPool;
    std::map<AlarmMsgKey, int64_t> _lastAlarmTime;
    int64_t _lastClearExpiredRecordTimeSec;
    mutable util::Mutex _mutex;

private:
    friend class AlarmManagerTest;
};

RA_TYPEDEF_PTR(AlarmManager);

RA_END_NAMESPACE(alarm);

#endif //RA_ALARMMANAGER_H
