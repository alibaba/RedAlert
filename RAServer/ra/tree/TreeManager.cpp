#include <tr1/functional>
#include <ra/tree/TreeManager.h>
#include <ra/common/CommonDefine.h>
#include <ra/app/RaApp.h>
#include <ra/util/Util.h>
#include <ra/fetcher/MetricTreeUtil.h>

using namespace std;
RA_USE_NAMESPACE(app);
RA_USE_NAMESPACE(util);
RA_USE_NAMESPACE(config);
RA_USE_NAMESPACE(process);
RA_USE_NAMESPACE(fetcher);

RA_BEGIN_NAMESPACE(tree);

TreeManager::TreeManager() 
    : _running(false)
    , _lastReloadTreeTimeUs(0)
    , _lastTreeHashValue(INVALID_HASH_VALUE)
    , _app(NULL)
    , _fetcherManager(NULL)
    , _configVersion(INVALID_CONFIG_VERSION)
    , _reloadTreeIntervalSec(TREE_MANAGER_RELOAD_TREE_INTERVAL_S)
    , _maxRandomLastRetriveTimeValueSec(DEFAULT_LAST_RETRIEVE_TIME_RANDOM_VALUE)
{
    
}

TreeManager::~TreeManager() { 
    stop();
}

bool TreeManager::init(RaApp *app, FetcherManager* fetcherManager,
                       int64_t reloadTreeIntervalSec, 
                       int32_t maxRandomLastRetriveTimeValueSec)
{
    if (NULL == app) {
        LOG(ERROR) << "app ptr is NULL, init tree manager failed";
        return false;
    }
    if (NULL == fetcherManager) {
        LOG(ERROR) << "fetcher manager ptr is NULL, init tree manager failed";
        return false;
    }

    if (reloadTreeIntervalSec <= 0) {
        LOG(ERROR) << "reloadTreeIntervalSec must be bigger than 0, "
	    "init tree manager failed";
        return false;
    }
    _maxRandomLastRetriveTimeValueSec = maxRandomLastRetriveTimeValueSec;
    _reloadTreeIntervalSec = reloadTreeIntervalSec;
    _app = app;
    _fetcherManager = fetcherManager;

    
    LOG(INFO) << "Tree Manager inited success: reloadTreeIntervalSec:" << _reloadTreeIntervalSec
	      << ",maxRandomLastRetriveTimeValueSec:" 
	      << _maxRandomLastRetriveTimeValueSec;
    return true;
}

bool TreeManager::start()
{
    if (NULL == _app) {
        LOG(ERROR) << "can not start tree manager thread whithout inited";
        return false;
    }
    if (_running) {
        LOG(ERROR) << "can not start tree manager thread twice";
        return false;
    }
    int err = pthread_create(&_thread, NULL, &TreeManager::workLoop, this);
    if (err != 0) {
        LOG(ERROR) << "create tree manager thread failed";
        return false;
    }
    _running = true;
    LOG(INFO) << "Tree manager start success!";
    return true;
}

void TreeManager::stop()
{
    if (!_running) return;
    _running = false;
    pthread_join(_thread, NULL);
    LOG(INFO) << "tree manager thread stopped";
}

void TreeManager::clear()
{
    ProcessPackageMapPtr ProcessPackageMapPtr(new ProcessPackageMap());
    setProcessPackageMap(ProcessPackageMapPtr);
}

void TreeManager::updateState(int32_t configVersion, int64_t curTimeUs,
                              uint64_t hashValue)
{
    _configVersion = configVersion;
    _lastReloadTreeTimeUs = curTimeUs;
    _lastTreeHashValue = hashValue;
}

void* TreeManager::workLoop(void *arg)
{
    TreeManager *mananger = (TreeManager *)arg;
    int64_t lastRuntime = 0;
    int64_t currentTime = Util::currentTimeInMicroseconds();
    while (mananger->_running) {
        int64_t elapsedTime = currentTime - lastRuntime;
        if (elapsedTime < TREE_MANAGER_LOOP_INTERVAL_US) {
            int64_t sleepTime = TREE_MANAGER_LOOP_INTERVAL_US - max((int64_t)0, elapsedTime);
            usleep(sleepTime);
        }
        lastRuntime = Util::currentTimeInMicroseconds();
        mananger->doWorkLoop(currentTime);
    }
    return NULL;
}

