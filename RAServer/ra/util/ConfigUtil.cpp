#include <vector>
#include <ra/util/FileUtil.h>
#include <ra/util/ConfigUtil.h>
#include <ra/common/CommonDefine.h>

RA_BEGIN_NAMESPACE(util);

using namespace std;

ConfigUtil::ConfigUtil() { 
}

ConfigUtil::~ConfigUtil() { 
}

int32_t ConfigUtil::getMaxConfigVersion(const string& configRoot)
{
    if(!FileUtil::isDir(configRoot)) {
        LOG(ERROR) << "assert config dir[" << configRoot << "] is dir failed!";
        return false;
    }
    vector<string> entryVec;
    if (!FileUtil::listDir(configRoot, entryVec, false)) {
        LOG(ERROR) << "list config dir[" << configRoot << "] failed!";
        return false;
    }
    int ret = INVALID_CONFIG_VERSION;
    for (vector<string>::const_iterator it = entryVec.begin(); 
         it != entryVec.end(); ++it) 
    {
        string versionDir = FileUtil::joinFilePath(configRoot, *it);
        if(!FileUtil::isDir(versionDir)) {
            continue;
        }
        int32_t ver = extractVersion(versionDir);
        if (INVALID_CONFIG_VERSION == ver) {
            continue;
        }
        if (INVALID_CONFIG_VERSION == ret || ver > ret) {
            ret = ver;
        }
    }
    return ret;
}

bool ConfigUtil::prepareConfigRoot(const string& configRoot)
{
    if (!FileUtil::isExist(configRoot)) {
        if (!FileUtil::mkDir(configRoot, false)) {
            LOG(ERROR) << "mkdir config root dir[" << configRoot << "] failed";
            return false;
        }
    }
    if (!FileUtil::isDir(configRoot)) {
        LOG(ERROR) << "path [" << configRoot << "] exists but not a folder";
        return false;
    }
    return true;
}

int32_t ConfigUtil::extractVersion(const string& versionConfigDir)
{
    string::size_type pos = versionConfigDir.find_last_not_of("/");
    string tmp(versionConfigDir);
    tmp.erase(pos + 1);
    pos = tmp.find_last_of("/");
    tmp.erase(0, pos+1);
    if (string::npos != tmp.find_first_not_of("0123456789")) {
        return INVALID_CONFIG_VERSION;
    }
    return strtol(tmp.c_str(), NULL, 10);
}

string ConfigUtil::getConfigFilePath(const string& configRoot, int32_t configVersion)
{
    char localPath[256];
    snprintf(localPath, sizeof(localPath), "%s/%d/%s",
             configRoot.c_str(), configVersion, CONF_FILE_NAME.c_str());
    return localPath;
}

RA_END_NAMESPACE(util);

