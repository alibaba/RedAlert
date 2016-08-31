#ifndef RA_RAAPP_H
#define RA_RAAPP_H

#include <ra/common/CommonDefine.h>
#include <ra/common/Log.h>
#include <ra/common/HttpServer.h>
#include <ra/util/FileUtil.h>
#include <ra/util/ThreadLock.h>
#include <ra/config/BootstrapConfig.h>
#include <ra/config/ConfigWrapper.h>
#include <ra/tree/TreeManager.h>
#include <ra/alarm/AlarmManager.h>
#include <ra/process/FetcherManager.h>
#include <ra/process/CheckerManager.h>
#include <ra/service/ListMetricHandler.h>
#include <ra/service/LoadConfigHandler.h>
#include <ra/service/HeartbeatClient.h>

RA_BEGIN_NAMESPACE(tree);
class TreeManagerTest;
RA_END_NAMESPACE(tree);

RA_BEGIN_NAMESPACE(service);
class HeartbeatClientTest;
class LoadConfigHandlerTest;
RA_END_NAMESPACE(service);

RA_BEGIN_NAMESPACE(app);

class RaApp
{
public:
    RaApp();
    virtual ~RaApp();
private:
    RaApp(const RaApp &);
    RaApp& operator=(const RaApp &);
public:
    bool init(int argc, char** argv);
    bool run();

    bool start();
    bool stop();

    bool loadConfig(int32_t configVersion);

    const config::ConfigWrapperPtr getConfigWrapper() {
        util::ScopedLock lock(_configLock);
        return _configWrapper;
    }

    std::string getLocalConfPath() const {
        return util::FileUtil::joinFilePath(_workDir, LOCAL_CONF_ROOT);
    }

    const std::string& getLocalAddress() const
    {
        return _localAddress;
    }

private:
    bool getCwd(std::string& cwd);
    bool loadConfig();
    bool loadBootstrapConf(const std::string& bootstrapPath);
    void setConfigWrapper(config::ConfigWrapperPtr &configWrapper) {
        util::ScopedLock lock(_configLock);
        _configWrapper = configWrapper;
    }

    bool parseCmdLineOptions(int argc, char** argv);
    bool initLog();
    void checkLastAlarmTime(tree::ProcessPackagePtr& processPackage);
    void printUsage(const char* cmd);
private:
    common::HttpServer *_httpServer;
    service::LoadConfigHandler *_loadConfigHandler;
    service::ListMetricHandler *_listMetricHandler;
    service::HeartbeatClient *_heartbeatClient;
    uint32_t _port;
    std::string _localAddress;
    std::string _fileSystem;
    std::string _logConfFile;
    std::string _bootstrapConfFile;
    std::string _workDir;
    config::BootstrapConfig _bootstrapConfig;

    mutable util::Mutex _configLock;
    config::ConfigWrapperPtr _configWrapper;

    tree::TreeManager _treeManager;
    process::FetcherManager _fetcherManager;
    process::CheckerManager _checkerManager;
    alarm::AlarmManager _alarmManager;
private:
    friend class RaAppTest;
    friend class tree::TreeManagerTest;
    friend class service::HeartbeatClientTest;
    friend class service::LoadConfigHandlerTest;
};

RA_TYPEDEF_PTR(RaApp);

RA_END_NAMESPACE(app);

#endif //RA_RAAPP_H
