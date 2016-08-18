#include <algorithm>
#include <ra/common/CommonDefine.h>
#include <ra/fetcher/MetricTreeUtil.h>

RA_BEGIN_NAMESPACE(fetcher);
using namespace std;

std::vector<std::string> MetricTreeUtil::pathToLabelVec(const MetricPath& path) {
    vector<string> labelVec;
    for (size_t i = 0; i < path.size(); i++) {
        labelVec.push_back(path[i]->getLabel());
    }
    return labelVec;
}

int MetricTreeUtil::comparePath(const MetricPath &path1, const MetricPath &path2) {
    size_t i = 0, j = 0;
    while (i < path1.size() && j < path2.size()) {
        const string &label1 = path1[i++]->getLabel();
        const string &label2 = path2[j++]->getLabel();
        if (label1 < label2) {
            return -1;
        }
        if (label2 < label1) {
            return 1;
        }
    }
    if (i < path1.size()) {
        return -1;
    }
    if (j < path2.size()) {
        return 1;
    }
    return 0;
}

int MetricTreeUtil::compareTree(const MetricNodePtr& root1, const MetricNodePtr& root2) {
    if (root1 == NULL && root2 == NULL) return 0;
    if (root1 == NULL || root2 == NULL) return -1;
    if (root1->getLabel() != root2->getLabel()) return -1;
    vector<MetricNodePtr> children1 = root1->getChildren();
    vector<MetricNodePtr> children2 = root2->getChildren();
    if (children1.size() != children2.size()) return -1;
    sort(children1.begin(), children1.end(), MetricNodePtrCmp());
    sort(children2.begin(), children2.end(), MetricNodePtrCmp());
    for (size_t i = 0; i < children1.size(); i++) {
        if (compareTree(children1[i], children2[i])) return -1;
    }
    return 0;
}

MetricNodePtr MetricTreeUtil::findNode(const MetricNodePtr& root, const vector<string>& labels) {
    if (labels.empty()) {
        return MetricNodePtr();
    }
    if (root == NULL) {
        return MetricNodePtr();
    }
    if (root->getLabel() != labels[0]) {
        return MetricNodePtr();
    }
    if (labels.begin() + 1 == labels.end()) {
        return root;
    }
    return findNode(root, labels.begin() + 1, labels.end());
}

MetricNodePtr MetricTreeUtil::findNode(const MetricNodePtr& root, vector<string>::const_iterator begin, vector<string>::const_iterator end) {
    if (begin == end) {
        return MetricNodePtr();
    }
    const string& head = (*begin);
    const vector<MetricNodePtr>& children = root->getChildren();
    vector<MetricNodePtr>::const_iterator iter = find_if(children.begin(), children.end(), MetricNodePtrEq(head));
    if (iter == children.end()) {
        return MetricNodePtr();
    }
    MetricNodePtr child = (*iter);
    if (begin + 1 == end) {
        return child;
    }
    return findNode(child, begin + 1, end);
}

bool MetricTreeUtil::insertPath(MetricNodePtr& root, const MetricPath& path) {
    if (path.empty()) {
        return true;
    }
    if (root == NULL) {
        root = path[0]->cloneNode();
    }
    if (root->getLabel() != path[0]->getLabel()) {
        return false;
    }
    return insertPath(root, path.begin() + 1, path.end());
}

bool MetricTreeUtil::insertPath(MetricNodePtr& root, MetricPath::const_iterator begin, MetricPath::const_iterator end) {
    if (begin == end) {
        return true;
    }
    MetricNodePtr head = (*begin);
    const vector<MetricNodePtr>& children = root->getChildren();
    vector<MetricNodePtr>::const_iterator iter = find_if(children.begin(), children.end(), MetricNodePtrEq(head));
    MetricNodePtr child;
    if (iter == children.end()) {
        child = head->cloneNode();
        if (child == NULL) {
            return false;
        }
        root->addChild(child);
    } else {
        child = (*iter);
    }
    return insertPath(child, begin + 1, end);
}

MetricNodePtr MetricTreeUtil::mergeTree(const MetricNodePtr& root1, const MetricNodePtr& root2) {
    if (root1 == NULL) return root2;
    if (root2 == NULL) return root1;
    if (root1->getLabel() != root2->getLabel()) {
        return MetricNodePtr();
    }
    const vector<MetricNodePtr>& children1 = root1->getChildren();
    const vector<MetricNodePtr>& children2 = root2->getChildren();
    vector<MetricNodePtr> setUnion;
    vector<MetricNodePtr> setIntersection;
    setUnion.insert(setUnion.end(), children1.begin(), children1.end());
    setUnion.insert(setUnion.end(), children2.begin(), children2.end());
    for (size_t i = 0; i < setUnion.size(); i++) {
        const MetricNodePtr& child = setUnion[i];
        vector<MetricNodePtr>::iterator iter = find_if(setIntersection.begin(), setIntersection.end(), MetricNodePtrEq(child));
        if (iter == setIntersection.end()) {
            setIntersection.push_back(child);
            continue;
        }
        MetricNodePtr mergedTree = mergeTree((*iter), child);
        if (mergedTree) {
            (*iter) = mergedTree;
        }
    }
    MetricNodePtr root = root1->cloneNode();
    if (root == NULL) return root;
    for (size_t i = 0; i < setIntersection.size(); i++) {
        root->addChild(setIntersection[i]);
    }
    return root;
}

bool MetricTreeUtil::traverseTree(const MetricNodePtr& root, const MetricTreeTraverseFunc& func) {
    if (root == NULL) return true;
    MetricPath path;
    return traverseTree(root, func, path);
}

bool MetricTreeUtil::traverseTree(const MetricNodePtr& root, const MetricTreeTraverseFunc& func, MetricPath& path) {
    path.push_back(root);
    const vector<MetricNodePtr> &children = root->getChildren();
    if (children.empty()) {
        if (!func(path)) return false;
        path.pop_back();
        return true;
    }
    for (size_t i = 0; i < children.size(); i++) {
        if (!traverseTree(children[i], func, path)) return false;
    }
    path.pop_back();
    return true;
}

RA_END_NAMESPACE(fetcher);

