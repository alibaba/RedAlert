#include <errno.h>
#include <ra/util/FileUtil.h>
#include <ra/util/ConfigUtil.h>
#include <ra/common/CommonDefine.h>
#include <ra/config/ConfigDownloader.h>

using namespace std;
RA_USE_NAMESPACE(util);

RA_BEGIN_NAMESPACE(config);
RA_LOG_SETUP(config, ConfigDownloader);

ConfigDownloader::ConfigDownloader() 
    : _version(INVALID_CONFIG_VERSION)
{ 
}

ConfigDownloader::~ConfigDownloader() { 
}

bool ConfigDownloader::init(const string& localConfPath)
{
    _localConfPath = localConfPath;
    if (!ConfigUtil::prepareConfigRoot(_localConfPath)) {
        LOG(ERROR) << "prepare local config root[" 
		   << _localConfPath << "] failed";
    }
    _version = ConfigUtil::getMaxConfigVersion(_localConfPath);
    return true;
}

int32_t ConfigDownloader::downloadConf(const string& versionConfigDir)
{
    int32_t remoteConfVer = ConfigUtil::extractVersion(versionConfigDir);
    if (remoteConfVer == INVALID_CONFIG_VERSION) {
        LOG(ERROR) << "can not extract version from " << versionConfigDir;
        return -1;
    }

    if (!FileUtil::isExist(versionConfigDir)) {
        LOG(ERROR) << "config path '" << versionConfigDir
		   <<"' does not exist";
        return -1;
    }

    if (!FileUtil::isDir(versionConfigDir)) {
        LOG(ERROR) << "config path '" << versionConfigDir << "' is not a folder!";
        return -1;
    }

    if (INVALID_CONFIG_VERSION != _version && remoteConfVer <= _version) {
        LOG(WARNING) << "remote config version[" << remoteConfVer 
		     << "], not bigger than local config version["
		     << _version << "], do nothing!";
        return 0;
    }

    char confDir[1024];
    //make local temp dir
    snprintf(confDir, sizeof(confDir), "%s/XXXXXX", _localConfPath.c_str());
    char* tempDir = mkdtemp(confDir);
    if (NULL == tempDir) {
        LOG(ERROR) << "tempdir not created, errno " << errno;
        return -1;
    }

    int32_t ret = -1;
    do {
        if (!FileUtil::copy(versionConfigDir, tempDir)) {
            LOG(ERROR) << "copy from " << versionConfigDir << " to "
		       << tempDir << " fail";
            break;
        }

        char srcDir[1024];
        snprintf(srcDir, sizeof(srcDir), "%s/%d", tempDir, remoteConfVer);
        char dstDir[1024];
        snprintf(dstDir, sizeof(dstDir), "%s/%d", _localConfPath.c_str(), remoteConfVer);
        if (!FileUtil::move(srcDir, dstDir)) {
            LOG(ERROR) << "move from " << srcDir << " to "
		       << dstDir << " fail";
            break;
        }
        // TODO: add checksum?
        ret = 1;
        _version = remoteConfVer;
    } while(false);

    if (FileUtil::isExist(tempDir)) {
        FileUtil::remove(tempDir);
    }
    return ret;
}

string ConfigDownloader::getConfigFilePath()
{
    if (INVALID_CONFIG_VERSION != _version) {
        return ConfigUtil::getConfigFilePath(_localConfPath, _version);
    }
    return string();
}

RA_END_NAMESPACE(config);

