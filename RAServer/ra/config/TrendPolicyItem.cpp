#include <ra/config/TrendPolicyItem.h>
#include <ra/util/Util.h>

RA_BEGIN_NAMESPACE(config);
RA_LOG_SETUP(config, TrendPolicyItem);

TrendPolicyItem::TrendPolicyItem()
{
    setTriggerType(PT_TREND);
}

TrendPolicyItem::~TrendPolicyItem()
{
}

RA_END_NAMESPACE(ra);

