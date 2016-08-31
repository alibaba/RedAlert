#ifndef RA_CONFIGUTIL_H
#define RA_CONFIGUTIL_H

#include <ra/common/Common.h>
#include <ra/common/Log.h>
#include <string>

RA_BEGIN_NAMESPACE(util);

class ConfigUtil
{
public:
    ConfigUtil();
    ~ConfigUtil();
private:
    ConfigUtil(const ConfigUtil &);
    ConfigUtil& operator=(const ConfigUtil &);
public:
    static int32_t getMaxConfigVersion(const std::string& configRoot);
    static bool prepareConfigRoot(const std::string& configRoot);
    static int32_t extractVersion(const std::string& versionConfigDir);
    static std::string getConfigFilePath(const std::string& configRoot, int32_t configVersion);
};

RA_END_NAMESPACE(util);

#endif //RA_CONFIGUTIL_H
