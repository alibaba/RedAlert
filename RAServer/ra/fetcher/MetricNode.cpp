#include <sstream>
#include <ra/common/Common.h>
#include <ra/common/Log.h>
#include <ra/fetcher/MetricNode.h>

RA_BEGIN_NAMESPACE(fetcher);
using namespace std;

int MetricNodePtrCmp::operator()(const MetricNodePtr& node1, const MetricNodePtr& node2) {
    if (node1 == NULL && node2 == NULL) return false;
    if (node1 == NULL) return true;
    if (node2 == NULL) return false;
    return node1->getLabel().compare(node2->getLabel()) < 0;
}

MetricNodePtrEq::MetricNodePtrEq(const std::string& label): _label(label) {
}

MetricNodePtrEq::MetricNodePtrEq(const MetricNodePtr& node): _node(node) {
}

bool MetricNodePtrEq::operator ()(const MetricNodePtr& other) {
    if (_node) return _node->getLabel() == other->getLabel();
    if (!_label.empty()) return _label == other->getLabel();
    return false;
}

MetricNodePtr MetricNode::cloneNode() const {
    return MetricNodePtr(new MetricNode(_label));
}

MetricNodePtr MetricNode::cloneTree() const {
    MetricNodePtr root(new MetricNode(_label));
    for (size_t i = 0; i < _children.size(); i++) {
        MetricNodePtr child = _children[i]->cloneTree();
        root->addChild(child);
    }
    return root;
}

string MetricNode::toDebugString(uint32_t indent) const {
    std::stringstream ss;
    std::string indentStr(indent, ' ');
    ss << indentStr << _label << ": [";
    if (!_children.empty()) ss << "\n";
    vector<MetricNodePtr> sortedChildren(_children);
    sort(sortedChildren.begin(), sortedChildren.end(), MetricNodePtrCmp());
    for (size_t i = 0; i < sortedChildren.size(); i++) {
        if (i > 0) ss << ",\n";
        ss << sortedChildren[i]->toDebugString(indent + 5);
    }
    if (_children.empty()) {
        ss << " ]";
    } else {
        ss << "\n" << indentStr << "   ]";
    }
    return ss.str();
}

RA_END_NAMESPACE(fetcher);
