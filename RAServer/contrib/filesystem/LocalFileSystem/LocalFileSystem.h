#ifndef RA_LOCAL_FILE_SYSTEM_H
#define RA_LOCAL_FILE_SYSTEM_H

#include <ra/common/Common.h>
#include <ra/common/Log.h>
#include <contrib/filesystem/FileSystem.h>

RA_BEGIN_NAMESPACE(fs);

class LocalFileSystem: public FileSystem {
public:
    LocalFileSystem();
    virtual ~LocalFileSystem();

    bool readFileContent(const std::string &filePath, std::string &content);
    bool isExist(const std::string& path);
    bool isFile(const std::string& path);
    bool isDirectory(const std::string& path);
    bool mkDir(const std::string& dirName, bool recursive = false);
    bool listDir(const std::string &path, std::vector<Entry> &entries, bool fileOnly = false);
    bool copy(const std::string& srcPath, const std::string& dstPath);
    bool move(const std::string& srcPath, const std::string& dstPath);
    bool remove(const std::string& path);

private:
    bool mkDirInternal(const std::string& dirName);
    bool _copyFile(const std::string &src, const std::string &dest);
    bool _copyDir(const std::string &src, const std::string &dest);
private:
    RA_LOG_DECLARE();
};

RA_END_NAMESPACE(fs);

#endif /* RA_LOCAL_FILE_SYSTEM_H */

