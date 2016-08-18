#include <ra/process/CheckerManager.h>
#include <ra/process/CheckerWorkItem.h>

RA_USE_NAMESPACE(util);
RA_USE_NAMESPACE(alarm);
RA_USE_NAMESPACE(tree);

RA_BEGIN_NAMESPACE(process);
RA_LOG_SETUP(process, CheckerManager);

CheckerManager::CheckerManager() 
    : _checkerPool(NULL)
    , _alarmManager(NULL)
    , _started(false)
    , _threadNum(0)
    , _queueSize(0)
    , _isBlocked(true)
{
}

CheckerManager::~CheckerManager() { 
    stop();
}

bool CheckerManager::init(size_t threadNum, size_t queueSize, 
                          AlarmManager* alarmManager)
{
    if (NULL == alarmManager) {
        RA_LOG(ERROR, "alarmManager is NULL , init CheckerManager failed");
        return false;
    }
    _threadNum = threadNum;
    _queueSize = queueSize;
    _alarmManager = alarmManager;
    

    RA_LOG(INFO, "Checker Manager inited success: threadNum:%zu, queueSize:%zu",
           _threadNum, _queueSize);

    return true;
}

bool CheckerManager::start()
{
    if (NULL == _alarmManager) {
        RA_LOG(ERROR, "can not start CheckerManager without inited");
        return false;
    }
    if (_started) {
        RA_LOG(ERROR, "can not start CheckerManager thread twice");
        return false;
    }
    _checkerPool = new ThreadPool(_threadNum, _queueSize);
    if (_checkerPool == NULL || !_checkerPool->start()) {
        RA_LOG(ERROR, "checker thread pool start fail!");
        delete _checkerPool;
        _checkerPool = NULL;
        return false;
    }
    _started = true;
    RA_LOG(INFO, "checker manager start success");
    return true;
}

void CheckerManager::stop()
{
    _started = false;
    if (_checkerPool) {
        _checkerPool->stop();
        delete _checkerPool;
        _checkerPool = NULL;
    }
}

size_t CheckerManager::getQueueItemCount() const
{
    if (NULL == _checkerPool) {
        return 0;
    }
    return _checkerPool->getQueueSize();
}

bool CheckerManager::pushProcessPackage(ProcessPackagePtr& processPackage)
{
    ScopedLock lock(_mutex);
    if (!_started || NULL == _checkerPool) {
        RA_LOG(ERROR, "can not push processPackage whithout started checker manager");
        return false;
    }
    CheckerWorkItem *workItem = new CheckerWorkItem();
    assert(NULL != workItem);
    if (!workItem->init(_alarmManager, processPackage)) {
        delete workItem;
        RA_LOG(ERROR, "init check workitem failed");
        return false;
    }
    if (!_checkerPool->push(workItem, _isBlocked)) {
        delete workItem;
        RA_LOG(ERROR, "push process package to  checker pool failed");
        return false;
    }
    RA_LOG(DEBUG, "push process package to checker pool success, metric:%s, id:%u", 
           processPackage->getRequestPackage()->getPolicyItem()->getMetric().c_str(),
           processPackage->getRequestPackage()->getPolicyItem()->getId());
    return true;
}


RA_END_NAMESPACE(process);

