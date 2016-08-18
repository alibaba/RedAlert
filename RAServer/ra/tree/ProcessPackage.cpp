#include <stdlib.h>
#include <ra/util/Util.h>
#include <ra/tree/ProcessPackage.h>
#include <ra/config/PolicyConfigItemBase.h>

using namespace std;

RA_BEGIN_NAMESPACE(tree);
RA_LOG_SETUP(tree, ProcessPackage);
RA_USE_NAMESPACE(util);

ProcessPackage::ProcessPackage(const RequestPackagePtr& requestPackage)
    : _requestPackage(requestPackage)
    , _lastRetrieveTimeUs(0)
    , _state(PPS_IDLE)
    , _lastNonOkAlarmTimeSec(INVALID_TIME)
{

}

ProcessPackage::~ProcessPackage() { 
}

void ProcessPackage::setRandomLastRetrieveTime(int64_t curTimeUs, 
        int32_t maxRandomLastRetriveTimeValueSec)
{
    assert(NULL != _requestPackage);
    const config::PolicyConfigItemBasePtr& policyItem =
        _requestPackage->getPolicyItem();
    assert(NULL != policyItem);
    int64_t randomMaxVal = MICROSECONDS * min(policyItem->getFetchInterval(),
            maxRandomLastRetriveTimeValueSec);
    
    if (randomMaxVal <= 0) {
        setLastRetrieveTimeUs(0);
        RA_LOG(INFO, "set random last retrive time(us):0");
    }
    else {
        int64_t currentTime = Util::currentTimeInMicroseconds();
        srand(currentTime);
        int64_t lastRetrieveTimeUs = curTimeUs - rand() % randomMaxVal + 1;
        setLastRetrieveTimeUs(lastRetrieveTimeUs);
        RA_LOG(INFO, "set random last retrive time(us):%"PRId64, lastRetrieveTimeUs);
    }
}

ProcessPackageKey ProcessPackage::getKey() const
{
    ProcessPackageKey key;
    assert(NULL != _requestPackage);
    const config::PolicyConfigItemBasePtr& policyItem =
        _requestPackage->getPolicyItem();
    assert(NULL != policyItem);
    key.id = policyItem->getId();
    key.metric = policyItem->getMetric();
    return key;
}

bool ProcessPackage::isReady(int64_t curTimeUs) const
{
    if (getState() != PPS_IDLE) {
        return false;
    }
    assert(NULL != _requestPackage);
    const config::PolicyConfigItemBasePtr& policyItem =
        _requestPackage->getPolicyItem();
    assert(NULL != policyItem);
    if (policyItem->getValidTime() * MICROSECONDS > curTimeUs) {
        return false;
    }
    assert(0 != MICROSECONDS);
    if (!policyItem->isEffective(curTimeUs / MICROSECONDS)) {
        return false;
    }
    int32_t fetchInterval = policyItem->getFetchInterval();
    if (getLastRetrieveTimeUs() + fetchInterval * MICROSECONDS > curTimeUs) {
        return false;
    }
    return true;
}

RA_END_NAMESPACE(tree);

