#include <contrib/filesystem/FileSystemFactory.h>
#include <contrib/filesystem/LocalFileSystem/LocalFileSystem.h>

using namespace std;

RA_BEGIN_NAMESPACE(fs);
RA_LOG_SETUP(fs, FileSystemFactory);

FileSystemPtr FileSystemFactory::create(string type) {
    if (type == "LocalFileSystem") {
        return FileSystemPtr(new LocalFileSystem());
    }
    RA_LOG(WARN, "Unknown filesystem type: '%s'", type.c_str());
    return FileSystemPtr();
}

RA_END_NAMESPACE(fs);
