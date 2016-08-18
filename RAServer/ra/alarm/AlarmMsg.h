#ifndef RA_ALARMMSG_H
#define RA_ALARMMSG_H

#include <ra/common/Common.h>
#include <ra/common/Log.h>
#include <ra/util/Util.h>
#include <ra/tree/ProcessPackage.h>

RA_BEGIN_NAMESPACE(alarm);

struct AlarmMsgKey
{
    std::string metric;
    uint32_t id;
    std::string toString() {
        std::string idStr;
        util::Util::toString(id, idStr);
        return "[" + idStr + "]" + metric;
    }
    bool operator<(const AlarmMsgKey& key) const {
        if (id != key.id) {
            return id < key.id;
        }
        return metric < key.metric;
    }
};

enum MsgCode {
    CODE_OK = 0,
    CODE_WARN = 1,
    CODE_CRITICAL = 2
};

class AlarmMsg
{
public:
    AlarmMsg(tree::ProcessPackagePtr processPackage, MsgCode code);
    virtual ~AlarmMsg();

private:
    AlarmMsg(const AlarmMsg &);
    AlarmMsg& operator=(const AlarmMsg &);

public:
    virtual AlarmMsgKey getKey() const;
    virtual int64_t getMinAlarmInterval() const;
    virtual const std::string& getAlarmGroup() const;
    virtual const std::string& getAlarmLevel() const;
    virtual std::string getDescription() const;
    virtual void setPackageLastAlarmTimeSec(int64_t lastAlarmTime);

    MsgCode getMsgCode() const  {
        return _code;
    }
    void setAlarmMsgStr(const std::string& alarmMsgStr) {
        _alarmMsgStr = alarmMsgStr;
        if (!_alarmMsgStr.empty()) {
            _alarmMsgStr = getAlarmMsgPrefix() + _alarmMsgStr;
        }
    }

    std::string getAlarmMsgStr() const {
        return _alarmMsgStr;
    }

    bool empty() const{
        return _alarmMsgStr.empty();
    }

protected:
    virtual std::string getAlarmMsgPrefix() const;
    
protected:
    tree::ProcessPackagePtr _processPackage;
    std::string _alarmMsgStr;
    MsgCode _code;

private:
    RA_LOG_DECLARE();
};

RA_TYPEDEF_PTR(AlarmMsg);

RA_END_NAMESPACE(alarm);

#endif //RA_ALARMMSG_H
