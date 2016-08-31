#include <list>
#include <stack>
#include <ra/util/FileUtil.h>

RA_BEGIN_NAMESPACE(util);
RA_USE_NAMESPACE(fs);

using namespace std;

FileSystemPtr FileUtil::_fs = FileSystemPtr();

FileUtil::FileUtil() { 
}

FileUtil::~FileUtil() { 
}

bool FileUtil::init(FileSystemPtr fs) {
    if (fs == NULL) {
        LOG(INFO) << "File system is NULL";
        return false;
    }
    _fs = fs;
    return true;
}

bool FileUtil::listDir(const string &path,
                       vector<string> &entryVec,
                       bool fileOnly)
{
    return listDir(path, entryVec, fileOnly, false);
}

bool FileUtil::listDir(const std::string &path,
                       std::vector<std::string>& entryVec,
                       bool fileOnly,
                       bool recursive)
{
    entryVec.clear();
    stack<string> dirs;
    dirs.push("");
    while (!dirs.empty()) {
        string relPath = dirs.top();
        dirs.pop();
        string absPath = FileUtil::joinFilePath(path, relPath);
        vector<FileSystem::Entry> children;
        bool ok = _fs->listDir(absPath, children, false);
        if (!ok) {
            LOG(ERROR) << "List dir recursive failed, path [" << absPath <<"]";
            return false;
        }
        for (size_t i = 0; i < children.size(); i++) {
            const FileSystem::Entry& child = children[i];
            string childPath = FileUtil::joinFilePath(relPath, child.name);
            if (!fileOnly || !child.isDir) {
                entryVec.push_back(childPath);
            }
            if (recursive && child.isDir) {
                dirs.push(childPath);
            }
        }
    }
    return true;
}

string FileUtil::joinFilePath(const string &path, const string &file)
{   
    if (path.empty()) {
        return file;
    }
    
    if (path[path.length() - 1] == '/') {
        return path + file;
    }
    
    return path + '/' + file;
}

string FileUtil::normalizeDir(const string &dirName)
{
    string tmpDirName = dirName;
    if (!tmpDirName.empty() && *(tmpDirName.rbegin()) != '/') {
        tmpDirName += "/";
    }
    return tmpDirName;
}

bool FileUtil::isExist(const std::string& path) {
    return _fs->isExist(path);
}

bool FileUtil::mkDir(const string &dirName,  bool recursive)
{
    if (!_fs->isExist(dirName)) {
        if (!_fs->mkDir(dirName, recursive)) {
            LOG(ERROR) << "create dir failed, dir:[" << dirName << "]";
            return false;
        }
        return true;
    }
    if (!_fs->isDirectory(dirName)) {
        LOG(ERROR) << "create dir failed, [" << dirName <<"] exists but not a directory";
        return false;
    }
    return true;
}

bool FileUtil::isDir(const std::string &pathName)
{
    return _fs->isDirectory(pathName);
}

bool FileUtil::removeLocalFile(const std::string &filePath)
{
    return (unlink(filePath.c_str()) == 0);
}

bool FileUtil::localFileExist(const std::string &filePath)
{
    if (access(filePath.c_str(), F_OK) == 0) {
        return true;
    }

    return false;
}

bool FileUtil::readFileContent(const string &filePath, string &content) {
    return _fs->readFileContent(filePath, content);
}

bool FileUtil::copy(const std::string& srcPath, const std::string& dstPath) {
    return _fs->copy(srcPath, dstPath);
}

bool FileUtil::move(const std::string& srcPath, const std::string& dstPath) {
    return _fs->move(srcPath, dstPath);
}

bool FileUtil::remove(const std::string& path) {
    return _fs->remove(path);
}

RA_END_NAMESPACE(util);

