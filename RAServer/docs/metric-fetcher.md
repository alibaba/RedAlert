- [Overview](#overview)
- [Metric Fetcher API](#metric-fetcher-api)
    - [`MetricFetcher::init()`](#metricfetcherinit)
    - [`MetricFetcher::close()`](#metricfetcherclose)
    - [`MetricFetcher::allocTree()`](#metricfetcheralloctree)
    - [`MetricFetcher::retrieve()`](#metricfetcherretrieve)
    - [`MetricFetcher::attachDataNode()`](#metricfetcherattachdatanode)

## Overview ##

Red Alert uses metrics collected by Ganglia, Graphite and AMnoitor as inputs for alerting.

## Metric Fetcher API ##

Red Alert supports metric fetchers with the following interface:

```cpp
typedef std::map<std::string, std::string> OptionMap;

class MetricFetcher {
public:
    virtual bool init(const OptionMap &options) = 0;
    virtual void close() = 0;
    virtual MetricNodePtr allocTree(int64_t start = -1, int64_t end = -1) = 0;
    virtual MetricNodePtr retrieve(const MetricNodePtr &root, int64_t start, int64_t end, int64_t step);
protected:
    virtual bool attachDataNode(const MetricPath& path, int64_t start, int64_t end, int64_t step);
}
```

### `MetricFetcher::init()` ###

Red Alert initializes a fetcher by calling `init(options)`, where `options` is a string-to-string map.

### `MetricFetcher::close()` ###

Red Alert stops a fetcher by calling `close()`. The fetcher should release all the resources in
this function.

### `MetricFetcher::allocTree()` ###

Metric data collected by monitoring system usually have hierarchical structure. Red Alert organizes
metric data in tree structure. `allocTree(start, end)` returns a smart pointer of type
[`MetricNodePtr`](ra/fetcher/MetricNode.h), which is the root of a metric tree. The metric tree includes
all the metrics with data collected in time interval `[start, end]`. The returned pointer to the root node
can be used as the first argument when calling `retrieve()` to fetch metric data.

For example, this is a metric tree for a search engine system:

<img src="docs/metric-tree.png" width="600" />

### `MetricFetcher::retrieve()` ###

`MetricFetcher::retrieve(root, start, end, step)` returns an augmented tree with a new level of leaf nodes,
which are instances of [`MetricDataNode`](ra/fetcher/MetricDataNode.h). Each data node is associated
with an instance of [`MetricData`](ra/fetcher/MetricData.h) containing the metric data for a host.
The data are collected at time slice `[start, start + step, start + 2 * step, ..., end]`.

The following figure shows an augmented tree:

<img src="docs/metric-data.png" width="800" />

### `MetricFetcher::attachDataNode()` ###

The default implementation of `retrieve()` in the base class [`MetricFetcher`](ra/fetcher/MetricFetcher.h)
is to traverse all the leaf nodes and call `attachDataNode()` by providing the path from root to the leaf node
as the first argument. [`MetricPath`](ra/fetcher/MetricNode.h) consists of a vector of `MetricNodePtr`.
Each instance of `MetricNode` is uniquely identified by the string returned by `MetricNode::getLabel()`.
In most scenarios, we only need to override `attachDataNode()` in subclass of `MetricFetcher` and
retrieve the data identified by the path.
