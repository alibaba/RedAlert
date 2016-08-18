#include <stdlib.h>
#include <errno.h>
#include <unistd.h>
#include <getopt.h>
#include <signal.h>
#include <sstream>
#include <curl/curl.h>
#include <contrib/filesystem/FileSystemFactory.h>
#include <ra/app/RaApp.h>
#include <ra/common/Json.h>
#include <ra/common/ErrorDefine.h>
#include <ra/common/CommonDefine.h>
#include <ra/util/ConfigUtil.h>
#include <ra/util/FileUtil.h>
#include <ra/util/Util.h>
#define DELETE_AND_SET_NULL(x) do {\
                                    if ((x) != NULL) { \
                                        delete (x);\
                                        (x) = NULL;\
                                    } \
                               } while (false) 
 
RA_USE_NAMESPACE(common);
RA_USE_NAMESPACE(config);
RA_USE_NAMESPACE(util);
RA_USE_NAMESPACE(fs);
RA_USE_NAMESPACE(service);
RA_USE_NAMESPACE(tree);
RA_USE_NAMESPACE(process);
RA_USE_NAMESPACE(alarm);

RA_BEGIN_NAMESPACE(app);
RA_LOG_SETUP(app, RaApp);
using namespace std;
volatile bool isStop = false;
static void handleSignal(int sig) {
    isStop = true;
}

static void registerSignalHandler() {
    signal(SIGINT, handleSignal);
    signal(SIGTERM, handleSignal);
    signal(SIGUSR1, handleSignal);
    signal(SIGUSR2, handleSignal);
}

RaApp::RaApp()
    : _httpServer(NULL)
    , _loadConfigHandler(NULL)
    , _listMetricHandler(NULL)
    , _heartbeatClient(NULL)
    , _port(0)
    , _fileSystem(DEFAULT_FILE_SYSTEM)
{
}

RaApp::~RaApp() {
    DELETE_AND_SET_NULL(_httpServer);
    DELETE_AND_SET_NULL(_loadConfigHandler);
    DELETE_AND_SET_NULL(_listMetricHandler);
    DELETE_AND_SET_NULL(_heartbeatClient);
}

