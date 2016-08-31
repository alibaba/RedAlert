#include <ra/config/SingularityPolicyItem.h>

RA_BEGIN_NAMESPACE(config);

SingularityPolicyItem::SingularityPolicyItem()
{
    setTriggerType(PT_SINGULARITY);
}

SingularityPolicyItem::~SingularityPolicyItem()
{
}

bool SingularityPolicyItem::fromJson(const common::JsonObjectPtr& json) {
    DeviationPolicyItem::fromJson(json);
    _checkType = CT_SINGLE;
    return true;
}

RA_END_NAMESPACE(ra);