bool TreeManager::doWorkLoop(int64_t curTimeUs)
{
    if (!needReloadTree(curTimeUs)) {
        return false;
    }

    // get spec set
    const ConfigWrapperPtr configPtr = _app->getConfigWrapper();
    StringSet specSet;
    configPtr->getAmonSpecs(specSet);
    int64_t start = curTimeUs - RETRIEVE_TREE_TIME_START * MICROSECONDS;
    int64_t end = curTimeUs;

    // fetch latest metric tree
    MetricRootMap metricRootMap;
    if (!getMetricTree(specSet, start, end, configPtr, metricRootMap)) {
        LOG(ERROR) << "get metric tree failed, will reload tree again";
        return false;
    }

    // merge into a master tree
    MetricNodePtr masterTree;
    MetricRootMap::iterator iter;
    for (iter = metricRootMap.begin(); iter != metricRootMap.end(); iter++) {
        masterTree = MetricTreeUtil::mergeTree(masterTree, iter->second);
    }
    setMasterMetricTree(masterTree);
    LOG(INFO) << "fetch and construct the latest master tree from " << metricRootMap.size() << " roots";

    // check if there is any policy
    int32_t configVersion = configPtr->getConfigVersion();
    if (configPtr->isEmptyConfig()) {
        clear();
        updateState(configVersion, curTimeUs, INVALID_HASH_VALUE);
        LOG(WARNING) << "empty policy config or not used server, "
	    "this server will do nothing";
        return true;
    }

    // create metric tree
    MetricTree tree;
    mergeTree(metricRootMap, tree);

    if (0 == tree.getTreeSize()) {
        clear();
        updateState(configVersion, curTimeUs, INVALID_HASH_VALUE);
        LOG(ERROR) << "fatal, tree is empty, please check amonitor config";
        return true;
    }

    //filter tree
    int32_t eraseCount = 0;
    int32_t keepCount = 0;
    tree.filterTree(std::tr1::bind(&ConfigWrapper::isMonitored, 
            configPtr.get(), std::tr1::placeholders::_1), eraseCount, keepCount);
    //estimate whether hash values is same
    uint64_t hashValue = tree.getHashValue();

    if (_configVersion == configVersion
        && _lastTreeHashValue != INVALID_HASH_VALUE
        && _lastTreeHashValue == hashValue)
    {
        updateState(configVersion, curTimeUs, hashValue);
        LOG(INFO) << "new tree hash value not changed, not generate packages";
        return false;
    }

    //construct request Package
    std::vector<RequestPackagePtr> requestPackageVec;
    genRequestPackages(configPtr, tree, requestPackageVec);
    
    //construct process package
    ProcessPackageMapPtr processPackageMapPtr(new ProcessPackageMap());
    genProcessPackages(requestPackageVec, *processPackageMapPtr);

    //update state
    updateState(configVersion, curTimeUs, hashValue);
    setProcessPackageMap(processPackageMapPtr);

    LOG(INFO) << "reload tree and reconstruct packages success, request packages: "
	      << requestPackageVec.size() << ", process packages: " << processPackageMapPtr->size();
    return true;
}

bool TreeManager::getMetricTree(
    const StringSet& specSet, int64_t start, int64_t end,
    const ConfigWrapperPtr& configPtr,
    MetricRootMap &metricRootMap) const
{
    StringSet::const_iterator it;
    for(it = specSet.begin(); it != specSet.end(); ++it) {
        const string& spec = *it;
        MetricNodePtr metricRoot;
        if (!_fetcherManager->getMetricTree(spec, start, end, configPtr, metricRoot)) {
            return false;
        }
        metricRootMap[spec] = metricRoot;
    }
    return true;
}

void TreeManager::mergeTree(const MetricRootMap &metricRootMap, MetricTree& tree) {
    MetricRootMap::const_iterator it;
    for (it = metricRootMap.begin(); it != metricRootMap.end(); it++) {
        const string& spec = it->first;
        const MetricNodePtr& root = it->second;
        MetricTreeUtil::traverseTree(root, tr1::bind(&TreeManager::mergePath, this, tr1::placeholders::_1, spec, tr1::ref(tree)));
    }
}

bool TreeManager::mergePath(const fetcher::MetricPath &path, const string &spec, MetricTree& tree) {
    tree.addPath(path, spec);
    return true;
}

bool TreeManager::needReloadTree(int64_t curTimeUs) const
{
    const ConfigWrapperPtr configPtr = _app->getConfigWrapper();
    if (NULL == configPtr) {
        return false;
    }
    int32_t appConfigVersion = configPtr->getConfigVersion();
    if (appConfigVersion != _configVersion || 
        curTimeUs - _lastReloadTreeTimeUs > _reloadTreeIntervalSec * MICROSECONDS)
    {
        return true;
    }
    return false;
}

void TreeManager::genRequestPackages(const ConfigWrapperPtr& configPtr, 
                                     const MetricTree& tree,
                                     vector<RequestPackagePtr>& requestPackageVec) const
{
    vector<PolicyConfigItemBasePtr> policyItemVec;
    configPtr->getPolicyItems(policyItemVec);
    vector<PolicyConfigItemBasePtr>::const_iterator it = policyItemVec.begin();
    for (; it != policyItemVec.end(); ++it) {
        const PolicyConfigItemBasePtr& policyItemPtr = *it;
        RequestPackagePtr requestPackage = genOneRequestPackage(
            configPtr, tree, policyItemPtr);
        if (NULL != requestPackage) {
            requestPackageVec.push_back(requestPackage);
        }
    }
}

