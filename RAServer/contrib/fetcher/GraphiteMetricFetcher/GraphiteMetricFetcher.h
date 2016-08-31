#ifndef RA_GRAPHITEMETRICFETCHER_H
#define RA_GRAPHITEMETRICFETCHER_H

#include <ra/common/Common.h>
#include <ra/common/Log.h>
#include <ra/common/HttpClient.h>
#include <ra/fetcher/MetricFetcher.h> 

RA_BEGIN_NAMESPACE(fetcher);

class GraphiteMetricFetcher : public MetricFetcher
{
public:
    GraphiteMetricFetcher();
    ~GraphiteMetricFetcher();

private:
    GraphiteMetricFetcher(const GraphiteMetricFetcher &);
    GraphiteMetricFetcher& operator=(const GraphiteMetricFetcher &);

public:
    virtual bool init(const OptionMap &options);
    virtual void close();
    virtual MetricNodePtr allocTree(int64_t start, int64_t end);
    virtual MetricNodePtr retrieve(
            const MetricNodePtr &root, int64_t start, int64_t end, int64_t step);
private:
    void getAllMetricsURL(std::string &url);
    void getMetricsURL(const std::string &metricPath, std::string &url);
    void getDataURL(const std::string &metricPath, int64_t start, int64_t end, std::string &url);
    bool getSubMetrics(const std::string &parentMetricPath, std::map<std::string, int> &subMetrics);
    bool getAllMetrics(std::set<std::string> &metrics);

    bool getData(const std::string &metricPath, int64_t start, int64_t end, std::map<uint64_t, double> &data);
    static bool makeTree(const std::set<std::string> &metrics, MetricNodePtr &metricNode);
    bool batchAllocTree(MetricNodePtr &metricNode);
    bool traverse(const std::string &parent, MetricNodePtr &metricNode);
    bool readGraphiteData(const MetricPath& path, int64_t start, int64_t end, int64_t step);

    static bool parseSubMetrics(const std::string &content, std::map<std::string, int> &subMetrics);
    static bool parseSubMetricData(const std::string &content, std::map<uint64_t, double> &metricData, const std::string &metricPath);
    static bool parseAllMetrics(const std::string &content, std::set<std::string> &metrics);
private:
    std::string _accessHost;//domain or host:ip, for example:http://127.0.0.1:8080/
    common::HttpClient _httpClient;
    bool _isBatchAllocTree;
private:
    friend class GraphiteMetricFetcherTest;
};

RA_TYPEDEF_PTR(GraphiteMetricFetcher);

RA_END_NAMESPACE(fetcher);

#endif //RA_GRAPHITEMETRICFETCHER_H
