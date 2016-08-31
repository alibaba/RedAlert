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

    LOG(INFO) << "Alarm Manager inited success: threadNum:" << _threadNum
	      << ", queueSize:" << queueSize << ", alimonitorUrl:"
	      << _alimonitorUrl << ", mailUser:" << _mailUser 
	      << ", mailPwd:" << _mailPwd << ", mailServer:" << _mailServer;
    return true;
}

bool AlarmManager::start()
{
    if (_started) {
        LOG(ERROR) << "can not start AlarmManager thread twice";
        return false;
    }
    _alarmPool = new ThreadPool(_threadNum, _queueSize);
    if (_alarmPool == NULL || !_alarmPool->start()) {
        LOG(ERROR) << "alarm thread pool start fail!";
        delete _alarmPool;
        _alarmPool = NULL;
        return false;
    }
    _started = true;
    LOG(INFO) << "Alarm manager start success";
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
        LOG(ERROR) << "can not push alarmMsg whithout started alarm manager";
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
            LOG(INFO) << "alarmMsg last alarm time:" << it->second << ", restrained! metric:"
		      << key.metric << ", "
		      << "id:" << key.id << ", alarmMsg:" << alarmMsg->getAlarmMsgStr();
            return true;
        }
        _lastAlarmTime[key] = currTimeSec;
    }
    
    AlarmWorkItem *workItem = new AlarmWorkItem(this, alarmMsg);
    if (!_alarmPool->push(workItem)) {
        delete workItem;
        LOG(ERROR) << "push alarmMsg to pool failed";
        return false;
    }
    VLOG(1) << "push alarmMsg to alarm pool success, metric:" 
	    << key.metric << ", id:" << key.id;
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
    LOG(INFO) << "Succesfully raise curlmail alarm to:[" << alarmGroupStr << "], msg:[" << body <<"]";
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
        LOG(WARNING) << "alarmMsgStr is truncated to length:"
		  << truncateAlarmMsgStr.size() << ", id:" << key.id << ", metric:" << key.metric;
    }
    else {
        snprintf(jsonMsg, sizeof(jsonMsg),
                 "{\"collection_flag\":0,"
                  "\"error_info\":\"\","
                  "\"MSG\":[{\"status\":%d,\"output\":\"%s\"}]}",
                 code, alarmMsgStr.c_str());        
    }
    VLOG(1) << "jsonMsg: " << jsonMsg;

    for (; it != alarmGroupVec.end(); ++it) {
        const string& alarmGroup = *it;
        common::HttpClient httpClient(false, ALIMONITOR_HTTP_TIMEOUT_MS / 1000);
        string query = "/passive?name=" + alarmGroup + "&msg=" + Util::urlEncode(jsonMsg);
        if (!httpClient.init()) {
            LOG(ERROR) << "http client init failed!";
            return;
        }
        common::HttpResponse httpResponse;
        string url = _alimonitorUrl + query;
        bool ret = httpClient.get(url, &httpResponse);
        if (!ret) {
            LOG(ERROR) << "get connection(" << _alimonitorUrl 
		       << ") or send packets(" << query << ") failed!, "
		       << "raise alimonitor alarm failed!";
            return;
        }
    }
    LOG(INFO) << "Succesfully raise alimonitor alarm to:[" << alarmGroupStr << "], code:[" 
	      << code << "], msg:[" << alarmMsgStr << "]";
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
        LOG(WARNING) << "alarmMsgStr is truncated to length:" << alarmMsgStr.size() 
		     <<", id:" << key.id << ", metric:" << key.metric;
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
    LOG(INFO) << "Succesfully raise cumt alarm to:[" << alarmGroupStr <<"], msg:["<< alarmMsgStr <<"]";
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
            LOG(ERROR) << "get host name failed, error[" << strerror(errno) << "]";
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
        LOG(ERROR) << "http client init failed!";
        return false;
    }
    string url = _alimonitorUrl + uri;
    string body = string(data);
    common::HttpResponse httpResponse;
    if (!httpClient.post(url, body, &httpResponse)) {
        LOG(ERROR) << "get connection(" << _alimonitorUrl << ") or send packets(" 
		   << data <<") failed!, "
		   << "raise cumt alarm failed!";
        return false;
    }
    const string &result = httpResponse.body;
    if (result.find("success=true") == string::npos) {
        LOG(ERROR) << "send cumt alarm failed, result:" << result;
        return false;
    }
    return true;
}

RA_END_NAMESPACE(alarm);
