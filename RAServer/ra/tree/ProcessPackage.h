#ifndef RA_PROCESSPACKAGE_H
#define RA_PROCESSPACKAGE_H

#include <ra/common/Common.h>
#include <ra/common/Log.h>
#include <ra/util/ThreadLock.h>
#include <ra/tree/RequestPackage.h>
#include <ra/process/TrendData.h>
#include <ra/fetcher/MetricFetcher.h>

RA_BEGIN_NAMESPACE(tree);

struct ProcessPackageKey
{
    uint32_t id;
    std::string metric;

    bool operator< (const ProcessPackageKey& key) const {
        if (id != key.id) {
            return id < key.id;
        }
        return metric < key.metric;
    }
};

class ProcessPackage
{
public:
    ProcessPackage(const RequestPackagePtr& requestPackage);
    ~ProcessPackage();

private:
    ProcessPackage(const ProcessPackage &);
    ProcessPackage& operator=(const ProcessPackage &);

public:
    enum ProcessPackageState {
        PPS_IDLE = 0,
        PPS_PROCESSING,
    };

public:
    const RequestPackagePtr& getRequestPackage() const {
        return _requestPackage;
    }

    const std::vector<fetcher::MetricNodePtr>& getMetricDataRoots() const {
        util::ScopedLock lock(_mutex);
        return _metricDataRoots;
    }

    void setMetricDataRoots(const std::vector<fetcher::MetricNodePtr>& dataRoots) {
        util::ScopedLock lock(_mutex);
        _metricDataRoots.assign(dataRoots.begin(), dataRoots.end());
    }

    void setLastRetrieveTimeUs(int64_t lastRetrieveTimeUs) {
        util::ScopedLock lock(_mutex);
        _lastRetrieveTimeUs = lastRetrieveTimeUs;
    }
    int64_t getLastRetrieveTimeUs() const {
        util::ScopedLock lock(_mutex);
        return _lastRetrieveTimeUs;
    }
    void setTrendData(const process::TrendDataPtr& trendData) {
        util::ScopedLock lock(_mutex);
        _trendData = trendData;
    }
    process::TrendDataPtr getTrendData() const {
        util::ScopedLock lock(_mutex);
        return _trendData;
    }

    ProcessPackageKey getKey() const;
    void setRandomLastRetrieveTime(int64_t curTimeUs, int32_t maxRandomLastRetriveTimeValueSec);

    ProcessPackageState getState() const {
        util::ScopedLock lock(_mutex);
        return _state;
    }
    void setState(ProcessPackageState state) {
        util::ScopedLock lock(_mutex);
        _state = state;
    }
    void setLastNonOkAlarmTimeSec(int64_t currTimeSec) {
        util::ScopedLock lock(_mutex);
        _lastNonOkAlarmTimeSec = currTimeSec;
    }
    int64_t getLastNonOkAlarmTimeSec() const  {
        util::ScopedLock lock(_mutex);
        return _lastNonOkAlarmTimeSec;
    }
    bool isReady(int64_t curTimeUs) const;

private:
    friend class ProcessPackageTest;
    friend class TreeManagerTest;

private:
    const RequestPackagePtr _requestPackage;
    int64_t _lastRetrieveTimeUs;
    mutable util::Mutex _mutex;
    ProcessPackageState _state;
    process::TrendDataPtr _trendData;
    int64_t _lastNonOkAlarmTimeSec;
    std::vector<fetcher::MetricNodePtr> _metricDataRoots;
    
private:
    RA_LOG_DECLARE();
};

RA_TYPEDEF_PTR(ProcessPackage);

RA_END_NAMESPACE(tree);

#endif //RA_PROCESSPACKAGE_H
