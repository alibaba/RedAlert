#ifndef RA_METRICNODE_H
#define RA_METRICNODE_H

#include <vector>
#include <string>
#include <ra/common/Common.h>
#include <ra/common/CommonDefine.h>
#include <ra/common/Log.h>
#include <ra/fetcher/MetricData.h>

RA_BEGIN_NAMESPACE(fetcher);

class MetricNode;
RA_TYPEDEF_PTR(MetricNode);
typedef std::vector<MetricNodePtr> MetricPath;
typedef std::map<std::string, fetcher::MetricNodePtr> MetricRootMap;

class MetricNodePtrCmp
{
public:
    int operator()(const MetricNodePtr& node1, const MetricNodePtr& node2);
};

class MetricNodePtrEq
{
public:
    MetricNodePtrEq(const std::string& label);
    MetricNodePtrEq(const MetricNodePtr& node);
    bool operator()(const MetricNodePtr& other);
private:
    std::string _label;
    MetricNodePtr _node;
};

class MetricNode
{
public:
    MetricNode(const std::string &label)
        : _label(label) {
    }

    virtual ~MetricNode() {
    }

    const std::string& getLabel() const {
        return _label;
    }

    void setLabel(const std::string &label) {
        _label = label;
    }

    virtual bool isDataNode() const {
        return false;
    }

    virtual MetricDataPtr getData() const {
        return MetricDataPtr();
    }

    virtual bool isLeafNode() const {
        return _children.empty();
    }

    virtual const std::vector<MetricNodePtr>& getChildren() const {
        return _children;
    }

    virtual void addChild(const MetricNodePtr &node) {
        _children.push_back(node);
    }

    virtual MetricNodePtr cloneNode() const;

    virtual MetricNodePtr cloneTree() const;

    virtual std::string toDebugString(uint32_t indent = 0) const;

private:
    MetricNode(const MetricNode&);
    MetricNode& operator=(const MetricNode&);

private:
    std::string _label;
    std::vector<MetricNodePtr> _children;
};

RA_END_NAMESPACE(fetcher);

#endif //RA_METRICNODE_H
