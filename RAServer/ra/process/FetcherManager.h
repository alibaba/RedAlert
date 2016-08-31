#ifndef RA_FETCHERMANAGER_H
#define RA_FETCHERMANAGER_H

#include <ra/common/Common.h>
#include <ra/common/Log.h>
#include <ra/util/ThreadPool.h>
#include <ra/config/ConfigWrapper.h>
#include <ra/alarm/AlarmManager.h>
#include <ra/tree/ProcessPackage.h>
#include <ra/process/CheckerManager.h>
#include <ra/alarm/InternalAlarmMsg.h>
#include <ra/fetcher/MetricNode.h>

RA_BEGIN_NAMESPACE(process);

class FetcherManager
{
public:
    FetcherManager();
    virtual ~FetcherManager();
private:
    FetcherManager(const FetcherManager &);
    FetcherManager& operator=(const FetcherManager &);
public:
    bool init(alarm::AlarmManager* alarmManager, size_t threadNum, 
              size_t queueSize, const std::string& internalAlarmGroup, 
              const std::string& internalAlarmLevel, 
              int32_t interalMinAlarmInterval,
              uint32_t retrieveMetricsPointCount,
              CheckerManager* checkManager);
    bool start();
    void stop();
    bool pushProcessPackage(tree::ProcessPackagePtr& processPackage);
    size_t getQueueItemCount() const;

    virtual bool getMetricTree(
            std::string spec, int64_t start, int64_t end,
            const config::ConfigWrapperPtr &configPtr,
            fetcher::MetricNodePtr &metricRoot);
    virtual bool retrieveMetrics(
            tree::ProcessPackagePtr& processPackage,
            int64_t curTimeUs);

protected:
    virtual fetcher::MetricFetcherPtr getMetricFetcher(
        const config::ConfigWrapperPtr& configPtr, const std::string& spec);

private:
    void pushInternalAlarmMessage(const std::string& alarmMsgStr,
            const config::ConfigWrapperPtr& configPtr, alarm::MsgCode code = alarm::CODE_CRITICAL);
    bool isNeedSetAlarmCodeOk(tree::ProcessPackagePtr& processPackage);

protected:
    alarm::AlarmManager* _alarmManager;

private:
    uint32_t _retrieveMetricsPointCount;
    util::ThreadPool *_fetcherPool;
    CheckerManager *_checkManager;
    size_t _threadNum;
    size_t _queueSize;
    std::string _internalAlarmLevel;
    std::string _internalAlarmGroup;
    int32_t _interalMinAlarmInterval;
    bool _started;
    int64_t _lastNonOkAlarmTimeSec;
};

RA_TYPEDEF_PTR(FetcherManager);

RA_END_NAMESPACE(process);

#endif //RA_FETCHERMANAGER_H
