#ifndef RA_CONFIGDOWNLOADER_H
#define RA_CONFIGDOWNLOADER_H

#include <ra/common/CommonDefine.h>
#include <ra/common/Log.h>

RA_BEGIN_NAMESPACE(config);

class ConfigDownloader
{
public:
    ConfigDownloader();
    ~ConfigDownloader();
private:
    ConfigDownloader(const ConfigDownloader &);
    ConfigDownloader& operator=(const ConfigDownloader &);

public:
    bool init(const std::string& localConfPath);
    //return <0 means error, =0 means no need cache, >0 means ok
    int32_t downloadConf(const std::string& configDir);
    std::string getConfigFilePath();

    int32_t getVersion() const {
        return _version;
    }

private:
    int32_t _version;
    std::string _localConfPath;
};

RA_TYPEDEF_PTR(ConfigDownloader);

RA_END_NAMESPACE(config);

#endif //RA_CONFIGDOWNLOADER_H
