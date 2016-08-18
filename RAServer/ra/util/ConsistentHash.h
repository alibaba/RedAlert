#ifndef RA_CONSISTENTHASH_H
#define RA_CONSISTENTHASH_H

#include <ra/common/CommonDefine.h>
#include <ra/common/Log.h>

RA_BEGIN_NAMESPACE(util);

class ConsistentHash
{
public:
    ConsistentHash();
    ~ConsistentHash();
public:
    typedef std::map<uint32_t, std::string> UIntStringMap;

    void addNode(const std::string& serviceName, uint32_t virtualNodeNum);
    //return null iff _hashRing is empty
    const std::string* getService(uint32_t key) const;

    void clear();

    //return false if key already exist, true if succ
    bool addVirtualNode(const std::string& serviceName, uint32_t key);

    bool empty() const { return _hashRing.empty(); }

    uint32_t size() const { return _hashRing.size(); }
private:
    UIntStringMap _hashRing;

    uint32_t DigestToUint(const uint8_t digest[16]);
    RA_LOG_DECLARE();
};

RA_TYPEDEF_PTR(ConsistentHash);

RA_END_NAMESPACE(util);

#endif //RA_CONSISTENTHASH_H