bool RaApp::init(int argc, char** argv) {
    if (!parseCmdLineOptions(argc, argv)) {
        printUsage(argv[0]);
        return false;
    }

    if (!_workDir.empty() && chdir(_workDir.c_str()) < 0) {
        fprintf(stderr, "chdir to %s fail, errno: %d\n", _workDir.c_str(), errno);
        return false;
    }

    if (!getCwd(_workDir)) {
        fprintf(stderr, "cannot get current work directory\n");
        return false;
    }

    string consoleLogConf = "log4cplus.rootLogger=INFO, raAppender\n\
log4cplus.appender.raAppender=log4cplus::ConsoleAppender\n\
log4cplus.appender.raAppender.layout=log4cplus::PatternLayout\n\
log4cplus.appender.raAppender.layout.ConversionPattern=[%D{%Y-%m-%d %H:%M:%Q}] [%p] [%T,%F -- %M:%L] %m%n";
    istringstream confStream(consoleLogConf);
    RA_LOG_CONFIG_WITH_CONTENT(confStream);

    FileSystemPtr fs = FileSystemFactory::create(_fileSystem);
    if (fs == NULL || !FileUtil::init(fs)) {
        RA_LOG(ERROR, "initialize file util fail, file system: '%s'\n", _fileSystem.c_str());
        return false;
    }

    if (!initLog()) {
        RA_LOG(ERROR, "initialize ra log failed\n");
        return false;
    }

    if (!loadBootstrapConf(_bootstrapConfFile)) {
        RA_LOG(ERROR, "load bootstrap config failed, path: '%s'", _bootstrapConfFile.c_str());
        return false;
    }

    const string& hbServerHost = _bootstrapConfig.getHeartbeatHost();
    uint16_t hbServerPort = _bootstrapConfig.getHeartbeatPort();
    string hostStr;
    if (!Util::getLocalAddress(hbServerHost, hbServerPort, hostStr)) {
        RA_LOG(ERROR, "get local address failed");
        return false;
    }
    stringstream portStr;
    portStr << _port;
    _localAddress = hostStr + ":" + portStr.str();

    if (!_alarmManager.init(_bootstrapConfig.getAlarmThreadNum(),
                            _bootstrapConfig.getAlarmQueueSize(),
                            _bootstrapConfig.getAliMonitorUrl(),
                            _bootstrapConfig.getMailUser(),
                            _bootstrapConfig.getMailPwd(),
                            _bootstrapConfig.getMailServerUrl()))
    {
        RA_LOG(ERROR, "init alarmManager failed");
        return false;
    }
    if (!_checkerManager.init(_bootstrapConfig.getCheckerThreadNum(),
                              _bootstrapConfig.getCheckerQueueSize(),
                              &_alarmManager))
    {
        RA_LOG(ERROR, "init checkerManager failed");
        return false;
    }
    if (!_fetcherManager.init(&_alarmManager, 
            _bootstrapConfig.getFetcherThreadNum(),
            _bootstrapConfig.getFetcherQueueSize(), 
            _bootstrapConfig.getInternalAlarmGroup(),
            _bootstrapConfig.getInternalAlarmLevel(),
            _bootstrapConfig.getInteralMinAlarmInterval(),
            _bootstrapConfig.getRetrieveMetricsPointCount(),
            &_checkerManager
            ))
    {
        RA_LOG(ERROR, "init fetcherManager failed");
        return false;
    }
    if (!_treeManager.init(this, &_fetcherManager,
                           _bootstrapConfig.getReloadTreeIntervalSec(),
                           _bootstrapConfig.getMaxRandomLastRetriveTimeValueSec()))
    {
        RA_LOG(ERROR, "init treeManager failed");
        return false;
    }
    
    _httpServer = new common::HttpServer();
    if (_httpServer == NULL || !_httpServer->init("0.0.0.0", _port)) {
        RA_LOG(ERROR, "initialize http server failed");
        return false;
    }
    _listMetricHandler = new ListMetricHandler(&_treeManager);
    _loadConfigHandler = new LoadConfigHandler(this);
    if (!_httpServer->registerHandler(HTTP_API_PATH_LIST_METRIC, _listMetricHandler)) {
        RA_LOG(ERROR, "register handler for http path '%s' failed", HTTP_API_PATH_LIST_METRIC.c_str());
        return false;
    }
    if (!_httpServer->registerHandler(HTTP_API_PATH_LOAD_CONFIG, _loadConfigHandler)) {
        RA_LOG(ERROR, "register handler for http path '%s' failed", HTTP_API_PATH_LOAD_CONFIG.c_str());
        return false;
    }

    _heartbeatClient = new HeartbeatClient(this, hbServerHost, hbServerPort, _localAddress);
    if (_heartbeatClient == NULL || !_heartbeatClient->init()) {
        RA_LOG(ERROR, "initialize heartbeat client failed");
        return false;
    }

    //auto load Config
    if (!loadConfig()) {
        RA_LOG(ERROR, "load local config when init failed");
    }

    RA_LOG(INFO, "App init success");
    return true;
}

bool RaApp::run() {
    registerSignalHandler();
    if (!start()) {
        RA_LOG(ERROR, "start app fail");
        return false;
    }

    while (!isStop) {
        int64_t curTimeUs = Util::currentTimeInMicroseconds();
        TreeManager::ProcessPackageMapPtr processPackageMapPtr = _treeManager.getProcessPackageMap();
        if (NULL != processPackageMapPtr) {
            TreeManager::ProcessPackageMap& processPackageMap = *processPackageMapPtr;
            TreeManager::ProcessPackageMap::iterator it = processPackageMap.begin();
            for (; it != processPackageMap.end(); ++it) {
                ProcessPackagePtr& processPackage = it->second;
                if (processPackage->isReady(curTimeUs)) {
                    processPackage->setState(ProcessPackage::PPS_PROCESSING);
                    _fetcherManager.pushProcessPackage(processPackage);
                }
                else if (processPackage->getState() == ProcessPackage::PPS_IDLE) {
                    checkLastAlarmTime(processPackage);
                }
            }
        }
        usleep(APP_LOOP_INTERVAL_US);
    }

    stop();
    return true;
}

