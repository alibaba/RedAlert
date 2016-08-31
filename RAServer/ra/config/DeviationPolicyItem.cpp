#include <ra/config/DeviationPolicyItem.h>

RA_USE_NAMESPACE(util);
RA_BEGIN_NAMESPACE(config);

DeviationPolicyItem::DeviationPolicyItem():
        _ascDiffRatio(0.0),
        _ascDiffValue(0.0),
        _descDiffRatio(0.0),
        _descDiffValue(0.0)
{
    setTriggerType(PT_NONE); // must set it manually
}

DeviationPolicyItem::~DeviationPolicyItem()
{
}

bool DeviationPolicyItem::fromJson(const common::JsonObjectPtr& json) {
    PolicyConfigItemBase::fromJson(json);
    // compatible with 0.1.0
    double diffRatio = 0.0, diffValue = 0.0;
    json->get("maxDiffRatio", diffRatio, 0.0);
    json->get("minDiffValue", diffValue, 0.0);
    if (!Util::doubleEqual(0.0, diffRatio) || !Util::doubleEqual(0.0, diffValue)) {
        _ascDiffRatio = diffRatio;
        _descDiffRatio = diffRatio;
        _ascDiffValue = diffValue;
        _descDiffValue = diffValue;
        return true;
    }

    json->get("ascDiffRatio", _ascDiffRatio, 0.0);
    json->get("ascDiffValue", _ascDiffValue, 0.0);
    json->get("descDiffRatio", _descDiffRatio, 0.0);
    json->get("descDiffValue", _descDiffValue, 0.0);
    return true;
}

bool DeviationPolicyItem::isDiffTooMuch(double current, double base) const
{
    double diff = current - base;
    double ascRatioValue = _ascDiffRatio*base;
    double descRatioValue = _descDiffRatio*base;

    if (diff > 0 && usingAscDiff()) {
        // ascending
        bool ret = !usingAscDiffValue() || diff >= _ascDiffValue;
        ret = ret && (!usingAscDiffRatio() || (diff >= ascRatioValue));
        return ret;
    }
    else if (diff < 0 && usingDescDiff()) {
        // descending
        bool ret = !usingDescDiffValue() || -diff >= _descDiffValue;
        ret = ret && (!usingDescDiffRatio() || (-diff >= descRatioValue));
        return ret;
    }

    return false;
}

RA_END_NAMESPACE(ra);

