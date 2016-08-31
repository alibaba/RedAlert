#include <ra/config/ThresholdPolicyItem.h>
#include <ra/util/Util.h>

RA_BEGIN_NAMESPACE(config);

ThresholdPolicyItem::ThresholdPolicyItem():
    _upBound(0.0),
    _downBound(0.0)
{
    setTriggerType(PT_THRESHOLD);
}

ThresholdPolicyItem::~ThresholdPolicyItem()
{
}

bool ThresholdPolicyItem::fromJson(const common::JsonObjectPtr& json) {
    PolicyConfigItemBase::fromJson(json);
    json->get("upBound", _upBound, 100.0);
    json->get("downBound", _downBound, 0.0);
    return true;
}

RA_END_NAMESPACE(ra);

