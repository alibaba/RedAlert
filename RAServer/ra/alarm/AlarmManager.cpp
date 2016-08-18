#include <errno.h>
#include <ra/common/HttpClient.h>
#include <ra/common/AlarmLog.h>
#include <ra/util/Util.h>
#include <ra/alarm/AlarmManager.h>
#include <ra/alarm/AlarmWorkItem.h>
#include <ra/alarm/MailSender.h>

RA_USE_NAMESPACE(util);
RA_USE_NAMESPACE(common);
using namespace std;
RA_BEGIN_NAMESPACE(alarm);
RA_LOG_SETUP(alarm, AlarmManager);

AlarmManager::AlarmManager() 
    : _started(false)   
    , _threadNum(0)
    , _queueSize(0)
    , _alarmPool(NULL)
    , _lastClearExpiredRecordTimeSec(0)
{
}

AlarmManager::~AlarmManager() { 
    stop();
}

bool AlarmManager::init(size_t threadNum, size_t queueSize,
                        const string& alimonitorUrl,
                        const string& mailUser, 
                        const string& mailPwd, 
                        const string& mailServer)
{
    _threadNum = threadNum;
    _queueSize = queueSize;

    _alimonitorUrl = alimonitorUrl;
    _mailUser = mailUser;
    _mailPwd = mailPwd;
    _mailServer = mailServer;

    RA_LOG(INFO, "Alarm Manager inited success: threadNum:%zu, queueSize:%zu, "
           "alimonitorUrl:%s, mailUser:%s, mailPwd:%s, mailServer:%s",
           _threadNum, _queueSize, _alimonitorUrl.c_str(), _mailUser.c_str(), 
           _mailPwd.c_str(), _mailServer.c_str());

    return true;
}

bool AlarmManager::start()
{
    if (_started) {
        RA_LOG(ERROR, "can not start AlarmManager thread twice");
        return false;
    }
    _alarmPool = new ThreadPool(_threadNum, _queueSize);
    if (_alarmPool == NULL || !_alarmPool->start()) {
        RA_LOG(ERROR, "alarm thread pool start fail!");
        delete _alarmPool;
        _alarmPool = NULL;
        return false;
    }
    _started = true;
    RA_LOG(INFO, "Alarm manager start success");
    return true;
}

void AlarmManager::stop()
{
    _started = false;
    if (_alarmPool) {
        _alarmPool->stop();
        delete _alarmPool;
        _alarmPool = NULL;
    }
}

size_t AlarmManager::getQueueItemCount() const
{
    if (NULL == _alarmPool) {
        return 0;
    }
    return _alarmPool->getQueueSize();
}

bool AlarmManager::pushAlarmMessage(AlarmMsgPtr& alarmMsg) 
{
    ScopedLock lock(_mutex);
    if (!_started || NULL == _alarmPool) {
        RA_LOG(ERROR, "can not push alarmMsg whithout started alarm manager");
        return false;
    }

    int64_t currTimeSec = Util::currentTimeInSeconds();
    if (currTimeSec - _lastClearExpiredRecordTimeSec > ALARM_RECORD_EXPIRE_TIME_SEC) {
        clearExpiredRecordTime(currTimeSec);
    }
    assert(NULL != alarmMsg);
    AlarmMsgKey key = alarmMsg->getKey();
    MsgCode code = alarmMsg->getMsgCode();
    if (CODE_OK != code) {
        int64_t minAlarmIntervalSec = alarmMsg->getMinAlarmInterval();
        map<AlarmMsgKey, int64_t>::const_iterator it = _lastAlarmTime.find(key);
        if (it != _lastAlarmTime.end() && currTimeSec - it->second <  minAlarmIntervalSec) {
            RA_LOG(INFO, "alarmMsg last alarm time:%"PRId64", restrained! metric:%s, "
                   "id:%u, alarmMsg:%s",
                   it->second, key.metric.c_str(), key.id, 
                   alarmMsg->getAlarmMsgStr().c_str());
            return true;
        }
        _lastAlarmTime[key] = currTimeSec;
    }
    
    AlarmWorkItem *workItem = new AlarmWorkItem(this, alarmMsg);
    if (!_alarmPool->push(workItem)) {
        delete workItem;
        RA_LOG(ERROR, "push alarmMsg to pool failed");
        return false;
    }
    RA_LOG(DEBUG, "push alarmMsg to alarm pool success, metric:%s, id:%u",
           key.metric.c_str(), key.id);
    return true;
}

