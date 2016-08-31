#include <ra/config/AvailabilityPolicyItem.h>

RA_BEGIN_NAMESPACE(config);

AvailabilityPolicyItem::AvailabilityPolicyItem():
    _minHostNum(0),
    _maxMissHostNum(0)
{
    setTriggerType(PT_AVAILABILITY);
}

AvailabilityPolicyItem::~AvailabilityPolicyItem()
{
}

bool AvailabilityPolicyItem::fromJson(const common::JsonObjectPtr& json) {
    PolicyConfigItemBase::fromJson(json);
    json->get("minHostNum", _minHostNum, 1);
    json->get("maxMissHostNum", _maxMissHostNum, 1);
    _checkType = CT_SINGLE;
    return true;
}

RA_END_NAMESPACE(ra);

