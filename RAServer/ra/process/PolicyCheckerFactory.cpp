#include <ra/process/PolicyCheckerFactory.h>
#include <ra/process/PolicyChecker.h>
#include <ra/process/ThresholdPolicyChecker.h>
#include <ra/process/TrendPolicyChecker.h>
#include <ra/process/CyclePolicyChecker.h>
#include <ra/process/AvailabilityPolicyChecker.h>
#include <ra/process/SingularityPolicyChecker.h>

RA_BEGIN_NAMESPACE(process);

PolicyCheckerFactory::PolicyCheckerFactory() { 
}

PolicyCheckerFactory::~PolicyCheckerFactory() { 
}

PolicyCheckerPtr PolicyCheckerFactory::createPolicyChecker(
        config::PolicyTrigger policyTriger)
{
    switch(policyTriger) {
    case config::PT_THRESHOLD: 
        return ThresholdPolicyCheckerPtr(new ThresholdPolicyChecker());
    case config::PT_TREND: 
        return TrendPolicyCheckerPtr(new TrendPolicyChecker());
    case config::PT_AVAILABILITY: 
        return AvailabilityPolicyCheckerPtr(new AvailabilityPolicyChecker());
    case config::PT_SINGULARITY: 
        return SingularityPolicyCheckerPtr(new SingularityPolicyChecker());
    case config::PT_CYCLE: 
        return CyclePolicyCheckerPtr(new CyclePolicyChecker());
    default:
        return PolicyCheckerPtr();
    }
}

RA_END_NAMESPACE(process);

