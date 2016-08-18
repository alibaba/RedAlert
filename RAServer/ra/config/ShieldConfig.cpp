#include <sstream>
#include <ra/util/Util.h>
#include <ra/util/SqlData.h>
#include <ra/config/ShieldConfig.h>

using namespace std;
RA_USE_NAMESPACE(util);

RA_BEGIN_NAMESPACE(config);
RA_LOG_SETUP(config, ShieldConfig);

ShieldConfig::ShieldConfig()
{ 
}

ShieldConfig::~ShieldConfig()
{ 
}

void ShieldConfig::clear()
{
    _shieldItemVec.clear();
    _metricShieldTimeMap.clear();
}

bool ShieldConfig::loadConfig(const std::string& configFilePath)
{
    clear();
    SqlData sqlData(configFilePath);
    const char* columns[] = { "id", "groupName", "metric", "host", "endTime", "author", "modifiedTime" };
    int32_t columnCount = sizeof(columns)/sizeof(columns[0]);
    bool ret = sqlData.load(TABLE_NAME_SHIELD, vector<string>(columns, columns + columnCount));
    if (!ret) {
        RA_LOG(ERROR, "load table[%s] failed", TABLE_NAME_SHIELD.c_str());
        return false;
    }
    int32_t rowNum = sqlData.getRow();
    int32_t colNum = sqlData.getCol();
    if (rowNum == 0) {
        RA_LOG(WARN, "table[%s] is empty", TABLE_NAME_SHIELD.c_str());
        return true;
    }
    if (columnCount != colNum) {
        RA_LOG(ERROR, "table[%s] format is illegal, colNum[%d] is not 7", 
               TABLE_NAME_SHIELD.c_str(), colNum);
        return false;
    }
    vector<string> rowVals;
    for (int32_t rowIndex = 0; rowIndex < rowNum; ++rowIndex) {
        ret = sqlData.getRow(rowIndex, rowVals);
        if (!ret) {
            RA_LOG(ERROR, "read table[%s] row failed", TABLE_NAME_SHIELD.c_str());
            return false;
        }
        ShieldItem item;
        RA_LOG(INFO, "shieldConfig:group[%s]metric[%s]host[%s]", rowVals[1].c_str(),
               rowVals[2].c_str(), rowVals[3].c_str());
        Util::fromString(rowVals[0], item.id);
        item.group = rowVals[1];
        item.metric = rowVals[2];
        const vector<string>& hostVec = Util::splitString(rowVals[3], ",");
        item.hostSet.insert(hostVec.begin(), hostVec.end());
        if (!Util::formatTime(rowVals[4], item.endTime)) {
            RA_LOG(ERROR, "invalid timeformat for shield config, id[%u], %s.%s,"
                   " set it to 0(no shield)", item.id, item.group.c_str(), item.metric.c_str());
            item.endTime = 0;
        }
        _shieldItemVec.push_back(item);
    }
    return true;
}

bool ShieldConfig::isShielded(time_t curTime, const string& group, 
        const string& metric, uint32_t host) const
{
    string key = Util::makeShieldKey(group, metric, host);
    time_t endTime = getMetricShieldTime(key);
    if (INVALID_TIME == endTime) {
        endTime = 0;
        for (vector<ShieldItem>::const_iterator it = _shieldItemVec.begin();
             it != _shieldItemVec.end(); ++it) 
        {
            if (it->isMatched(group, metric, host)) {
                time_t tmpEndTime = it->endTime;
                if (tmpEndTime > endTime) {
                    endTime = tmpEndTime;
                }
            }
        }
        setMetricShieldTime(key, endTime);
    }
    return curTime < endTime;
}

time_t ShieldConfig::getMetricShieldTime(const string& metricKey) const
{
    ScopedLock lock(_cacheLock);
    map<string, time_t>::const_iterator it = _metricShieldTimeMap.find(metricKey);
    if (it == _metricShieldTimeMap.end()) {
        return INVALID_TIME;
    }
    return it->second;
}

void ShieldConfig::setMetricShieldTime(const string& metricKey, time_t endTime) const
{
    assert(INVALID_TIME != endTime);
    ScopedLock lock(_cacheLock);
    _metricShieldTimeMap[metricKey] = endTime;
}

RA_END_NAMESPACE(config);

