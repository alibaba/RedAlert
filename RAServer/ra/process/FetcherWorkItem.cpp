#include <ra/util/Util.h>
#include <ra/process/FetcherWorkItem.h>
#include <ra/alarm/AlarmManager.h>

RA_USE_NAMESPACE(util);
RA_USE_NAMESPACE(tree);
RA_USE_NAMESPACE(alarm);
RA_USE_NAMESPACE(config);

RA_BEGIN_NAMESPACE(process);
RA_LOG_SETUP(process, FetcherWorkItem);

FetcherWorkItem::FetcherWorkItem(
    FetcherManager* fetcherManager,
    ProcessPackagePtr& processPackage,
    CheckerManager *checkerManager,
    AlarmManager* alarmManager)
    : _processPackage(processPackage)
    , _fetcherManager(fetcherManager)
    , _checkerManager(checkerManager)
    , _alarmManager(alarmManager)
{
}

FetcherWorkItem::~FetcherWorkItem() 
{
}

void FetcherWorkItem::process()
{
    int64_t curTimeUs = Util::currentTimeInMicroseconds();
    if (!_fetcherManager->retrieveMetrics(_processPackage, curTimeUs)) {
        const RequestPackagePtr& requestPackage = _processPackage->getRequestPackage();
        assert(NULL != requestPackage);
        const PolicyConfigItemBasePtr& policyConfigItemBase = requestPackage->getPolicyItem();
        assert(NULL != policyConfigItemBase);
        RA_LOG(ERROR, "retrieve metrics failed, metric:%s, id:%u",
               policyConfigItemBase->getMetric().c_str(), policyConfigItemBase->getId());
        //update state
        _processPackage->setState(ProcessPackage::PPS_IDLE);
    }
    else {
        //set last retrieve time
        _processPackage->setLastRetrieveTimeUs(curTimeUs);
        assert(NULL != _checkerManager);
        if (!_checkerManager->pushProcessPackage(_processPackage)) {
            RA_LOG(ERROR, "push process package to check pool failed");
            //update state
            _processPackage->setState(ProcessPackage::PPS_IDLE);
        }
    }
}

RA_END_NAMESPACE(process);

