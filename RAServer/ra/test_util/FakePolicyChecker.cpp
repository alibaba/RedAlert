#include <ra/test_util/FakePolicyChecker.h>

using namespace std;
RA_USE_NAMESPACE(tree);
RA_USE_NAMESPACE(alarm);
RA_USE_NAMESPACE(process);

RA_BEGIN_NAMESPACE(test_util);
RA_LOG_SETUP(test_util, FakePolicyChecker);

FakePolicyChecker::FakePolicyChecker() 
{
    checkSingleHostCount = 0;
    checkHostTotalCount = 0;
    checkMetricTotalCount = 0;
    totalCheckCount = 0;
}

FakePolicyChecker::~FakePolicyChecker() { 
}

bool FakePolicyChecker::init(const ProcessPackagePtr& processPackage)
{
    if (!PolicyChecker::init(processPackage)) {
        return false;
    }
    _alarmMsg.reset(new AlarmMsg(processPackage, CODE_CRITICAL));
    return true;
}

void FakePolicyChecker::checkSingleHost(const vector<NormalizedMetricDataPtr> &metricDataVec) 
{
    ++checkSingleHostCount;
    ++totalCheckCount;
    PolicyChecker::checkSingleHost(metricDataVec);
}
    
bool FakePolicyChecker::checkHostVal(const string& metric, const MetricDataItem& hostData,
        const vector<NormalizedMetricDataPtr> &metricDataVec) 
{
    checkHostValueVec.push_back(hostData);
    return true;
};

void FakePolicyChecker::checkHostTotal(vector<NormalizedMetricDataPtr> &metricDataVec) {
    ++checkHostTotalCount;
    ++totalCheckCount;
    PolicyChecker::checkHostTotal(metricDataVec);
}
bool FakePolicyChecker::checkHostTotalVal(const string& metric, const MetricDataItem& hostTotalData,
                       const vector<NormalizedMetricDataPtr> &metricDataVec)
{
    checkHostTotaltValueVec.push_back(hostTotalData);
    return true;
}
    
void FakePolicyChecker::checkMetricTotal(vector<NormalizedMetricDataPtr> &metricDataVec)
{
    ++checkMetricTotalCount;
    ++totalCheckCount;
    PolicyChecker::checkMetricTotal(metricDataVec);
}
bool FakePolicyChecker::checkMetricTotalVal(const MetricDataItem& metricTotalData,
                         const vector<NormalizedMetricDataPtr> &metricDataVec) 
{
    checkMetricTotaltValueVec.push_back(metricTotalData);
    return true;
};


RA_END_NAMESPACE(test_util);

