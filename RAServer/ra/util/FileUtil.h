#ifndef RA_FILEUTIL_H
#define RA_FILEUTIL_H

#include <string>
#include <vector>
#include <ra/common/Common.h>
#include <ra/common/Log.h>
#include <contrib/filesystem/FileSystem.h>

RA_BEGIN_NAMESPACE(util);

class FileUtil
{
public:
    FileUtil();
    ~FileUtil();
private:
    FileUtil(const FileUtil &);
    FileUtil& operator=(const FileUtil &);

public:
    static bool init(ra::fs::FileSystemPtr fs);
    static bool readFileContent(const std::string &filePath, std::string &content);
    static std::string joinFilePath(const std::string &dir,
            const std::string &file);
    static std::string normalizeDir(const std::string &dirName);
    static bool listDir(const std::string &path,
                        std::vector<std::string> &entryVec,
                        bool fileOnly = false);

    //TODO: not support hadoop and zfs, pangu
    static bool isExist(const std::string& path);
    static bool mkDir(const std::string &dirName,  bool recursive = true);
    static bool isDir(const std::string &pathName);
    static bool removeLocalFile(const std::string &filePath);
    static bool localFileExist(const std::string &filePath);
    static bool copy(const std::string& srcPath, const std::string& dstPath);
    static bool move(const std::string& srcPath, const std::string& dstPath);
    static bool remove(const std::string& path);

private:
    static bool listDir(const std::string &path,
                        std::vector<std::string>& entryVec,
                        bool fileOnly,
                        bool recursive);

private:
    RA_LOG_DECLARE();
    static const char DIR_DELIM = '/';
    static ra::fs::FileSystemPtr _fs;
};

RA_END_NAMESPACE(util);

#endif //RA_FILEUTIL_H
