#ifndef RA_TRENDDATA_H
#define RA_TRENDDATA_H

#include <ra/common/Common.h>
#include <ra/common/Log.h>
#include <ra/util/ThreadLock.h>

RA_BEGIN_NAMESPACE(process);

struct TrendKey
{
    uint32_t host;
    std::string metric;
    bool operator < (const TrendKey& key) const {
        if (host != key.host) {
            return host < key.host;
        }
        return metric < key.metric;
    }
};

class TrendData
{
public:
    TrendData();
    ~TrendData();
private:
    TrendData(const TrendData &);
    TrendData& operator = (const TrendData &);

private:
    struct TrendMeta {
        double smoothedValue;
        double trendValue;
        uint64_t lastTime;
        uint64_t lastInterval;
        bool init;

        TrendMeta() {
            reset();
        }

        void reset() {
            smoothedValue = 0.0;
            trendValue = 0.0;
            lastTime = 0;
            lastInterval = 0;
            init = false;
        }
    };

public:
    double getSmoothingFactor() const {
        util::ScopedLock lock(_mutex);
        return _smoothingFactor;
    }
    double getTrendFactor() const {
        util::ScopedLock lock(_mutex);
        return _trendFactor;
    }
    uint64_t getAutoExpireTimeUs() const {
        util::ScopedLock lock(_mutex);
        return _autoExpireTimeUs;
    }
    size_t getSize() const {
        util::ScopedLock lock(_mutex);
        return _data.size();
    }
    void setFactors(double smoothingFactor, double trendFactor);
    void setAutoExpireTimeUs(uint64_t autoExpireTimeUs);
    bool getForecastVal(const TrendKey& k, uint64_t curTime, 
                        double& forecastVal);

    void feedTrend(const TrendKey& k, uint64_t time, double value);

    void clearExpiredData(uint64_t curTimeUs);

private:
    std::map<TrendKey, TrendMeta> _data;
    mutable util::Mutex _mutex;

    uint64_t _autoExpireTimeUs;
    double _smoothingFactor;
    double _trendFactor;

private:
    friend class TrendDataTest;

private:
    RA_LOG_DECLARE();
};

RA_TYPEDEF_PTR(TrendData);

RA_END_NAMESPACE(process);

#endif //RA_TRENDDATA_H
