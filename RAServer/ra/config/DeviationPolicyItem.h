#ifndef RA_DEVIATIONPOLICYITEM_H
#define RA_DEVIATIONPOLICYITEM_H

#include <ra/common/CommonDefine.h>
#include <ra/common/Log.h>
#include <ra/config/PolicyConfigItemBase.h>
#include <ra/util/Util.h>
RA_BEGIN_NAMESPACE(config);

class DeviationPolicyItem: public PolicyConfigItemBase
{
public:
    DeviationPolicyItem();
    ~DeviationPolicyItem();

public:
    bool fromJson(const common::JsonObjectPtr& json);

    double getAscDiffRatio() const
    {
        return _ascDiffRatio;
    }

    double getAscDiffValue() const
    {
        return _ascDiffValue;
    }

    double getDescDiffRatio() const
    {
        return _descDiffRatio;
    }

    double getDescDiffValue() const
    {
        return _descDiffValue;
    }

    void setAscDiffRatio(double v)
    {
        _ascDiffRatio = v;
    }

    void setAscDiffValue(double v)
    {
        _ascDiffValue = v;
    }

    void setDescDiffRatio(double v)
    {
        _descDiffRatio = v;
    }

    void setDescDiffValue(double v)
    {
        _descDiffValue = v;
    }

    bool isDiffTooMuch(double current, double base) const;
private:
    bool usingAscDiffRatio() const
    {
        return !util::Util::doubleEqual(_ascDiffRatio, 0.0);
    }

    bool usingAscDiffValue() const
    {
        return !util::Util::doubleEqual(_ascDiffValue, 0.0);
    }

    bool usingDescDiffRatio() const
    {
        return !util::Util::doubleEqual(_descDiffRatio, 0.0);
    }

    bool usingDescDiffValue() const
    {
        return !util::Util::doubleEqual(_descDiffValue, 0.0);
    }

    bool usingAscDiff() const
    {
        return usingAscDiffRatio() || usingAscDiffValue();
    }

    bool usingDescDiff() const
    {
        return usingDescDiffRatio() || usingDescDiffValue();
    }
    
private:
    double _ascDiffRatio;
    double _ascDiffValue;
    double _descDiffRatio;
    double _descDiffValue;
};

RA_TYPEDEF_PTR(DeviationPolicyItem);

RA_END_NAMESPACE(ra);

#endif //RA_DEVIATIONPOLICYITEM_H
