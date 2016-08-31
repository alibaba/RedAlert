#ifndef RA_FETCHERWORKITEM_H
#define RA_FETCHERWORKITEM_H

#include <ra/common/Common.h>
#include <ra/common/Log.h>
#include <ra/util/ThreadPool.h>
#include <ra/tree/ProcessPackage.h>
#include <ra/process/FetcherManager.h>
#include <ra/process/CheckerManager.h>

RA_BEGIN_NAMESPACE(process);

class FetcherWorkItem: public util::WorkItem
{
public:
    FetcherWorkItem(FetcherManager* fetcherManager, 
                    tree::ProcessPackagePtr& processPackage, 
                    CheckerManager *checkerManager,
                    alarm::AlarmManager* alarmManager);
    ~FetcherWorkItem();

private:
    FetcherWorkItem(const FetcherWorkItem &);
    FetcherWorkItem& operator=(const FetcherWorkItem &);

public:
    /*override*/ void process();

private:
    tree::ProcessPackagePtr _processPackage;
    FetcherManager* _fetcherManager;
    CheckerManager *_checkerManager;
    alarm::AlarmManager* _alarmManager;

private:
    friend class FetcherWorkItemTest;
};

RA_TYPEDEF_PTR(FetcherWorkItem);

RA_END_NAMESPACE(process);

#endif //RA_FETCHERWORKITEM_H