void AlarmManager::clearExpiredRecordTime(int64_t currTimeSec)
{
    map<AlarmMsgKey, int64_t>::iterator it = _lastAlarmTime.begin();
    while (it != _lastAlarmTime.end()) {
        int64_t lastAlarmTimeSec = it->second;
        if (currTimeSec - lastAlarmTimeSec > ALARM_RECORD_EXPIRE_TIME_SEC) {
            _lastAlarmTime.erase(it++);
        }
        else {
            ++it;
        }
    }
    if (_lastClearExpiredRecordTimeSec < currTimeSec) {
        _lastClearExpiredRecordTimeSec = currTimeSec;
    }
}

void AlarmManager::raiseCurlMailAlarm(AlarmMsgPtr& alarmMsg)
{
    MailSender sender(_mailUser, _mailPwd, _mailServer);
    AlarmMsgKey key = alarmMsg->getKey();
    const string& alarmGroupStr = alarmMsg->getAlarmGroup();
    const vector<string>& addressVec = Util::splitString(alarmGroupStr, MULTI_ALARM_GROUP_SEP);
    const string& title = key.toString();
    const string& body = alarmMsg->getAlarmMsgStr();
    sender.sendMail(addressVec, title, body);
    AlarmLog alarmLog;
    alarmLog.setAlarmMsg(body, alarmMsg->getAlarmLevel(), alarmGroupStr);
    alarmLog.setDescription(alarmMsg->getDescription());
    RA_LOG(INFO, "Succesfully raise curlmail alarm to:[%s], msg:[%s]", 
           alarmGroupStr.c_str(), body.c_str());

}

void AlarmManager::raiseAlimonitorAlarm(AlarmMsgPtr& alarmMsg)
{
    assert(NULL != alarmMsg);
    const string& alarmMsgStr = alarmMsg->getAlarmMsgStr();
    const string& alarmGroupStr = alarmMsg->getAlarmGroup();
    const vector<string>& alarmGroupVec = Util::splitString(alarmGroupStr, MULTI_ALARM_GROUP_SEP);
    vector<string>::const_iterator it = alarmGroupVec.begin();

    MsgCode code = alarmMsg->getMsgCode();
    //'{"collection_flag":0,"error_info":"","MSG":[{"status":2,"output":"$content"}]}'
    char jsonMsg[MAX_ALARM_MESSEG_LENGTH + 200];
    if (alarmMsgStr.size() > MAX_ALARM_MESSEG_LENGTH) {
        string truncateAlarmMsgStr = alarmMsgStr.substr(0, MAX_ALARM_MESSEG_LENGTH) + "...";
        snprintf(jsonMsg, sizeof(jsonMsg),
                 "{\"collection_flag\":0,"
                  "\"error_info\":\"\","
                  "\"MSG\":[{\"status\":%d,\"output\":\"%s\"}]}",
                 code, truncateAlarmMsgStr.c_str());
        AlarmMsgKey key = alarmMsg->getKey();
        RA_LOG(WARN, "alarmMsgStr is truncated to length:%zu, id:%u, metric:%s", 
               truncateAlarmMsgStr.size(), key.id, key.metric.c_str());
    }
    else {
        snprintf(jsonMsg, sizeof(jsonMsg),
                 "{\"collection_flag\":0,"
                  "\"error_info\":\"\","
                  "\"MSG\":[{\"status\":%d,\"output\":\"%s\"}]}",
                 code, alarmMsgStr.c_str());        
    }
    RA_LOG(DEBUG, "jsonMsg: %s", jsonMsg);

    for (; it != alarmGroupVec.end(); ++it) {
        const string& alarmGroup = *it;
        common::HttpClient httpClient(false, ALIMONITOR_HTTP_TIMEOUT_MS / 1000);
        string query = "/passive?name=" + alarmGroup + "&msg=" + Util::urlEncode(jsonMsg);
        if (!httpClient.init()) {
            RA_LOG(ERROR, "http client init failed!");
            return;
        }
        common::HttpResponse httpResponse;
        string url = _alimonitorUrl + query;
        bool ret = httpClient.get(url, &httpResponse);
        if (!ret) {
            RA_LOG(ERROR, "get connection(%s) or send packets(%s) failed!, "
                   "raise alimonitor alarm failed!", _alimonitorUrl.c_str(), query.c_str());
            return;
        }
    }
    RA_LOG(INFO, "Succesfully raise alimonitor alarm to:[%s], code:[%d], msg:[%s]", 
           alarmGroupStr.c_str(), code, alarmMsgStr.c_str());
    if (code == CODE_OK) {
        alarmMsg->setPackageLastAlarmTimeSec(INVALID_TIME);
    }
    else {
        int64_t currTimeSec = Util::currentTimeInSeconds();
        alarmMsg->setPackageLastAlarmTimeSec(currTimeSec);
    }
    AlarmLog alarmLog;
    alarmLog.setAlarmMsg(alarmMsgStr, alarmMsg->getAlarmLevel(), alarmGroupStr);
    alarmLog.setDescription(alarmMsg->getDescription());
}

