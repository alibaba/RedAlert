#include <ra/common/Json.h>
#include <ra/util/Util.h>
#include <ra/util/SqlData.h>
#include <ra/config/PolicyConfig.h>

using namespace std;

RA_BEGIN_NAMESPACE(config);
RA_LOG_SETUP(config, PolicyConfig);
RA_USE_NAMESPACE(common);
RA_USE_NAMESPACE(util);

PolicyConfig::PolicyConfig()
{
}

PolicyConfig::~PolicyConfig()
{
}

void PolicyConfig::clear()
{
    _policyItemVec.clear();
}

void PolicyConfig::filterMetrics(
        const std::tr1::function<bool (const std::string&)>& isMyMetric)
{
    RA_LOG(INFO, "Total number of policies before filter: %zu", _policyItemVec.size());
    vector<PolicyConfigItemBasePtr>::iterator it = _policyItemVec.end();
    while(it != _policyItemVec.begin()) {
        --it;
        const string &metric  = (*it)->getMetric();
        if (!isMyMetric(metric)) {
            _policyItemVec.erase(it);
        }
    }
    RA_LOG(INFO, "Total number of policies after filter: %zu", _policyItemVec.size());
}

bool PolicyConfig::loadConfig(const std::string& configFilePath)
{
    clear();
    SqlData sqlData(configFilePath);
    const char* columns[] = { "id", "groupName", "metric", "type", "validTime", "content", "author", "modifiedTime" };
    int32_t columnCount = sizeof(columns)/sizeof(columns[0]);
    bool ret = sqlData.load(TABLE_NAME_POLICY, vector<string>(columns, columns + columnCount));
    if (!ret) {
        RA_LOG(ERROR, "load table[%s] failed", TABLE_NAME_POLICY.c_str());
        return false;
    }
    int32_t rowNum = sqlData.getRow();
    int32_t colNum = sqlData.getCol();
    if (rowNum == 0) {
        RA_LOG(WARN, "table[%s] is empty", TABLE_NAME_POLICY.c_str());
        return true;
    }
    if (columnCount != colNum) {
        RA_LOG(ERROR, "table[%s] format is illegal, colNum[%d] is not 8",
               TABLE_NAME_POLICY.c_str(), colNum);
        return false;
    }
    vector<string> rowVals;
    for (int32_t rowIndex = 0; rowIndex < rowNum; ++rowIndex) {
        ret = sqlData.getRow(rowIndex, rowVals);
        if (!ret) {
            RA_LOG(ERROR, "read table[%s] row failed", TABLE_NAME_POLICY.c_str());
            return false;
        }
        uint32_t id = 0;
        Util::fromString(rowVals[0], id);
        const string& group = rowVals[1];
        const string& metric = rowVals[2];
        const string& policyType = rowVals[3];
        time_t validTime = 0;
        if (!Util::formatTime(rowVals[4], validTime)) {
            RA_LOG(ERROR, "validTime format is not correct in policy config, "
                   "id[%u], %s.%s, set it to current time", id, group.c_str(),
                   metric.c_str());
            validTime = Util::currentTimeInSeconds();
        }
        
        PolicyConfigItemBasePtr policyItem = createPolicyItem(policyType);
        policyItem->setId(id);
        policyItem->setGroup(group);
        policyItem->setMetric(metric);
        policyItem->setValidTime(validTime);

        const string& content = rowVals[5];
        if (NULL == policyItem) {
            RA_LOG(ERROR, "create [%s] policy item failed", policyType.c_str());
            return false;
        }

        JsonPtr json = Json::load(content);
        if (!fromJson(json, *policyItem)) {
            RA_LOG(ERROR, "cannot load policy [%s]", content.c_str());
            return false;
        }
        _policyItemVec.push_back(policyItem);
    }
    return true;
}

PolicyConfigItemBasePtr PolicyConfig::createPolicyItem(
        const string& policyType) const
{
    if (policyType == POLICY_TYPE_TREND) {
        return TrendPolicyItemPtr(new TrendPolicyItem());
    } 
    if (policyType == POLICY_TYPE_THRESHOLD) {
        return ThresholdPolicyItemPtr(new ThresholdPolicyItem());
    } 
    if (policyType == POLICY_TYPE_AVAILABILITY) {
        return AvailabilityPolicyItemPtr(new AvailabilityPolicyItem());
    } 
    if (policyType == POLICY_TYPE_SINGULARITY) {
        return SingularityPolicyItemPtr(new SingularityPolicyItem());
    } 
    if (policyType == POLICY_TYPE_CYCLE) {
        return CyclePolicyItemPtr(new CyclePolicyItem());
    } 
    RA_LOG(ERROR, "unkown polcy type [%s]", policyType.c_str());
    return PolicyConfigItemBasePtr();
}

bool PolicyConfig::isMonitored(const std::string& metric) const
{
    vector<PolicyConfigItemBasePtr>::const_iterator it = _policyItemVec.begin();
    while(it != _policyItemVec.end()) {
        const string& metricPattern = (*it)->getMetric();
        if (Util::isPatternMatch(metricPattern, metric)) {
            RA_LOG(DEBUG, "pattern: '%s', metric: '%s'", metricPattern.c_str(), metric.c_str());
            return true;
        }
        ++it;
    }
    return false;
}

RA_END_NAMESPACE(ra);

