#ifndef RA_STRING_HASH_H
#define RA_STRING_HASH_H

#include <ra/common/Common.h>
#include <ra/common/Log.h>

RA_BEGIN_NAMESPACE(util);

class StringHash
{
public:
    StringHash();
    ~StringHash();
public:
    static uint32_t hashString(const char *str, uint32_t dwHashType);
    static uint32_t hashString(const char *str, size_t size, uint32_t dwHashType);
    static uint64_t hashString64(const char *str);
    static uint64_t hashString64(const char *str, size_t size);
};

RA_END_NAMESPACE(util);

#endif /* RA_STRING_HASH_H */

