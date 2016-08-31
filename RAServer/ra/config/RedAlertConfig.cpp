#include <ra/util/SqlData.h>
#include <ra/util/Util.h>
#include <ra/util/StringHash.h>
#include <ra/config/RedAlertConfig.h>
#include <ra/common/CommonDefine.h>

RA_BEGIN_NAMESPACE(config);
using namespace std;
RA_USE_NAMESPACE(util);

RedAlertConfig::RedAlertConfig()
    : _hashMethod(StringHash::hashString)
    , _containThisServer(false)
{
}

RedAlertConfig::~RedAlertConfig() { 
}

bool RedAlertConfig::loadConfig(const string& configFilePath, const string& localAddress)
{
    clear();
    SqlData sqlData(configFilePath);
    const char* columns[] = { "id", "address", "service", "weight", "author", "modifiedTime" };
    int32_t columnCount = sizeof(columns)/sizeof(columns[0]);
    bool ret = sqlData.load(TABLE_NAME_RA, vector<string>(columns, columns + columnCount));
    if (!ret) {
        LOG(ERROR) << "load table[" << TABLE_NAME_RA << "] failed";
        return false;
    }
    int32_t rowNum = sqlData.getRow();
    int32_t colNum = sqlData.getCol();
    if (rowNum == 0) {
        LOG(ERROR) << "table[" << TABLE_NAME_RA << "] is empty";
        return true;
    }
    if (columnCount != colNum) {
        LOG(ERROR) << "table[" << TABLE_NAME_RA << "] format is illegal, colNum["
		      << colNum << "] is not 6";
        return false;
    }

    vector<string> rowVals;
    _containThisServer = false;
    for (int32_t rowIndex = 0; rowIndex < rowNum; ++rowIndex) {
        ret = sqlData.getRow(rowIndex, rowVals);
        if (!ret) {
            LOG(ERROR) << "read table[" << TABLE_NAME_RA << "] row failed";
            return false;
        }
        uint32_t id = 0;
        uint32_t weight = 0;
        Util::fromString(rowVals[0], id);
        const string& address = rowVals[1];
        const string& serviceName = rowVals[2];
        Util::fromString(rowVals[3], weight);
        HostItem item(address, serviceName, id, weight);
        _hostVec.push_back(item);

        if (address == localAddress) {
            _containThisServer = true;
            _serviceName = serviceName;
        }
    }
    initHash();
    return true;
}

void RedAlertConfig::clear()
{
    _hostVec.clear();
    _containThisServer = false;
    _consistentHash.clear();
}

void RedAlertConfig::initHash()
{
    for (vector<HostItem>::const_iterator it = _hostVec.begin();
         it != _hostVec.end(); ++it) 
    {
        const HostItem& hostItem = *it;
        _consistentHash.addNode(hostItem.getServiceName(), hostItem.getWeight());
    }
}

bool RedAlertConfig::isMyMetric(const string& metricPath) const
{
    if (_consistentHash.empty()) {
        LOG(WARNING) << "no consistent hash provided";
        return true;
    }
    uint32_t key = _hashMethod(metricPath.c_str(), 0);
    const string* service = _consistentHash.getService(key);
    if (service && *service == _serviceName) {
        return true;
    }
    return false;
}

RA_END_NAMESPACE(config);
