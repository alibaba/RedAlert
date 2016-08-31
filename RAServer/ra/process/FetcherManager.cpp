#include <ra/util/Util.h>
#include <ra/process/FetcherManager.h>
#include <ra/process/FetcherWorkItem.h>
#define COLLECT_AGENT_DATA_DELAY (5 * 1000000) // 5s
using namespace std;
RA_USE_NAMESPACE(util);
RA_USE_NAMESPACE(tree);
RA_USE_NAMESPACE(config);
RA_USE_NAMESPACE(alarm);
RA_USE_NAMESPACE(fetcher);

RA_BEGIN_NAMESPACE(process);
RA_LOG_SETUP(process, FetcherManager);

FetcherManager::FetcherManager() 
    : _alarmManager(NULL)
    , _retrieveMetricsPointCount(1)
    , _fetcherPool(NULL)
    , _checkManager(NULL)
    , _threadNum(0)
    , _queueSize(0)
    , _interalMinAlarmInterval(60)
    , _started(false)
    , _lastNonOkAlarmTimeSec(INVALID_TIME)
{ 
}

FetcherManager::~FetcherManager() {
    stop();
}

bool FetcherManager::init(AlarmManager* alarmManager, size_t threadNum, 
                          size_t queueSize, const string& internalAlarmGroup, 
                          const string& internalAlarmLevel, 
                          int32_t interalMinAlarmInterval,
                          uint32_t retrieveMetricsPointCount,
                          CheckerManager* checkManager)
{
    if (NULL == alarmManager) {
        LOG(ERROR) << "alarmManager is NULL, init FetcherManager failed";
        return false;
    }
    if (NULL == checkManager) {
        LOG(ERROR) << "checker pool is NULL, init FetcherManager failed";
        return false;
    }
    if (0 == retrieveMetricsPointCount) {
        LOG(ERROR) << "retrieveMetricsPointCount is 0, init FetcherManager failed";
        return false;
    }
    _alarmManager = alarmManager;
    _threadNum = threadNum;
    _queueSize = queueSize;
    _internalAlarmGroup = internalAlarmGroup;
    _internalAlarmLevel = internalAlarmLevel;
    _interalMinAlarmInterval = interalMinAlarmInterval;
    _retrieveMetricsPointCount = retrieveMetricsPointCount;
    _checkManager = checkManager;

    LOG(INFO) << "Fetcher Manager inited success: threadNum:" 
	      << _threadNum <<", queueSize:" << _queueSize
	      << "internalAlarmGroup:" << _internalAlarmGroup 
	      <<", internalAlarmLevel:" << _internalAlarmLevel 
	      << ", retrieveMetricsPointCount:" << _retrieveMetricsPointCount;
    return true;
}

bool FetcherManager::start()
{
    if (NULL == _alarmManager || NULL == _checkManager) {
        LOG(ERROR) << "can not start FetcherManager thread pool whithout inited";
        return false;
    }
    if (_started) {
        LOG(ERROR) << "can not start FetcherManager thread twice";
        return false;
    }
    _fetcherPool = new util::ThreadPool(_threadNum, _queueSize);
    assert(NULL != _fetcherPool);
    if (!_fetcherPool->start()) {
        delete _fetcherPool;
        _fetcherPool = NULL;
        LOG(ERROR) << "fetcher thread start fail!";
        return false;
    }
    _started = true;
    LOG(INFO) << "Fetcher manager start success";
    return true;
}

void FetcherManager::stop()
{
    _started = false;
    if (_fetcherPool) {
        _fetcherPool->stop();
        delete _fetcherPool;
        _fetcherPool = NULL;
    }
}

size_t FetcherManager::getQueueItemCount() const
{
    if (NULL == _fetcherPool) {
        return 0;
    }
    return _fetcherPool->getQueueSize();
}

bool FetcherManager::pushProcessPackage(ProcessPackagePtr& processPackage)
{
    if (!_started || NULL == _fetcherPool) {
        LOG(ERROR) << "can not push processPackage whithout started fetcher manager";
        return false;
    }
    if (isNeedSetAlarmCodeOk(processPackage)) {
        return true;
    }
    FetcherWorkItem *workItem = new FetcherWorkItem(this, processPackage,
            _checkManager, _alarmManager);
    if (!_fetcherPool->push(workItem)) {
        delete workItem;
        LOG(ERROR) << "push process package for fetch fail";
        return false;
    }
    VLOG(1) << "push process package to fetcher pool success, metric:" 
	    << processPackage->getRequestPackage()->getPolicyItem()->getMetric() <<", id:"
	    << processPackage->getRequestPackage()->getPolicyItem()->getId();
    return true;
}

MetricFetcherPtr FetcherManager::getMetricFetcher(
    const ConfigWrapperPtr& configPtr, const string& spec) {
    MetricFetcherPtr fetcher = configPtr->getMetricFetcher(spec);
    if (NULL == fetcher) {
        string alarmMsg = "get fetcher with spec[" + spec + "] failed";
        pushInternalAlarmMessage(alarmMsg, configPtr);
    }
    return fetcher;
}