bool RaApp::start() {
    curl_global_init(CURL_GLOBAL_ALL);

    if (!_httpServer->start()) {
        RA_LOG(ERROR, "start http server '%s:%d' fail", "0.0.0.0", _port);
        return false;
    }
    RA_LOG(INFO, "start http server '%s:%d' succ", "0.0.0.0", _port);

    if (!_alarmManager.start()) {
        RA_LOG(ERROR, "alarmManager start failed");
        return false;
    }
    if (!_checkerManager.start()) {
        RA_LOG(ERROR, "checkerManager start failed");
        return false;
    }
    if (!_fetcherManager.start()) {
        RA_LOG(ERROR, "fetcherManager start failed");
        return false;
    }
    if (!_treeManager.start()) {
        RA_LOG(ERROR, "treeManager start failed");
        return false;
    }
    if (!_heartbeatClient->start()) {
        RA_LOG(ERROR, "heartbeatClient start failed");
        return false;
    }
    RA_LOG(INFO, "RaApp start succ!");
    return true;
}

bool RaApp::stop() {
    RA_LOG(INFO, "RaApp stop");
    if (!_httpServer->stop()) {
        RA_LOG(ERROR, "stop http server '%s:%d' fail", "0.0.0.0", _port);
    }
    if (!_heartbeatClient->stop()) {
        RA_LOG(ERROR, "stop heartbeat client");
    }
    _alarmManager.stop();
    _checkerManager.stop();
    _fetcherManager.stop();
    _treeManager.stop();
    _configWrapper.reset();
    RA_LOG(INFO, "RA server stops");
    isStop = true;
    return true;
}

bool RaApp::parseCmdLineOptions(int argc, char** argv) {
    static struct option long_options[] = {
        {"port",     required_argument, 0, 'p' },
        {"fileSys",  optional_argument, 0, 'f' },
        {"logConf",  optional_argument, 0, 'l' },
        {"bootConf", optional_argument, 0, 'c' },
        {"workDir",  optional_argument, 0, 'd' },
        {0,          0,                 0,  0  }
    };
    optind = 0;
    int port = -1;
    while (1) {
        int opt = getopt_long(argc, argv, "p:f:l:c:d:", long_options, NULL);
        if (opt == -1) break;
        switch (opt) {
            case 'p':
                port = atoi(optarg);
                if (port < 0) {
                    fprintf(stderr, "error: port number is negative: %d\n", port);
                    return false;
                }
                _port = port;
                printf("port number: %u\n", _port);
                break;
            case 'f':
                if (optarg) {
                    _fileSystem = string(optarg);
                    printf("file system: '%s'\n", _fileSystem.c_str());
                }
                break;
            case 'l':
                if (optarg) {
                    _logConfFile = string(optarg);
                    printf("log config: '%s'\n", _logConfFile.c_str());
                }
                break;
            case 'c':
                if (optarg) {
                    _bootstrapConfFile = string(optarg);
                    printf("bootstrap config: '%s'\n", _bootstrapConfFile.c_str());
                }
                break;
            case 'd':
                if (optarg) {
                    _workDir = string(optarg);
                    printf("work directory: '%s'\n", _workDir.c_str());
                }
                break;
            default:
                fprintf(stderr, "error: invalid option: %d\n", opt);
                return false;
                break;
        }
    }
    if (port <= 0) {
        fprintf(stderr, "error: port number is not specified\n");
        return false;
    }
    if (_bootstrapConfFile.empty()) {
        fprintf(stderr, "bootstrap file path is empty\n");
        return false;
    }
    return true;
}

void RaApp::printUsage(const char* cmd) {
    assert(NULL != cmd);
    fprintf(stderr, "usage: %s\n"
            "-p, --port <port number> (required)\n"
            "-c, --confDir <absolute path to ra config> (required)\n"
            "-l, --logConf <absolute path to log config>\n"
            "-d, --workDir <absolute path to work directory>\n"
            "-f, --fileSys <file system>\n",
            cmd);
}

