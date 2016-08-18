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
        RA_LOG(ERROR, "prepare local config root[%s] failed", _localConfPath.c_str());
    }
    _version = ConfigUtil::getMaxConfigVersion(_localConfPath);
    return true;
}

int32_t ConfigDownloader::downloadConf(const string& versionConfigDir)
{
    int32_t remoteConfVer = ConfigUtil::extractVersion(versionConfigDir);
    if (remoteConfVer == INVALID_CONFIG_VERSION) {
        RA_LOG(ERROR, "can not extract version from %s", versionConfigDir.c_str());
        return -1;
    }

    if (!FileUtil::isExist(versionConfigDir)) {
        RA_LOG(ERROR, "config path '%s' does not exist", versionConfigDir.c_str());
        return -1;
    }

    if (!FileUtil::isDir(versionConfigDir)) {
        RA_LOG(ERROR, "config path '%s' is not a folder!", versionConfigDir.c_str());
        return -1;
    }

    if (INVALID_CONFIG_VERSION != _version && remoteConfVer <= _version) {
        RA_LOG(WARN, "remote config version[%d], not bigger than local "
               "config version[%d], do nothing!", remoteConfVer, _version);
        return 0;
    }

    char confDir[1024];
    //make local temp dir
    snprintf(confDir, sizeof(confDir), "%s/XXXXXX", _localConfPath.c_str());
    char* tempDir = mkdtemp(confDir);
    if (NULL == tempDir) {
        RA_LOG(ERROR, "tempdir not created, errno %d", errno);
        return -1;
    }

    int32_t ret = -1;
    do {
        if (!FileUtil::copy(versionConfigDir, tempDir)) {
            RA_LOG(ERROR, "copy from %s to %s fail", versionConfigDir.c_str(), tempDir);
            break;
        }

        char srcDir[1024];
        snprintf(srcDir, sizeof(srcDir), "%s/%d", tempDir, remoteConfVer);
        char dstDir[1024];
        snprintf(dstDir, sizeof(dstDir), "%s/%d", _localConfPath.c_str(), remoteConfVer);
        if (!FileUtil::move(srcDir, dstDir)) {
            RA_LOG(ERROR, "move from %s to %s fail", srcDir, dstDir);
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

