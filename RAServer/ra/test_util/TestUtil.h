#ifndef RA_TESTUTIL_H
#define RA_TESTUTIL_H

#include <ra/common/Common.h>
#include <ra/common/Log.h>
#include <ra/config/PolicyConfigItemBase.h>

RA_BEGIN_NAMESPACE(test_util);

class TestUtil
{
public:
    TestUtil();
    ~TestUtil();
private:
    TestUtil(const TestUtil &);
    TestUtil& operator=(const TestUtil &);

public:
    static config::PolicyConfigItemBasePtr generatePolicyItem(const std::string& metric, 
            int32_t fetchInterval, int32_t timeRangeCount, int32_t id = 0, config::PolicyTrigger pt = config::PT_THRESHOLD);
};

RA_TYPEDEF_PTR(TestUtil);

RA_END_NAMESPACE(test_util);

#endif //RA_TESTUTIL_H
