#ifndef RA_POLICYCHECKERFACTORY_H
#define RA_POLICYCHECKERFACTORY_H

#include <ra/common/CommonDefine.h>
#include <ra/common/Log.h>
#include <ra/process/PolicyChecker.h>
#include <ra/config/PolicyConfigItemBase.h>

RA_BEGIN_NAMESPACE(process);

class PolicyCheckerFactory
{
public:
    PolicyCheckerFactory();
    ~PolicyCheckerFactory();
private:
    PolicyCheckerFactory(const PolicyCheckerFactory &);
    PolicyCheckerFactory& operator=(const PolicyCheckerFactory &);

public:
    static PolicyCheckerPtr createPolicyChecker(config::PolicyTrigger policyTriger);

private:
    RA_LOG_DECLARE();
};

RA_TYPEDEF_PTR(PolicyCheckerFactory);

RA_END_NAMESPACE(process);

#endif //RA_POLICYCHECKERFACTORY_H
