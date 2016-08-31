#ifndef RA_HOSTITEM_H
#define RA_HOSTITEM_H

#include <ra/common/CommonDefine.h>
#include <ra/common/Log.h>

RA_BEGIN_NAMESPACE(config);

class HostItem
{
public:
    HostItem();
    HostItem(const std::string& address, const std::string& serviceName, uint32_t id);
    HostItem(const std::string& address, const std::string& serviceName, 
             uint32_t id, uint32_t weight);
    ~HostItem();
public:
    uint32_t getHostID() const {
        return _hostID;
    }

    const std::string& getServiceName() const {
        return _serviceName;
    }

    uint32_t getWeight() const {
        return _weight;
    }

    const std::string& getIpAddress() const
    {
        return _address;
    }
private:
    std::string _address;
    std::string _serviceName;
    uint32_t _hostID;
    uint32_t _weight;
};

RA_TYPEDEF_PTR(HostItem);

RA_END_NAMESPACE(config);

#endif //RA_HOSTITEM_H
