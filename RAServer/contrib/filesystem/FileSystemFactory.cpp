#include <contrib/filesystem/FileSystemFactory.h>
#include <contrib/filesystem/LocalFileSystem/LocalFileSystem.h>

using namespace std;

RA_BEGIN_NAMESPACE(fs);

FileSystemPtr FileSystemFactory::create(string type) {
    if (type == "LocalFileSystem") {
        return FileSystemPtr(new LocalFileSystem());
    }
    LOG(ERROR) << "Unknown filesystem type: " << type;
    return FileSystemPtr();
}

RA_END_NAMESPACE(fs);
