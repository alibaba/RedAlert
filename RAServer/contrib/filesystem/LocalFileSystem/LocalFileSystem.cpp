#include <stdio.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <dirent.h> 
#include <assert.h>
#include <errno.h>
#include <ra/util/Util.h>
#include "LocalFileSystem.h"
using namespace std;
RA_BEGIN_NAMESPACE(fs);
RA_USE_NAMESPACE(util);
RA_LOG_SETUP(fs, LocalFileSystem);

LocalFileSystem::LocalFileSystem() {
}

LocalFileSystem::~LocalFileSystem() {
}

bool LocalFileSystem::readFileContent(const std::string& filePath, std::string& content) {
    FILE *pFile = fopen(filePath.c_str(), "r");
    if (!pFile) {
        RA_LOG(ERROR, "Fail to open %s", filePath.c_str());
        return false;
    }
    char *pBuf;  
    int retCode = fseek(pFile, 0, SEEK_END); 
    if (retCode) {
        RA_LOG(ERROR, "Fail to seek end %s", filePath.c_str());
        return false;
    }
    int len = ftell(pFile); 
    RA_LOG(ERROR, "Length = %d", len);
    if (len > 1024 * 1024 * 10 || len < 0) {
        RA_LOG(ERROR, "Too long[%d] to read.", len);
        return false;
    }
    pBuf = new char[len+1]; 
    rewind(pFile); 
    fread(pBuf, 1, len, pFile); 
    pBuf[len] = 0; 
    content = pBuf;
    fclose(pFile); 
    delete pBuf;
    return true;
}

bool LocalFileSystem::isExist(const std::string& path) {
    return access(path.c_str(), F_OK) == 0;
}

bool LocalFileSystem::isFile(const std::string& path) {
    struct stat buf;
    if (stat(path.c_str(), &buf) < 0) {
        return false;
    }

    if(S_ISREG(buf.st_mode)) {
        return true;
    }

    return false;
}

bool LocalFileSystem::isDirectory(const std::string& path) {
    struct stat buf;
    if (stat(path.c_str(), &buf) < 0) {
        return false;
    }

    if (S_ISDIR(buf.st_mode)) {
        return true;
    }

    return false;
}

bool LocalFileSystem::mkDirInternal(const string& dirName) {
    size_t pos = dirName.rfind('/');
    if (pos == string::npos) {
        if (mkdir(dirName.c_str(), 0755) < 0) {
            if (errno != EEXIST) {
                RA_LOG(ERROR, "create directory %s fail, %s", 
                        dirName.c_str(), strerror(errno));
                return false;
            }
        }
        return true;
    }

    string parentDir = dirName.substr(0, pos);
    if (!parentDir.empty() && access(parentDir.c_str(), F_OK) != 0) {
        if (!mkDirInternal(parentDir)) {
            return false;
        }
    }
    
    if (mkdir(dirName.c_str(), 0755) < 0) {
        if (errno != EEXIST) {
            RA_LOG(ERROR, "create directory %s fail, %s", 
                      dirName.c_str(), strerror(errno));
            return false;
        }
    }
    
    return true;
}

bool LocalFileSystem::mkDir(const std::string& dirName, bool recursive) {
    string dir = dirName;
    size_t len = dir.size();
    if (dir[len - 1] == '/') {
        if (len == 1) {
            RA_LOG(ERROR, "directory %s already exist", dir.c_str());
            return false;
        } else {
            dir.resize(len - 1);
        }
    }

    if (access(dir.c_str(), F_OK) == 0) {
        RA_LOG(ERROR, "directory %s already exist", dirName.c_str());
        return false;
    }

    size_t pos = dir.rfind('/');
    if (pos == string::npos) {
        if (mkdir(dir.c_str(), 0755) < 0) {
            RA_LOG(ERROR, "create directory %s fail, %s", dir.c_str(), 
                      strerror(errno));
            return false;
        }
        return true;
    }
    string parentDir = dir.substr(0, pos);
    if (!parentDir.empty() && access(parentDir.c_str(), F_OK) != 0) {
        if (recursive) {
            if (!mkDirInternal(parentDir)) {
                RA_LOG(ERROR, "create directory %s fail, fail to create "
                        "parent dir", dir.c_str());
                return false;
            }
        } else {
            RA_LOG(ERROR, "create directory %s fail, parameter -p is needed",                       dir.c_str());
            return false;
        }
    }

    if (mkdir(dir.c_str(), 0755) < 0) {
        RA_LOG(ERROR, "create directory %s fail, %s", 
               dir.c_str(), strerror(errno));
        return false;
    }

    return true;
}

