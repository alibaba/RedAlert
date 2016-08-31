#ifndef RA_CHECKERWORKITEM_H
#define RA_CHECKERWORKITEM_H

#include <ra/common/Common.h>
#include <ra/common/Log.h>
#include <ra/util/ThreadPool.h>
#include <ra/alarm/AlarmManager.h>
#include <ra/tree/ProcessPackage.h>
#include <ra/process/PolicyChecker.h>

RA_BEGIN_NAMESPACE(process);

class CheckerWorkItem: public util::WorkItem
{
public:
    CheckerWorkItem();
    ~CheckerWorkItem();

private:
    CheckerWorkItem(const CheckerWorkItem &);
    CheckerWorkItem& operator=(const CheckerWorkItem &);

public:
    bool init(alarm::AlarmManager* alarmManager, tree::ProcessPackagePtr& processPackage);
    /*override*/ void process();

private:
    alarm::AlarmManager* _alarmManager;
    tree::ProcessPackagePtr _processPackage;
    PolicyCheckerPtr _policyChecker;

private:
    friend class CheckerWorkItemTest;
};

RA_TYPEDEF_PTR(CheckerWorkItem);

RA_END_NAMESPACE(process);

#endif //RA_CHECKERWORKITEM_H
