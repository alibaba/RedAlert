#ifndef RA_CHECKERMANAGER_H
#define RA_CHECKERMANAGER_H

#include <ra/common/Common.h>
#include <ra/common/Log.h>
#include <ra/util/ThreadLock.h>
#include <ra/util/ThreadPool.h>
#include <ra/alarm/AlarmManager.h>
#include <ra/tree/ProcessPackage.h>

RA_BEGIN_NAMESPACE(process);

class CheckerManager
{
public:
    CheckerManager();
    ~CheckerManager();
private:
    CheckerManager(const CheckerManager &);
    CheckerManager& operator=(const CheckerManager &);

public:
    bool init(size_t threadNum, size_t queueSize, alarm::AlarmManager* alarmManager);
    bool start();
    void stop();
    bool pushProcessPackage(tree::ProcessPackagePtr& processPackage);
    size_t getQueueItemCount() const;

private:
    util::ThreadPool *_checkerPool;
    alarm::AlarmManager* _alarmManager;
    bool _started;
    size_t _threadNum;
    size_t _queueSize;
    bool _isBlocked;
    mutable util::Mutex _mutex;

private:
    friend class FetcherWorkItemTest;
    friend class CheckerManagerTest;

private:
    RA_LOG_DECLARE();
};

RA_TYPEDEF_PTR(CheckerManager);

RA_END_NAMESPACE(process);

#endif //RA_CHECKERMANAGER_H
