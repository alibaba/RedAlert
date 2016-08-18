#ifndef RA_CYCLEPOLICYITEM_H
#define RA_TRENDPOLICYITEM_H

#include <ra/common/CommonDefine.h>
#include <ra/common/Log.h>
#include <ra/config/DeviationPolicyItem.h>
RA_BEGIN_NAMESPACE(config);

class TrendPolicyItem: public DeviationPolicyItem
{
public:
    TrendPolicyItem();
    ~TrendPolicyItem();

private:
    RA_LOG_DECLARE();
};

RA_TYPEDEF_PTR(TrendPolicyItem);

RA_END_NAMESPACE(ra);

#endif //RA_TRENDPOLICYITEM_H
