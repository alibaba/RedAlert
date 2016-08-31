#include <ra/util/Util.h>
#include <ra/process/CheckerWorkItem.h>
#include <ra/config/PolicyConfigItemBase.h>
#include <ra/process/PolicyCheckerFactory.h>

using namespace std;
RA_USE_NAMESPACE(util);
RA_USE_NAMESPACE(tree);
RA_USE_NAMESPACE(config);
RA_USE_NAMESPACE(alarm);

RA_BEGIN_NAMESPACE(process);

CheckerWorkItem::CheckerWorkItem()
    : _alarmManager(NULL)
{
}

CheckerWorkItem::~CheckerWorkItem() 
{
}

bool CheckerWorkItem::init(AlarmManager* alarmManager, ProcessPackagePtr& processPackage)
{
    if (NULL == alarmManager) {
        LOG(ERROR) << "alarm manager is NULL, init CheckerWorkItem failed";
        return false;
    }
    if (NULL == processPackage) {
        LOG(ERROR) << "processPackage is NULL, init CheckerWorkItem failed";
        return false;
    }
    _alarmManager = alarmManager;
    _processPackage = processPackage;

    const RequestPackagePtr& requestPackage = _processPackage->getRequestPackage();
    if (NULL == requestPackage) {
        LOG(ERROR) << "requestPackage in processPackage is NULL, init CheckerWorkItem failed";
        return false;
    }
    const PolicyConfigItemBasePtr& policyConfigItemBase = requestPackage->getPolicyItem();
    if (NULL == policyConfigItemBase) {
        LOG(ERROR) << "policyConfigItem in processPackage is NULL, init CheckerWorkItem failed";
        return false;
    }

    _policyChecker = PolicyCheckerFactory::createPolicyChecker(
        policyConfigItemBase->getTriggerType());
    if (NULL == _policyChecker) {
        LOG(ERROR) << "create policyChecker failed, init CheckerWorkItem failed, metric["
		   << policyConfigItemBase->getMetric() << "], id[" 
		   << policyConfigItemBase->getId() << "]";
        return false;
    }
    if (!_policyChecker->init(_processPackage)) {
        LOG(ERROR) << "init policy checker failed failed, init CheckerWorkItem failed,"
		   << " metric[" << policyConfigItemBase->getMetric() <<"], id[" 
		   <<  policyConfigItemBase->getId() <<"]";
        return false;
    }

    return true;
}

void CheckerWorkItem::process()
{
    //processPackage state will be clear in destruction of AlarmMsg
    assert(NULL != _processPackage);
    alarm::AlarmMsgPtr alarmMsg = _policyChecker->check();
    if (alarmMsg == NULL) return;
    if (!alarmMsg->empty()) {
        assert(NULL != _alarmManager);
        if (!_alarmManager->pushAlarmMessage(alarmMsg)) {
            const RequestPackagePtr& requestPackage = _processPackage->getRequestPackage();
            assert(NULL != requestPackage);
            const PolicyConfigItemBasePtr& policyConfigItemBase = requestPackage->getPolicyItem();
            assert(NULL != policyConfigItemBase);
            LOG(ERROR) << "push alarmMsg failed, metric[" 
		       << policyConfigItemBase->getMetric() << "], id[" 
		       << policyConfigItemBase->getId()<< "]";
        }
    }
}

RA_END_NAMESPACE(process);

