#include <ra/test_util/TestUtil.h>
#include <ra/config/ThresholdPolicyItem.h>
#include <ra/config/DeviationPolicyItem.h>
#include <ra/config/AvailabilityPolicyItem.h>
#include <ra/config/TrendPolicyItem.h>
#include <ra/config/SingularityPolicyItem.h>
#include <ra/config/CyclePolicyItem.h>

using namespace std;
RA_USE_NAMESPACE(config);

RA_BEGIN_NAMESPACE(test_util);

TestUtil::TestUtil() { 
}

TestUtil::~TestUtil() { 
}

PolicyConfigItemBasePtr TestUtil::generatePolicyItem(const std::string& metric, 
        int32_t fetchInterval, int32_t timeRangeCount, int32_t id, PolicyTrigger pt)
{
    PolicyConfigItemBasePtr policyItem;
    switch (pt) {
    case PT_THRESHOLD: {
        policyItem.reset(new ThresholdPolicyItem());
        break;
    }
    case PT_TREND: {
        policyItem.reset(new TrendPolicyItem());
        break;
    }
    case PT_SINGULARITY: {
        policyItem.reset(new SingularityPolicyItem());
        break;
    }
    case PT_CYCLE: {
        policyItem.reset(new CyclePolicyItem());
        break;
    }
    case PT_AVAILABILITY: {
        policyItem.reset(new AvailabilityPolicyItem());
        break;
    }
    default:
        policyItem.reset(new PolicyConfigItemBase());
    }
    policyItem->setId(id);
    policyItem->setFetchInterval(fetchInterval);
    policyItem->setMetric(metric);
    policyItem->setCheckType(CT_SINGLE);
    for (int32_t i = 0; i < timeRangeCount; ++i) {
        RetrieveTimeRange retrieveTimeRange;
        int32_t cycleTime = - i * 100;
        retrieveTimeRange.start = cycleTime - fetchInterval;
        retrieveTimeRange.end = cycleTime;
        policyItem->_retrieveTimeRangeVec.push_back(retrieveTimeRange);
    }
    return policyItem;
}


RA_END_NAMESPACE(test_util);

