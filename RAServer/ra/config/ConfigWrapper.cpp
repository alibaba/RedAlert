#include <ra/common/CommonDefine.h>
#include <ra/config/ConfigWrapper.h>
#include <ra/util/ConfigUtil.h>
#include <ra/util/SqlData.h>

RA_BEGIN_NAMESPACE(config);

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
        LOG(ERROR) << "load red alert config failed";
        return false;
    }
    if (!_redAlertConfig.containThisServer()) {
        _configVersion = configVersion;
        LOG(WARNING) << "red alert config has no this server, "
	    "this server will do nothing";
        return true;
    }

    //load amonitor config
    ret = _dataSourceConfig.loadConfig(configFilePath);
    if (!ret) {
        LOG(ERROR) << "load amonitor config failed";
        return false;
    }

    //load pair values
    ret = loadPairVals(configFilePath);
    if (!ret) {
        LOG(ERROR) << "load pair values failed";
        return false;
    }

    //load shield config
    ret = _shieldConfig.loadConfig(configFilePath);
    if (!ret) {
        LOG(ERROR) << "load shield config failed";
        return false;
    }

    //load policy config
    ret = _policyConfig.loadConfig(configFilePath);
    if (!ret) {
        LOG(ERROR) << "load policy Config config failed";
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
        LOG(ERROR) << "load table[" << TABLE_NAME_PAIR << "] failed";
        return false;
    }
    int32_t rowNum = sqlData.getRow();
    int32_t colNum = sqlData.getCol();
    if (rowNum < expectLoadCount) {
        LOG(ERROR) << "load pair failed, because miss config item";
        return false;
    }
    if (columnCount != colNum) {
        LOG(ERROR) << "table[" << TABLE_NAME_PAIR << "] format is illegal, colNum["
		   << colNum << "] is not 5";
        return false;
    }
    vector<string> rowVals;
    for (int32_t rowIndex = 0; rowIndex < rowNum; ++rowIndex) {
        ret = sqlData.getRow(rowIndex, rowVals);
        if (!ret) {
            LOG(ERROR) << "read table[" << TABLE_NAME_PAIR << "] row failed";
            return false;
        }
        const string& key = rowVals[1];
        const string& val = rowVals[2];
        if (key == "smoothingFactor") {
            Util::fromString(val, _smoothingFactor);
            LOG(INFO) << "smoothingFactor:" << _smoothingFactor;
            --expectLoadCount;
        }
        else if (key == "trendFactor") {
            Util::fromString(val, _trendFactor);
            LOG(INFO) << "trendFactor:" << _trendFactor;
            --expectLoadCount;
        }
    }
    if (expectLoadCount != 0) {
        LOG(INFO) << "load pair failed, because miss config item";
        return false;
    }
    return true;
}

RA_END_NAMESPACE(config);

