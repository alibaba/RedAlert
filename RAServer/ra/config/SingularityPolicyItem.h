#ifndef RA_SINGULARITYPOLICYITEM_H
#define RA_SINGULARITYPOLICYITEM_H

#include <ra/common/CommonDefine.h>
#include <ra/common/Log.h>
#include <ra/config/DeviationPolicyItem.h>
RA_BEGIN_NAMESPACE(config);

class SingularityPolicyItem: public DeviationPolicyItem
{
public:
    SingularityPolicyItem();
    ~SingularityPolicyItem();

    bool fromJson(const common::JsonObjectPtr& json);
};

RA_TYPEDEF_PTR(SingularityPolicyItem);

RA_END_NAMESPACE(ra);

#endif //RA_SINGULARITYPOLICYITEM_H