bool RaApp::initLog() {
    if (_logConfFile.empty()) {
        RA_LOG(INFO, "log config file path is empty, skipping configuration\n");
        return true;
    }
    string fileContent;
    if (!FileUtil::readFileContent(_logConfFile, fileContent)) {
        RA_LOG(WARN, "cannot read content of log config, file path: '%s'\n", _logConfFile.c_str());
        return false;
    }
    istringstream istream(fileContent);
    RA_LOG_CONFIG_WITH_CONTENT(istream);
    return true;
}

bool RaApp::getCwd(std::string& cwd) {
    long path_max;
    size_t size;
    char *buf = NULL;
    char *ptr = NULL;

    path_max = pathconf(".", _PC_PATH_MAX);
    if (path_max == -1) {
        size = 1024;
    } else if (path_max > 10240) {
        size = 10240;
    } else {
        size = path_max;
    }

    for (buf = ptr = NULL; ptr == NULL; size *= 2) {
        if ((buf = (char *)realloc(buf, size)) == NULL) {
            RA_LOG(ERROR, "cannot allocate memory for cwd buffer");
            return false;
        }
        ptr = getcwd(buf, size);
        if (ptr == NULL && errno != ERANGE) {
            RA_LOG(ERROR, "cannot get cwd, errno: %d", errno);
            return false;
        }
    }
    cwd.assign(ptr);
    free(buf);
    return true;
}

bool RaApp::loadConfig()
{
    const string& localConfPath = getLocalConfPath();
    if (!ConfigUtil::prepareConfigRoot(localConfPath)) {
        RA_LOG(ERROR, "prepare local config root error, load config failed");
        return false;
    }
    int32_t configVersion = ConfigUtil::getMaxConfigVersion(localConfPath);
    return loadConfig(configVersion);
}

bool RaApp::loadConfig(int32_t configVersion)
{
    if (INVALID_CONFIG_VERSION == configVersion) {
        RA_LOG(ERROR, "can not load invalid config version");
        return false;
    }

    if (NULL != _configWrapper && _configWrapper->getConfigVersion() == configVersion) {
        RA_LOG(WARN, "ra is loading config of version == current config version, do nothing!");
        return true;
    }

    if (NULL != _configWrapper && _configWrapper->getConfigVersion() > configVersion) {
        RA_LOG(WARN, "ra is loading config of version smaller than current config version");
    }

    ConfigWrapperPtr tmpConfigWrapper(new ConfigWrapper);
    const string& localConfPath = getLocalConfPath();
    bool ret = tmpConfigWrapper->loadConfig(localConfPath, configVersion, _localAddress);
    if (!ret) {
        RA_LOG(ERROR, "load config failed, config root[%s], configVersion[%d]", 
               localConfPath.c_str(), configVersion);
        return false;
    }
    setConfigWrapper(tmpConfigWrapper);
    return true;
}

bool RaApp::loadBootstrapConf(const string& bootstrapPath)
{
    std::string content;
    if (!FileUtil::readFileContent(bootstrapPath, content)) {
        RA_LOG(ERROR, "read bootstrap file[%s] failed", bootstrapPath.c_str());
        return false;
    }
    JsonPtr json = Json::load(content);
    if (!fromJson(json, _bootstrapConfig)) {
        RA_LOG(ERROR, "cannot load bootstrap file[%s]", bootstrapPath.c_str());
        return false;
    }
    return true;
}

void RaApp::checkLastAlarmTime(ProcessPackagePtr& processPackage)
{
    assert(processPackage);
    int64_t currTimeSec = Util::currentTimeInSeconds();
    int64_t lastNonOkAlarmTimeSec = processPackage->getLastNonOkAlarmTimeSec();
    if (INVALID_TIME != lastNonOkAlarmTimeSec && 
        currTimeSec - lastNonOkAlarmTimeSec > _bootstrapConfig.getSendOkCodeIntervalSec())
    {
        processPackage->setState(ProcessPackage::PPS_PROCESSING);
        alarm::AlarmMsgPtr _alarmMsg(new AlarmMsg(processPackage, CODE_OK));
        _alarmManager.pushAlarmMessage(_alarmMsg);
    }
}

RA_END_NAMESPACE(app);

