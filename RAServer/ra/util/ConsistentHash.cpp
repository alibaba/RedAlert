#include <ra/util/MD5.h>
#include <ra/util/ConsistentHash.h>

RA_BEGIN_NAMESPACE(util);

ConsistentHash::ConsistentHash() { 
}

ConsistentHash::~ConsistentHash() { 
}

void ConsistentHash::addNode(const std::string& serviceName,
                             uint32_t virtualNodeNum)
{
    char virtualNodeName[128];
    int nameLen;
    uint8_t digest[16];
    uint32_t key;
    for (uint32_t i = 0; i < virtualNodeNum; ++i) {
        nameLen = snprintf(virtualNodeName, sizeof(virtualNodeName), "%s-%u", serviceName.c_str(), i);
        assert(nameLen > 0);
        DoMd5((uint8_t*)virtualNodeName, nameLen, digest);

        key = DigestToUint(digest);
        if (_hashRing.find(key) != _hashRing.end()) {
            // just skip this virtualnode, simple solution for now
            continue;
        }

        _hashRing[key] = serviceName;
    }
}

const std::string* ConsistentHash::getService(uint32_t key) const
{
    if (_hashRing.empty()) {
        return NULL;
    }

    UIntStringMap::const_iterator it = _hashRing.upper_bound(key);
    if (_hashRing.end() == it) {
        it = _hashRing.upper_bound(0);
    }

    assert(_hashRing.end() != it);
    if (_hashRing.end() == it) {
        return NULL;
    }

    return &it->second;
}

void ConsistentHash::clear()
{
    _hashRing.clear();
}

uint32_t ConsistentHash::DigestToUint(const uint8_t digest[16])
{
    return digest[0]
            | (digest[4] << 8)
            | (digest[8] << 16)
            | (digest[12] << 24);
}

bool ConsistentHash::addVirtualNode(const std::string& serviceName, uint32_t key)
{
    if (_hashRing.end() != _hashRing.find(key)) {
        return false;
    }

    _hashRing[key] = serviceName;
    return true;
}

RA_END_NAMESPACE(util);