bool FetcherManager::getMetricTree(
    string spec, int64_t start, int64_t end,
    const config::ConfigWrapperPtr& configPtr,
    fetcher::MetricNodePtr& metricRoot)
{
    MetricFetcherPtr fetcher = getMetricFetcher(configPtr, spec);
    if (fetcher == NULL) {
        return false;
    }
    metricRoot = fetcher->allocTree(start, end);
    if (metricRoot == NULL) {
        string alarmMsg = "get metric tree from fetcher[" + spec + "] fail";
        pushInternalAlarmMessage(alarmMsg, configPtr);
        return false;
    }
    return true;
}

bool FetcherManager::retrieveMetrics(
    ProcessPackagePtr& processPackage, int64_t curTimeUs)
{
    const RequestPackagePtr requestPackage = processPackage->getRequestPackage();
    assert(NULL != requestPackage);
    const ConfigWrapperPtr &configPtr = requestPackage->getConfig();
    assert(NULL != configPtr);

    vector<RetrieveTimeRange> retrieveTimeRangeVec;
    requestPackage->getRetrieveTimeRange(retrieveTimeRangeVec);
    size_t timeRangeCount = retrieveTimeRangeVec.size();
    if (0 == timeRangeCount) {
        PolicyConfigItemBasePtr policyItem =  requestPackage->getPolicyItem();
        LOG(ERROR) << "There is no retrieve timeRange of policy item, "
	    "maybe policy config is not correct, metric:" << policyItem->getMetric() 
	    << ", id:" << policyItem->getId();
        return false;
    }

    StringSet specSet;
    requestPackage->getSpecs(specSet);
    if (specSet.empty()) {
        LOG(INFO) << "Spec set is empty";
        return true;
    }

    vector<MetricNodePtr> metricDataRoots(timeRangeCount);
    for (size_t i = 0; i < retrieveTimeRangeVec.size(); ++i) {
        const RetrieveTimeRange& retrieveTimeRange = retrieveTimeRangeVec[i];
        int64_t startTime = curTimeUs + retrieveTimeRange.start * MICROSECONDS - COLLECT_AGENT_DATA_DELAY;
        int64_t endTime = curTimeUs + retrieveTimeRange.end * MICROSECONDS - COLLECT_AGENT_DATA_DELAY;
        int64_t timeStep = (endTime - startTime + 1 + (_retrieveMetricsPointCount - 1)) / _retrieveMetricsPointCount;
        for (StringSet::const_iterator it = specSet.begin(); it != specSet.end(); ++it) {
            const string& spec = *it;
            MetricNodePtr metricRoot = requestPackage->getMetricRoot(spec);
            MetricFetcherPtr fetcher = getMetricFetcher(configPtr, spec);
            if (fetcher == NULL) {
                LOG(ERROR) << "Cannot find fetcher for " <<  spec;
                return false;
            }
            MetricNodePtr dataRoot = fetcher->retrieve(metricRoot, startTime, endTime, timeStep);
            if (dataRoot == NULL) {
                string alarmMsgStr = "retrieve metrics with fetcher[" + spec + "] request fail";
                pushInternalAlarmMessage(alarmMsgStr, configPtr);
                return false;
            }
            metricDataRoots[i].swap(dataRoot);
        }
    }

    VLOG(1) << "Retrieve " << metricDataRoots.size() << " metric data roots";
    processPackage->setMetricDataRoots(metricDataRoots);
    return true;
}

bool FetcherManager::isNeedSetAlarmCodeOk(ProcessPackagePtr& processPackage)
{
    int64_t currTimeSec = Util::currentTimeInMicroseconds();
    if (INVALID_TIME != _lastNonOkAlarmTimeSec && 
        currTimeSec - _lastNonOkAlarmTimeSec > _interalMinAlarmInterval) {
        const RequestPackagePtr requestPackage = processPackage->getRequestPackage();
        assert(NULL != requestPackage);
        const ConfigWrapperPtr &configPtr = requestPackage->getConfig();
        assert(NULL != configPtr);
        pushInternalAlarmMessage("set status ok", configPtr, CODE_OK);
        return true;
    }
    return false;
}

void FetcherManager::pushInternalAlarmMessage(const string& alarmMsgStr, 
        const ConfigWrapperPtr& configPtr, MsgCode code)
{
    LOG(ERROR) << alarmMsgStr;
    InternalAlarmMsgPtr internalAlarmMsgPtr(new InternalAlarmMsg(
                _internalAlarmGroup, _internalAlarmLevel, 
                _interalMinAlarmInterval, configPtr, code));
    internalAlarmMsgPtr->setAlarmMsgStr(alarmMsgStr);
    if (_alarmManager == NULL) {
        LOG(ERROR) << "alarm manager is NULL";
        return;
    }

    AlarmMsgPtr alarmMsgPtr = tr1::static_pointer_cast<AlarmMsg>(internalAlarmMsgPtr);
    _alarmManager->pushAlarmMessage(alarmMsgPtr);

    if (code == CODE_OK) {
        LOG(INFO) << "set internal alarm code to OK";
        _lastNonOkAlarmTimeSec = INVALID_TIME;
    }
    else {
        _lastNonOkAlarmTimeSec = Util::currentTimeInMicroseconds();
    }
}

RA_END_NAMESPACE(process);
