#ifndef RA_FILE_SYSTEM_FACTORY_H
#define RA_FILE_SYSTEM_FACTORY_H

#include <string>
#include <ra/common/Common.h>
#include <ra/common/Log.h>
#include <contrib/filesystem/FileSystem.h>

RA_BEGIN_NAMESPACE(fs);

class FileSystemFactory {
public:
    static FileSystemPtr create(std::string type);

private:
    FileSystemFactory(const FileSystemFactory &);
    FileSystemFactory& operator=(const FileSystemFactory &);

private:
    RA_LOG_DECLARE();
};

RA_END_NAMESPACE(fs);

#endif /* RA_FILE_SYSTEM_FACTORY_H */

