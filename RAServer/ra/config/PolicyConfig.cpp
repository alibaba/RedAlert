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
    LOG(INFO) << "Total number of policies before filter: " << _policyItemVec.size();
    vector<PolicyConfigItemBasePtr>::iterator it = _policyItemVec.end();
    while(it != _policyItemVec.begin()) {
        --it;
        const string &metric  = (*it)->getMetric();
        if (!isMyMetric(metric)) {
            _policyItemVec.erase(it);
        }
    }
    LOG(INFO) << "Total number of policies after filter: " << _policyItemVec.size();
}

bool PolicyConfig::loadConfig(const std::string& configFilePath)
{
    clear();
    SqlData sqlData(configFilePath);
    const char* columns[] = { "id", "groupName", "metric", "type", "validTime", "content", "author", "modifiedTime" };
    int32_t columnCount = sizeof(columns)/sizeof(columns[0]);
    bool ret = sqlData.load(TABLE_NAME_POLICY, vector<string>(columns, columns + columnCount));
    if (!ret) {
        LOG(ERROR) << "load table[" << TABLE_NAME_POLICY <<"] failed";
        return false;
    }
    int32_t rowNum = sqlData.getRow();
    int32_t colNum = sqlData.getCol();
    if (rowNum == 0) {
        LOG(WARNING) << "table[" << TABLE_NAME_POLICY << "] is empty";
        return true;
    }
    if (columnCount != colNum) {
        LOG(ERROR) <<  "table[" << TABLE_NAME_POLICY << "] format is illegal, colNum[" 
		   << colNum << "] is not 8";
        return false;
    }
    vector<string> rowVals;
    for (int32_t rowIndex = 0; rowIndex < rowNum; ++rowIndex) {
        ret = sqlData.getRow(rowIndex, rowVals);
        if (!ret) {
            LOG(ERROR) << "read table[" << TABLE_NAME_POLICY << "] row failed";
            return false;
        }
        uint32_t id = 0;
        Util::fromString(rowVals[0], id);
        const string& group = rowVals[1];
        const string& metric = rowVals[2];
        const string& policyType = rowVals[3];
        time_t validTime = 0;
        if (!Util::formatTime(rowVals[4], validTime)) {
            LOG(ERROR) << "validTime format is not correct in policy config, "
		"id[" << id << "], " << group << "." 
		       << metric << ", set it to current time";
            validTime = Util::currentTimeInSeconds();
        }
        
        PolicyConfigItemBasePtr policyItem = createPolicyItem(policyType);
        policyItem->setId(id);
        policyItem->setGroup(group);
        policyItem->setMetric(metric);
        policyItem->setValidTime(validTime);

        const string& content = rowVals[5];
        if (NULL == policyItem) {
            LOG(ERROR) << "create [" << policyType << "] policy item failed";
            return false;
        }

        JsonPtr json = Json::load(content);
        if (!fromJson(json, *policyItem)) {
            LOG(ERROR) << "cannot load policy [" << content << "]";
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
    LOG(ERROR) << "unkown polcy type [" << policyType << "]";
    return PolicyConfigItemBasePtr();
}

bool PolicyConfig::isMonitored(const std::string& metric) const
{
    vector<PolicyConfigItemBasePtr>::const_iterator it = _policyItemVec.begin();
    while(it != _policyItemVec.end()) {
        const string& metricPattern = (*it)->getMetric();
        if (Util::isPatternMatch(metricPattern, metric)) {
            VLOG(1) << "pattern: '" << metricPattern << "', metric: '" 
		    << metric <<"'";
            return true;
        }
        ++it;
    }
    return false;
}

RA_END_NAMESPACE(ra);

