#ifndef RA_METRICTREEUTIL_H
#define RA_METRICTREEUTIL_H

#include <vector>
#include <string>
#include <tr1/functional>
#include <ra/common/Common.h>
#include <ra/common/Log.h>
#include <ra/fetcher/MetricNode.h>

RA_BEGIN_NAMESPACE(fetcher);

typedef std::tr1::function<bool (const MetricPath&)> MetricTreeTraverseFunc;

class MetricTreeUtil {
public:
    static std::vector<std::string> pathToLabelVec(const MetricPath &path);
    static int comparePath(const MetricPath &path1, const MetricPath &path2);
    static int compareTree(const MetricNodePtr &root1, const MetricNodePtr &root2);
    static MetricNodePtr findNode(const MetricNodePtr &root, const std::vector<std::string> &labels);
    static bool insertPath(MetricNodePtr &root, const MetricPath &path);
    static MetricNodePtr mergeTree(const MetricNodePtr &root1, const MetricNodePtr &root2);
    static bool traverseTree(const MetricNodePtr &root, const MetricTreeTraverseFunc &func);

private:
    static MetricNodePtr findNode(const MetricNodePtr &root,
                                  std::vector<std::string>::const_iterator begin,
                                  std::vector<std::string>::const_iterator end);
    static bool insertPath(MetricNodePtr &root, MetricPath::const_iterator begin, MetricPath::const_iterator end);
    static bool traverseTree(const MetricNodePtr &root, const MetricTreeTraverseFunc &func, MetricPath &path);

};

RA_END_NAMESPACE(fetcher);

#endif /* RA_METRICTREEUTIL_H */

