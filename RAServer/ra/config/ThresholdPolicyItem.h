#ifndef RA_THRESHOLDPOLICYITEM_H
#define RA_THRESHOLDPOLICYITEM_H

#include <ra/common/CommonDefine.h>
#include <ra/common/Log.h>
#include <ra/config/PolicyConfigItemBase.h>

RA_BEGIN_NAMESPACE(process);
class ThresholdPolicyCheckerTest;
RA_END_NAMESPACE(process);

RA_BEGIN_NAMESPACE(config);

class ThresholdPolicyItem: public PolicyConfigItemBase
{
public:
    ThresholdPolicyItem();
    ~ThresholdPolicyItem();

    bool fromJson(const common::JsonObjectPtr& json);

    double getUpBound() const {
        return _upBound;
    }
    double getDownBound() const {
        return _downBound;
    }

private:
    friend class process::ThresholdPolicyCheckerTest;

private:
    double _upBound;
    double _downBound;

    RA_LOG_DECLARE();
};

RA_TYPEDEF_PTR(ThresholdPolicyItem);

RA_END_NAMESPACE(ra);

#endif //RA_THRESHOLDPOLICYITEM_H
