#include <ra/tree/ProcessPackage.h>
#include <ra/fetcher/MetricNode.h>
#include <ra/alarm/AlarmMsg.h>

using namespace std;
RA_USE_NAMESPACE(util);
RA_USE_NAMESPACE(tree);
RA_USE_NAMESPACE(config);
RA_USE_NAMESPACE(fetcher);

RA_BEGIN_NAMESPACE(alarm);
RA_LOG_SETUP(alarm, AlarmMsg);

AlarmMsg::AlarmMsg(ProcessPackagePtr processPackage, MsgCode code)
    : _processPackage(processPackage), _code(code)
{
}

AlarmMsg::~AlarmMsg() 
{ 
    if (NULL != _processPackage) {
        _processPackage->setState(ProcessPackage::PPS_IDLE);
    }
}

string AlarmMsg::getAlarmMsgPrefix() const {
    assert(NULL != _processPackage);
    const RequestPackagePtr& requestPackage = _processPackage->getRequestPackage();
    assert(NULL != requestPackage);
    const ConfigWrapperPtr& config = requestPackage->getConfig();
    assert(NULL != config);
    char buffer[512];
    snprintf(buffer, sizeof(buffer), "RA-%s:%d#",
             config->getServiceName().c_str(), config->getConfigVersion());
    return buffer;
}

AlarmMsgKey AlarmMsg::getKey() const 
{
    assert(NULL != _processPackage);
    const RequestPackagePtr& requestPackage = _processPackage->getRequestPackage();
    assert(NULL != requestPackage);
    const PolicyConfigItemBasePtr& policyItem = requestPackage->getPolicyItem();
    assert(NULL != policyItem);
    AlarmMsgKey key;
    key.id = policyItem->getId();
    key.metric = policyItem->getMetric();
    return key;
}

int64_t AlarmMsg::getMinAlarmInterval() const
{
    assert(NULL != _processPackage);
    const RequestPackagePtr& requestPackage = _processPackage->getRequestPackage();
    assert(NULL != requestPackage);
    const PolicyConfigItemBasePtr& policyItem = requestPackage->getPolicyItem();
    assert(NULL != policyItem);
    return policyItem->getMinAlarmInterval();
}

const string& AlarmMsg::getAlarmGroup() const
{
    assert(NULL != _processPackage);
    const RequestPackagePtr& requestPackage = _processPackage->getRequestPackage();
    assert(NULL != requestPackage);
    const PolicyConfigItemBasePtr& policyItem = requestPackage->getPolicyItem();
    assert(NULL != policyItem);
    return policyItem->getAlarmGroup();
}

const string& AlarmMsg::getAlarmLevel() const
{
    assert(NULL != _processPackage);
    const RequestPackagePtr& requestPackage = _processPackage->getRequestPackage();
    assert(NULL != requestPackage);
    const PolicyConfigItemBasePtr& policyItem = requestPackage->getPolicyItem();
    assert(NULL != policyItem);
    return policyItem->getAlarmLevel();
}

string AlarmMsg::getDescription() const
{
    if (_code == CODE_OK) {
        return "set code ok";
    }
    assert(NULL != _processPackage);
    vector<MetricNodePtr> dataRoots = _processPackage->getMetricDataRoots();
    if (dataRoots.empty()) {
        return "there is no response";
    }
    string description;
    string sizeStr;
    Util::toString(dataRoots.size(), sizeStr);
    description = sizeStr + " metric data trees:\n";
    for (size_t i = 0; i < dataRoots.size(); ++i) {
        string indexStr;
        Util::toString(i + 1, indexStr);
        string str = "#" + indexStr + ":[";
        if (NULL == dataRoots[i]) {
            str.append("NULL");
        }
        else {
            str.append(dataRoots[i]->toDebugString());
        }
        description.append(str);
        description.append("]\n");
    }
    return description;
}

void AlarmMsg::setPackageLastAlarmTimeSec(int64_t lastAlarmTime) {
    assert(NULL != _processPackage);
    _processPackage->setLastNonOkAlarmTimeSec(lastAlarmTime);
}



RA_END_NAMESPACE(alarm);