RequestPackagePtr TreeManager::genOneRequestPackage(
        const ConfigWrapperPtr& configPtr,
        const MetricTree& tree,
        const PolicyConfigItemBasePtr& policyItemPtr) const
{
    StringSet metricSet;
    const string& metricPattern = policyItemPtr->getMetric();
    tree.getMetrics(metricPattern, metricSet);

    //filter shield metrics
    StringSet::iterator metricIter = metricSet.begin();
    while (metricIter != metricSet.end()) {
        const string& metric = *metricIter;
        if (policyItemPtr->isFiltered(metric)) {
            metricSet.erase(metricIter++);
        }
        else {
            ++metricIter;
        }
    }

    //assign metrics to amon spec
    Str2Set specMetricSetMap;
    for (metricIter = metricSet.begin(); metricIter != metricSet.end();
         ++metricIter)
    {
        const string& metric = *metricIter;
        const StringSet& specSet = tree.getSpecs(metric);
        StringSet::const_iterator specIter = specSet.begin();
        for (; specIter != specSet.end(); ++specIter) {
            const string& spec = *specIter;
            specMetricSetMap[spec].insert(metric);
        }
    }
    
    RequestPackagePtr requestPackage(new RequestPackage(configPtr, policyItemPtr));
    requestPackage->swapMetricSet(metricSet);

    //gen retrieveRequest for each amon spec
    Str2Set::const_iterator it = specMetricSetMap.begin();
    for (; it != specMetricSetMap.end(); ++it) {
        const string& spec = it->first;
        const StringSet& metricSet = it->second;
        MetricNodePtr root = genMetricSubtree(tree, metricSet);
        if (root != NULL) {
            requestPackage->updateMetricRootMap(spec, root);
        }
    }
    return requestPackage;
}

MetricNodePtr TreeManager::genMetricSubtree(const MetricTree& tree, const StringSet& metricSet) const {
    MetricNodePtr root;
    StringSet::iterator it;
    for (it = metricSet.begin(); it != metricSet.end(); ++it) {
        const string& metric = *it;
        MetricPath metricPath;
        if (!tree.getMericPath(metric, metricPath)) {
            LOG(WARNING) << "Cannot get metric path " << metric;
            continue;
        }
        if (!MetricTreeUtil::insertPath(root, metricPath)) {
            LOG(WARNING) << "Cannot insert metric path " << metric;
            continue;
        }
    }
    return root;
}

void TreeManager::genProcessPackages(
    const vector<RequestPackagePtr>& requestPackageVec,
    ProcessPackageMap& processPackageMap) const
{
    processPackageMap.clear();
    vector<RequestPackagePtr>::const_iterator it = requestPackageVec.begin();
    for (; it != requestPackageVec.end(); ++it) {
        const RequestPackagePtr& requestPackagePtr = *it;
        assert(NULL != requestPackagePtr);
        ProcessPackagePtr processPackagePtr(new ProcessPackage(requestPackagePtr));
        const ProcessPackageKey& key = processPackagePtr->getKey();

        TrendDataPtr trendData;
        const ProcessPackageMapPtr& processPackageMapPtr = getProcessPackageMap();
        int64_t curTimeUs = Util::currentTimeInMicroseconds();
        if (NULL != processPackageMapPtr && 
            processPackageMapPtr->find(key) != processPackageMapPtr->end()) 
        {
            const ProcessPackageMap& processPackageMap = *processPackageMapPtr;
            const ProcessPackagePtr& package = processPackageMap.find(key)->second;
            assert(NULL != package);
            int64_t lastRetrieveTimeUs = package->getLastRetrieveTimeUs();
            processPackagePtr->setLastRetrieveTimeUs(lastRetrieveTimeUs);
            int64_t lastNonOkAlarmTimeSec = package->getLastNonOkAlarmTimeSec();
            processPackagePtr->setLastNonOkAlarmTimeSec(lastNonOkAlarmTimeSec);
            trendData = package->getTrendData();
            assert(NULL != trendData);
            trendData->clearExpiredData(curTimeUs);
        }
        else {
            processPackagePtr->setRandomLastRetrieveTime(curTimeUs, 
                    _maxRandomLastRetriveTimeValueSec);
            trendData.reset(new TrendData());
        }

        const ConfigWrapperPtr& config = requestPackagePtr->getConfig();
        assert(NULL != config);
        double smoothingFactor = config->getSmoothingFactor();
        double trendFactor = config->getTrendFactor();
        trendData->setFactors(smoothingFactor, trendFactor);

        const PolicyConfigItemBasePtr& policyItem =
            requestPackagePtr->getPolicyItem();
        assert(NULL != policyItem);
        int32_t fetchInterval = policyItem->getFetchInterval();
        uint64_t trendAutoExpireTimeUs =
            MICROSECONDS * fetchInterval * TREND_AUTO_EXPIRE_TIMES;
        trendData->setAutoExpireTimeUs(trendAutoExpireTimeUs);
        processPackagePtr->setTrendData(trendData);
        if (PT_TREND == requestPackagePtr->getTriggerType()) {
            LOG(INFO) << "id:" << key.id <<", metric:"
		      << key.metric <<", set smoothingFactor:"
		      << smoothingFactor << ", trendFactor:" 
		      << trendFactor <<", autoExpireTimeUs:" << trendAutoExpireTimeUs;
        }
        processPackageMap[key] = processPackagePtr;
    }
}

RA_END_NAMESPACE(tree);
