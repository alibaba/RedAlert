#include <ra/process/TrendData.h>
#include <ra/common/CommonDefine.h>

using namespace std;

RA_BEGIN_NAMESPACE(process);
RA_USE_NAMESPACE(util);
RA_LOG_SETUP(process, TrendData);

TrendData::TrendData()
    : _autoExpireTimeUs(MAX_TREND_HISTORY_MS)
    , _smoothingFactor(0.45)
    , _trendFactor(0.2)
{
}

TrendData::~TrendData() { 
}

void TrendData::setFactors(double smoothingFactor, double trendFactor)
{
    ScopedLock lock(_mutex);
    _smoothingFactor = smoothingFactor;
    _trendFactor = trendFactor;
}

void TrendData::setAutoExpireTimeUs(uint64_t autoExpireTimeUs)
{
    ScopedLock lock(_mutex);
    _autoExpireTimeUs = min(MAX_TREND_HISTORY_MS, autoExpireTimeUs);
}

bool TrendData::getForecastVal(const TrendKey& k, uint64_t curTime, 
        double& forecastVal)
{
    ScopedLock lock(_mutex);
    map<TrendKey, TrendMeta>::iterator it = _data.find(k);
    if (it == _data.end()) {
        RA_LOG(INFO, "has no trend data of metric:%s, host:%u",
               k.metric.c_str(), k.host);
        return false;
    }
    TrendMeta& meta = it->second;
    if (!meta.init || 0 == meta.lastInterval) {
        RA_LOG(INFO, "trend data of metric:%s, host:%u is not inited",
               k.metric.c_str(), k.host);
        return false;
    }
    if (curTime < meta.lastTime) {
        RA_LOG(WARN, "metric:%s, host:%u, trendLastTime:%"PRIu64
               " bigger than curTime:%"PRIu64, 
               k.metric.c_str(), k.host, meta.lastTime, curTime);
        forecastVal = meta.smoothedValue;
        return true;
    }
    if (curTime - meta.lastTime > _autoExpireTimeUs) {
        RA_LOG(WARN, "metric:%s, host:%u, curTime:%"PRIu64
               " - trendLastTime:%"PRIu64" < autoExpirTime:%"PRIu64
               ", trend meta reseted!", k.metric.c_str(), k.host,
               curTime, meta.lastTime, _autoExpireTimeUs);
        meta.reset();
        return false;
    }
    uint64_t step = (curTime - meta.lastTime) / meta.lastInterval;
    forecastVal = meta.smoothedValue + meta.trendValue * step;
    return true;
}

void TrendData::feedTrend(
    const TrendKey& k, uint64_t time, double value) 
{
    ScopedLock lock(_mutex);
    TrendMeta& meta = _data[k];
    assert(INVALID_METRIC_DATA != value);
    double lastSmoothedValue = meta.init ? meta.smoothedValue : value;
    meta.init = true;

    double lastTrend = meta.trendValue;
    meta.smoothedValue = _smoothingFactor * value +
                         (1 - _smoothingFactor) * (lastSmoothedValue + lastTrend);
    meta.trendValue = _trendFactor * (meta.smoothedValue - lastSmoothedValue)
                      + (1 - _trendFactor) * lastTrend;
    meta.lastInterval = time - meta.lastTime;
    meta.lastTime = time;
}

void TrendData::clearExpiredData(uint64_t curTimeUs) 
{
    ScopedLock lock(_mutex);
    map<TrendKey, TrendMeta>::iterator it = _data.begin();
    while (it != _data.end()) {
        TrendMeta& meta = it->second;
        if (!meta.init || 
            (curTimeUs > meta.lastTime &&
             curTimeUs - meta.lastTime > _autoExpireTimeUs))
        {
            _data.erase(it++);
        }
        else {
            ++it;
        }
    }
}


RA_END_NAMESPACE(process);