void AlarmManager::raiseUcmtAlarm(AlarmMsgPtr& alarmMsg)
{
    assert(alarmMsg);
    string alarmMsgStr = alarmMsg->getAlarmMsgStr();
    const string& alarmGroupStr = alarmMsg->getAlarmGroup();

    if (alarmMsgStr.size() > MAX_ALARM_MESSEG_LENGTH) {
        alarmMsgStr = alarmMsgStr.substr(0, MAX_ALARM_MESSEG_LENGTH) + "...";
        AlarmMsgKey key = alarmMsg->getKey();
        RA_LOG(WARN, "alarmMsgStr is truncated to length:%zu, id:%u, metric:%s", 
               alarmMsgStr.size(), key.id, key.metric.c_str());
    }

    const vector<string>& alarmGroupVec = Util::splitString(alarmGroupStr, MULTI_ALARM_GROUP_SEP);
    vector<string>::const_iterator it = alarmGroupVec.begin();
    MsgCode code = alarmMsg->getMsgCode();
    for (; it != alarmGroupVec.end(); ++it) {
        const string& alarmGroup = *it;
        if (!doUcmtAlarm(alarmGroup, alarmMsgStr, code)) {
            return;
        }
        if (CODE_OK != code) {
            sleep(3);
            doUcmtAlarm(alarmGroup, alarmMsgStr, CODE_OK);

        }
    }
    RA_LOG(INFO, "Succesfully raise cumt alarm to:[%s], msg:[%s]", 
           alarmGroupStr.c_str(), alarmMsgStr.c_str());
    AlarmLog alarmLog;
    alarmLog.setAlarmMsg(alarmMsgStr, alarmMsg->getAlarmLevel(), alarmGroupStr);
    alarmLog.setDescription(alarmMsg->getDescription());
}

bool AlarmManager::doUcmtAlarm(const string& alarmGroup, const string& alarmMsgStr, 
        int32_t code, const string& hostName)
{
    string realHostName = hostName;
    if (hostName.empty()) {
        char hostNameBuff[128];
        int retCode = gethostname(hostNameBuff, sizeof(hostNameBuff));
        if (0 != retCode) {
            RA_LOG(ERROR, "get host name failed, error[%s]", strerror(errno));
            return false;
        }
        realHostName = string(hostNameBuff);
    }
    int32_t curTimeSec = Util::currentTimeInSeconds();
    char data[MAX_ALARM_MESSEG_LENGTH + 200];
    snprintf(data, sizeof(data),
             "timestamp=%d&host_name=%s&svc_description=%s"
             "&return_code=%d&plugin_output=%s", 
             curTimeSec, Util::urlEncode(realHostName).c_str(), alarmGroup.c_str(), 
             code, Util::urlEncode(alarmMsgStr).c_str());
    common::HttpClient httpClient(false, UCMT_HTTP_TIMEOUT_MS / 1000);
    string uri = "/send_mon.php";
    if (!httpClient.init()) {
        RA_LOG(ERROR, "http client init failed!");
        return false;
    }
    string url = _alimonitorUrl + uri;
    string body = string(data);
    common::HttpResponse httpResponse;
    if (!httpClient.post(url, body, &httpResponse)) {
        RA_LOG(ERROR, "get connection(%s) or send packets(%s) failed!, "
               "raise cumt alarm failed!", _alimonitorUrl.c_str(), data);
        return false;
    }
    const string &result = httpResponse.body;
    if (result.find("success=true") == string::npos) {
        RA_LOG(ERROR, "send cumt alarm failed, result:%s", result.c_str());
        return false;
    }
    return true;
}

RA_END_NAMESPACE(alarm);
