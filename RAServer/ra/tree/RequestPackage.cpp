#include <ra/tree/RequestPackage.h>
#include <ra/config/ConfigWrapper.h>

using namespace std;
RA_USE_NAMESPACE(config);
RA_USE_NAMESPACE(fetcher);

RA_BEGIN_NAMESPACE(tree);

RequestPackage::RequestPackage(const ConfigWrapperPtr& config, 
        const PolicyConfigItemBasePtr& policyItem)
    : _config(config)
    , _policyItem(policyItem)
{ 
}

RequestPackage::~RequestPackage() { 
}

MetricNodePtr RequestPackage::getMetricRoot(const string& spec) const
{
    MetricRootMap::const_iterator it = _metricRootMap.find(spec);
    if (it == _metricRootMap.end()) {
        return MetricNodePtr();
    }
    return it->second;
}

void RequestPackage::getSpecs(StringSet& specSet) const
{
    specSet.clear();
    MetricRootMap::const_iterator it = _metricRootMap.begin();
    for(; it != _metricRootMap.end(); ++it) {
        const string& spec = it->first;
        specSet.insert(spec);
    }
}

void RequestPackage::getRetrieveTimeRange(vector<RetrieveTimeRange>& retrieveTimeRangeVec) const
{
    if (NULL == _policyItem){
        LOG(ERROR) << "policyItem is NULL, can not get getRetrieveTimeRange";
        retrieveTimeRangeVec.clear();
    }
    else {
        _policyItem->getRetrieveTimeRange(retrieveTimeRangeVec);
    }
}

RA_END_NAMESPACE(tree);

