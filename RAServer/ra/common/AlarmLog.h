#ifndef RA_ALARMLOG_H
#define RA_ALARMLOG_H

#include <ra/common/Common.h>
#include <ra/common/Log.h>

RA_BEGIN_NAMESPACE(common);

class AlarmLog
{
public:
    AlarmLog();
    ~AlarmLog();

private:
    AlarmLog(const AlarmLog &);
    AlarmLog& operator=(const AlarmLog &);

public:
    void setAlarmMsg(const std::string& alarmMsg, 
                     const std::string& alarmLevel, 
                     const std::string& alarmGroup) 
    {
        _alarmMsg = alarmMsg;
        _alarmLevel = alarmLevel;
        _alarmGroup = alarmGroup;
    }

    void setDescription(const std::string& description) {
        _description = description;
    }
    
private:
    void writeLog();
    
private:
    std::string _alarmLevel;
    std::string _alarmGroup;
    std::string _alarmMsg;
    std::string _description;
};

RA_TYPEDEF_PTR(AlarmLog);

RA_END_NAMESPACE(common);

#endif //RA_ALARMLOG_H
