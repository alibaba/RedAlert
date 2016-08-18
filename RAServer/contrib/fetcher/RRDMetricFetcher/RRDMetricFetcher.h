#ifndef RA_RRDMETRICFECHER_H
#define RA_RRDMETRICFECHER_H

#include <ra/common/Common.h>
#include <ra/common/Log.h>
#include <ra/fetcher/MetricFetcher.h>

RA_BEGIN_NAMESPACE(fetcher);

class RRDMetricFetcher: public MetricFetcher
{
public:
    RRDMetricFetcher();
    virtual ~RRDMetricFetcher();

private:
    RRDMetricFetcher(const RRDMetricFetcher &);
    RRDMetricFetcher& operator=(const RRDMetricFetcher &);

public:
    virtual bool init(const OptionMap &options);
    virtual void close();
    virtual MetricNodePtr allocTree(int64_t start, int64_t end);
    virtual MetricNodePtr retrieve(
            const MetricNodePtr &root, int64_t start, int64_t end, int64_t step);
    
private:
    static void compareAndAddChild(MetricNodePtr &parent, const std::string &label);
    static bool getMetricName(const std::string &metricFileName, std::string &metricName);
    bool readRRDData(const MetricPath& path, int64_t start, int64_t end,int64_t step,
                     std::map< std::string, std::vector<std::string> >& hostNameBuf);

private:
    bool traverse(std::string &path, MetricNodePtr &root) const;
    bool getMetricValue(std::string& rrdPath, time_t start, time_t end,
                        uint64_t step, MetricDataPtr data);
private:
    std::string _rrdRootDir;
    uint32_t _triesNum;
    char startBuf[10000];
    char endBuf[10000];
private:
    friend class RRDMetricFetcherTest;
private:
    RA_LOG_DECLARE();
};

RA_TYPEDEF_PTR(RRDMetricFetcher);

RA_END_NAMESPACE(fetcher);

#endif //RA_RRDMETRICFECHER_H
