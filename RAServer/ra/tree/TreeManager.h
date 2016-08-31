#ifndef RA_TREEMANAGER_H
#define RA_TREEMANAGER_H

#include <pthread.h>
#include <ra/common/Common.h>
#include <ra/common/Log.h>
#include <ra/util/ThreadLock.h>
#include <ra/tree/MetricTree.h>
#include <ra/tree/RequestPackage.h>
#include <ra/tree/ProcessPackage.h>
#include <ra/process/FetcherManager.h>
#include <ra/fetcher/MetricFetcher.h>

RA_BEGIN_NAMESPACE(app);
class RaApp;
RA_END_NAMESPACE(app);

RA_BEGIN_NAMESPACE(service);
class ListMetricHandlerTest;
RA_END_NAMESPACE(service);

RA_BEGIN_NAMESPACE(tree);

class TreeManager
{
public:
    TreeManager();
    ~TreeManager();
private:
    TreeManager(const TreeManager &);
    TreeManager& operator=(const TreeManager &);

public:
    typedef std::map<ProcessPackageKey, ProcessPackagePtr> ProcessPackageMap;
    RA_TYPEDEF_PTR(ProcessPackageMap);

public:
    bool init(app::RaApp *app, process::FetcherManager* fetcherManager,
              int64_t reloadTreeIntervalSec, int32_t maxRandomLastRetriveTimeValueSec);
    bool start();
    void stop();

    ProcessPackageMapPtr getProcessPackageMap() const {
        util::ScopedLock lock(_lockPackageMap);
        return _processPackageMap;
    }

    fetcher::MetricNodePtr getMasterMetricTree() const {
        util::ScopedLock lock(_lockMasterTree);
        return _masterMetricTree;
    }

private:
    typedef std::map<std::string, StringSet> Str2Set;

private:

    bool getMetricTree(const StringSet& specSet, int64_t start, int64_t end,
                       const config::ConfigWrapperPtr& configPtr,
                       fetcher::MetricRootMap &metricRootMap) const;

    void mergeTree(const fetcher::MetricRootMap &metricRootMap, MetricTree& tree);
    bool mergePath(const fetcher::MetricPath &path, const std::string &spec, MetricTree& tree);

    void genProcessPackages(
            const std::vector<RequestPackagePtr>& requestPackageVec,
            ProcessPackageMap& processPackageMap) const;

    void genRequestPackages(const config::ConfigWrapperPtr& configPtr, 
                            const MetricTree& tree,
                            std::vector<RequestPackagePtr>& requestPackageVec) const;
    
    RequestPackagePtr genOneRequestPackage(
            const config::ConfigWrapperPtr& configPtr, 
            const MetricTree& tree,
            const config::PolicyConfigItemBasePtr& policyItemPtr) const;

    fetcher::MetricNodePtr genMetricSubtree(
            const MetricTree& tree,
            const StringSet& metricSet) const;

    bool needReloadTree(int64_t curTimeUs) const;
    
    void setProcessPackageMap(const ProcessPackageMapPtr& processPackageMapPtr) {
        util::ScopedLock lock(_lockPackageMap);
        _processPackageMap = processPackageMapPtr;
    }

    void setMasterMetricTree(const fetcher::MetricNodePtr& masterMetricTree) {
        util::ScopedLock lock(_lockMasterTree);
        _masterMetricTree = masterMetricTree;
    }

    static void* workLoop(void *arg);
    bool doWorkLoop(int64_t curTimeUs);

    void updateState(int32_t configVersion, int64_t curTimeUs,
                     uint64_t hashValue);
    void clear();

private:
    pthread_t _thread;
    bool _running;
    int64_t _lastReloadTreeTimeUs;
    uint64_t _lastTreeHashValue;

    app::RaApp *_app;
    process::FetcherManager *_fetcherManager;
    int32_t _configVersion;
    int64_t _reloadTreeIntervalSec;
    int32_t _maxRandomLastRetriveTimeValueSec;

    mutable util::Mutex _lockPackageMap;
    ProcessPackageMapPtr _processPackageMap;
    mutable util::Mutex _lockMasterTree;
    fetcher::MetricNodePtr _masterMetricTree;

private:
    friend class TreeManagerTest;
    friend class service::ListMetricHandlerTest;
};

RA_TYPEDEF_PTR(TreeManager);

RA_END_NAMESPACE(tree);

#endif //RA_TREEMANAGER_H
