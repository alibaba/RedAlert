#ifndef RA_CYCLEPOLICYITEM_H
#define RA_CYCLEPOLICYITEM_H

#include <ra/common/CommonDefine.h>
#include <ra/common/CommonDefine.h>
#include <ra/common/Log.h>
#include <ra/config/DeviationPolicyItem.h>
#include <vector>

RA_BEGIN_NAMESPACE(config);

class CyclePolicyItem: public DeviationPolicyItem
{
public:
    CyclePolicyItem();
    ~CyclePolicyItem();

    bool isDiffTooMuch(double current, const std::vector<double>& historyValVec) const;

    bool fromJson(const common::JsonObjectPtr& json);

    int32_t getCycleTime() const {
        return _cycleTime;
    }
    int32_t getCycleCount() const{
        return _cycleCount;
    }

    void setCycleTime(int32_t value) {
        _cycleTime = value;
    }
    void setCycleCount(int32_t value){
        _cycleCount = value;
    }

private:
    int32_t _cycleTime;// second
    int32_t _cycleCount;
    RA_LOG_DECLARE();
};

RA_TYPEDEF_PTR(CyclePolicyItem);

RA_END_NAMESPACE(ra);

#endif //RA_CYCLEPOLICYITEM_H
