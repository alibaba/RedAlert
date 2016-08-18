#ifndef RA_FILE_SYSTEM_H
#define RA_FILE_SYSTEM_H

#include <ra/common/Common.h>
#include <ra/common/Log.h>

RA_BEGIN_NAMESPACE(fs);

class FileSystem
{
public:
    class Entry
    {
    public:
        bool isDir;
        std::string name;
        Entry(bool isDir_, const std::string& name_): isDir(isDir_), name(name_) { }
    };

public:
    virtual ~FileSystem() { }
    virtual bool readFileContent(const std::string &filePath, std::string &content) = 0;
    virtual bool isExist(const std::string& path) = 0;
    virtual bool isFile(const std::string& path) = 0;
    virtual bool isDirectory(const std::string& path) = 0;
    virtual bool mkDir(const std::string& dirName, bool recursive = false) = 0;
    virtual bool listDir(const std::string &path, std::vector<Entry> &entries, bool fileOnly = false) = 0;
    virtual bool copy(const std::string& srcPath, const std::string& dstPath) = 0;
    virtual bool move(const std::string& srcPath, const std::string& dstPath) = 0;
    virtual bool remove(const std::string& path) = 0;
};

RA_TYPEDEF_PTR(FileSystem);

RA_END_NAMESPACE(fs);

#endif //RA_FILE_SYSTEM_H
