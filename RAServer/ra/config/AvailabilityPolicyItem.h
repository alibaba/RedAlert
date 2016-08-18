#ifndef RA_AVAILABILITYPOLICYITEM_H
#define RA_AVAILABILITYPOLICYITEM_H

#include <ra/common/CommonDefine.h>
#include <ra/common/Log.h>
#include <ra/config/PolicyConfigItemBase.h>

RA_BEGIN_NAMESPACE(config);

class AvailabilityPolicyItem: public PolicyConfigItemBase
{
public:
    AvailabilityPolicyItem();
    ~AvailabilityPolicyItem();

    bool fromJson(const common::JsonObjectPtr& json);

    int32_t getMinHostNum() const {
        return _minHostNum;
    }
    int32_t getMaxMissHostNum() const {
        return _maxMissHostNum;
    }
    void setMinHostNum(int32_t min) {
        _minHostNum = min;
    }
private:
    int32_t _minHostNum;
    int32_t _maxMissHostNum;
    RA_LOG_DECLARE();
};

RA_TYPEDEF_PTR(AvailabilityPolicyItem);

RA_END_NAMESPACE(ra);

#endif //RA_AVAILABILITYPOLICYITEM_H
