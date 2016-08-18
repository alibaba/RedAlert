#include <ra/common/CommonDefine.h>
#include <ra/config/ConfigWrapper.h>
#include <ra/util/ConfigUtil.h>
#include <ra/util/SqlData.h>

RA_BEGIN_NAMESPACE(config);
RA_LOG_SETUP(config, ConfigWrapper);

using namespace std;
RA_USE_NAMESPACE(util);

ConfigWrapper::ConfigWrapper() 
    : _smoothingFactor(0),
      _trendFactor(0),
      _configVersion(INVALID_CONFIG_VERSION)
{ 
    
}

ConfigWrapper::~ConfigWrapper() { 
}

void ConfigWrapper::clear()
{
    _smoothingFactor = 0.0;
    _trendFactor = 0.0;
    _redAlertConfig.clear();
    _policyConfig.clear();
    _shieldConfig.clear();
    _dataSourceConfig.clear();
}

bool ConfigWrapper::loadConfig(const string& configRoot, int32_t configVersion,
                               const string& localAddress)
{
    clear();
    const string& configFilePath = ConfigUtil::getConfigFilePath(configRoot,
            configVersion);

    // load red alert config
    bool ret = _redAlertConfig.loadConfig(configFilePath, localAddress);
    if (!ret) {
        RA_LOG(ERROR, "load red alert config failed");
        return false;
    }
    if (!_redAlertConfig.containThisServer()) {
        _configVersion = configVersion;
        RA_LOG(WARN, "red alert config has no this server, "
               "this server will do nothing");
        return true;
    }

    //load amonitor config
    ret = _dataSourceConfig.loadConfig(configFilePath);
    if (!ret) {
        RA_LOG(ERROR, "load amonitor config failed");
        return false;
    }

    //load pair values
    ret = loadPairVals(configFilePath);
    if (!ret) {
        RA_LOG(ERROR, "load pair values failed");
        return false;
    }

    //load shield config
    ret = _shieldConfig.loadConfig(configFilePath);
    if (!ret) {
        RA_LOG(ERROR, "load shield config failed");
        return false;
    }

    //load policy config
    ret = _policyConfig.loadConfig(configFilePath);
    if (!ret) {
        RA_LOG(ERROR, "load policy Config config failed");
        return false;
    }

    //filter policy config
    _policyConfig.filterMetrics(std::tr1::bind(&RedAlertConfig::isMyMetric, 
            &_redAlertConfig, std::tr1::placeholders::_1));
    
    _configVersion = configVersion;
    return true;
}

bool ConfigWrapper::loadPairVals(const string& configFilePath)
{
    int32_t expectLoadCount = 2;
    SqlData sqlData(configFilePath);
    const char* columns[] = { "id", "key", "value", "author", "modifiedTime" };
    int32_t columnCount = sizeof(columns)/sizeof(columns[0]);
    bool ret = sqlData.load(TABLE_NAME_PAIR, vector<string>(columns, columns + columnCount));
    if (!ret) {
        RA_LOG(ERROR, "load table[%s] failed", TABLE_NAME_PAIR.c_str());
        return false;
    }
    int32_t rowNum = sqlData.getRow();
    int32_t colNum = sqlData.getCol();
    if (rowNum < expectLoadCount) {
        RA_LOG(ERROR, "load pair failed, because miss config item");
        return false;
    }
    if (columnCount != colNum) {
        RA_LOG(ERROR, "table[%s] format is illegal, colNum[%d] is not 5",
               TABLE_NAME_PAIR.c_str(), colNum);
        return false;
    }
    vector<string> rowVals;
    for (int32_t rowIndex = 0; rowIndex < rowNum; ++rowIndex) {
        ret = sqlData.getRow(rowIndex, rowVals);
        if (!ret) {
            RA_LOG(ERROR, "read table[%s] row failed", TABLE_NAME_PAIR.c_str());
            return false;
        }
        const string& key = rowVals[1];
        const string& val = rowVals[2];
        if (key == "smoothingFactor") {
            Util::fromString(val, _smoothingFactor);
            RA_LOG(INFO, "smoothingFactor:%.2lf", _smoothingFactor);
            --expectLoadCount;
        }
        else if (key == "trendFactor") {
            Util::fromString(val, _trendFactor);
            RA_LOG(INFO, "trendFactor:%.2lf", _trendFactor);
            --expectLoadCount;
        }
    }
    if (expectLoadCount != 0) {
        RA_LOG(ERROR, "load pair failed, because miss config item");
        return false;
    }
    return true;
}

RA_END_NAMESPACE(config);

