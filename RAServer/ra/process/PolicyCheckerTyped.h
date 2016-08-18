#ifndef RA_POLICYCHECKERTYPED_H
#define RA_POLICYCHECKERTYPED_H

#include <ra/common/Common.h>
#include <ra/common/CommonDefine.h>
#include <ra/common/Log.h>
#include <ra/config/PolicyConfigItemBase.h>
#include <ra/process/PolicyChecker.h>

RA_BEGIN_NAMESPACE(process);

template <typename PolicyItemType = config::PolicyConfigItemBase>
class PolicyCheckerTyped: public PolicyChecker
{
public:
    virtual bool init(const tree::ProcessPackagePtr& processPackage);

protected:
    std::tr1::shared_ptr<PolicyItemType> _policyItem;
};

template <typename PolicyItemType>
bool PolicyCheckerTyped<PolicyItemType>::init(
    const tree::ProcessPackagePtr &processPackage)
{
    if (!PolicyChecker::init(processPackage)) {
        return false;
    }
    _policyItem = std::tr1::dynamic_pointer_cast<PolicyItemType>(_requestPackage->getPolicyItem());
    if (_policyItem == NULL) {
        RA_LOG(ERROR, "Cannot initialize policy checker: policy item is NULL");
        return false;
    }
    return true;
}

RA_END_NAMESPACE(process);

#endif //RA_POLICYCHECKERTYPED_H
