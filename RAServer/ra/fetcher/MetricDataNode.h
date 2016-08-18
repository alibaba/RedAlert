#ifndef RA_DATANODE_H
#define RA_DATANODE_H

#include <sstream>
#include <ra/common/Common.h>
#include <ra/common/CommonDefine.h>
#include <ra/common/Log.h>
#include <ra/fetcher/MetricNode.h>
#include <ra/fetcher/MetricData.h>

RA_BEGIN_NAMESPACE(fetcher);

class MetricDataNode: public MetricNode
{
public:
    MetricDataNode(MetricDataPtr data)
        : MetricNode(METRIC_DATA_NODE_LABEL + "@" + data->hostname), _data(data) {
    }

    virtual ~MetricDataNode() {
    }

    bool isDataNode() const {
        return true;
    }

    MetricDataPtr getData() const {
        return _data;
    }

    virtual MetricNodePtr cloneNode() const {
        return MetricNodePtr(new MetricDataNode(_data));
    }

    virtual MetricNodePtr cloneTree() const {
        return cloneNode();
    }

    virtual std::string toDebugString(uint32_t indent) const {
        if (_data == NULL) {
            return "EmptyDataNode";
        }
        std::stringstream ss;
        ss << std::string(indent, ' ') << getLabel();
        ss << "(" << _data->startTime() / MICROSECONDS << "," << _data->endTime() / MICROSECONDS << ")";
        ss << "{ avg: " << _data->avgValue() << ", min: " << _data->minValue() << ", max: " << _data->maxValue() << " }";
        return ss.str();
    }

private:
    MetricDataNode(const MetricDataNode&);
    MetricDataNode& operator=(const MetricDataNode&);

private:
    MetricDataPtr _data;
};

RA_TYPEDEF_PTR(MetricDataNode);
RA_END_NAMESPACE(fetcher);

#endif //RA_DATANODE_H
