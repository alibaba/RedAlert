#include <ra/config/HostItem.h>
using namespace std;

RA_BEGIN_NAMESPACE(config);

HostItem::HostItem()
    : _hostID(0),
      _weight(0)
{ 
}

HostItem::HostItem(const string& address, const string& serviceName, uint32_t id)
    : _address(address), _serviceName(serviceName), _hostID(id), _weight(0)
{
}
HostItem::HostItem(const string& address, const string& serviceName, 
                   uint32_t id, uint32_t weight)
    : _address(address), _serviceName(serviceName), 
      _hostID(id), _weight(weight)
{
}

HostItem::~HostItem() { 
}


RA_END_NAMESPACE(config);