bool LocalFileSystem::listDir(const std::string& path, std::vector<Entry>& entries, bool fileOnly) {
    DIR* dp; 
    struct dirent* ep;
    dp = opendir(path.c_str()); 
    if (dp == NULL) {
        RA_LOG(ERROR, "open dir %s fail, %s", 
               path.c_str(), strerror(errno));
        return false;
    }
    
    while ((ep = readdir(dp)) != NULL) {
        if (strcmp(ep->d_name, "." ) == 0 || strcmp(ep->d_name, "..") == 0) {
            continue;
        }
        string xPath  = path + "/" + ep->d_name;
        bool isDir = isDirectory(xPath);
        if (!fileOnly || (fileOnly && !isDir)) {
            Entry entry(isDir, string(ep->d_name));
            entries.push_back(entry);
        }
    } 
    if (closedir(dp) < 0) {
        RA_LOG(ERROR, "close dir %s fail, %s", 
               path.c_str(), strerror(errno));
        return false;
    }
    
    return true;

}

bool LocalFileSystem::_copyDir(const string &src, const string &dest) {
    string xDest;
    if (!isDirectory(dest)) {
        return false;
    }
    if (!isExist(dest)) {
        if (!mkDir(dest, true)) {
            return false;
        }
        xDest = dest;
    } else {
        vector<string> parts = Util::splitString(src,"/");
        string baseName = parts[parts.size() - 1];
        string expectDir = dest + "/" + baseName;
        if (!isExist(expectDir)) {
            if (!mkDir(expectDir)) {
                return false;
            }
        }
        xDest = expectDir;
    }
    vector<Entry> entries;
    if (!listDir(src, entries, false)) {
        return false;
    }
    for (size_t i = 0; i < entries.size(); ++i) {
        string fileName = entries[i].name;
        string fullPath = src + "/" + fileName;
        if (isDirectory(fullPath)) {
            if(!_copyDir(fullPath, xDest)) {
                return false;
            }
        } else {
            if (!_copyFile(fullPath, xDest)) {
                return false;
            }
        }
    }
    return true;
}

bool LocalFileSystem::_copyFile(const string &src, const string &dest)
{
    int fd_to, fd_from;
    char buf[4096];
    ssize_t nread;
    const char *from = src.c_str();
    string dstFullPath;
    if (isDirectory(dest)) {
        vector<string> parts = Util::splitString(src, "/");
        string baseName = parts[parts.size() - 1];
        dstFullPath = dest + "/" + baseName;
    } else {
        dstFullPath = dest;
    }
    const char *to = dstFullPath.c_str();

    fd_from = open(from, O_RDONLY);
    if (fd_from < 0)
        return false;

    fd_to = open(to, O_WRONLY | O_CREAT | O_EXCL, 0666);
    if (fd_to < 0)
        return false;

    while (nread = read(fd_from, buf, sizeof buf), nread > 0)
    {
        char *out_ptr = buf;
        ssize_t nwritten;

        do {
            nwritten = write(fd_to, out_ptr, nread);

            if (nwritten >= 0)
            {
                nread -= nwritten;
                out_ptr += nwritten;
            }
            else if (errno != EINTR)
            {
                return false;
            }
        } while (nread > 0);
    }

    if (nread == 0)
    {
        if (close(fd_to) < 0)
        {
            fd_to = -1;
            return false;
        }
        close(fd_from);

        /* Success! */
        return true;
    }
    return false;
}

bool LocalFileSystem::copy(const std::string& srcPath, const std::string& dstPath) {
    if(isDirectory(srcPath)) {
        return _copyDir(srcPath, dstPath);
    }

    return _copyFile(srcPath, dstPath);
}

bool LocalFileSystem::move(const std::string& srcPath, const std::string& dstPath) {
    return rename(srcPath.c_str(), dstPath.c_str()) == 0;
}

bool LocalFileSystem::remove(const std::string& path) {
    if(isDirectory(path)) {
        return rmdir(path.c_str()) == 0;
    }

    return remove(path.c_str()) == 0;
}

RA_END_NAMESPACE(fs);
