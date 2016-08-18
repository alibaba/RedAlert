#ifndef RA_POLICYCHECKERTEST_H
#define RA_POLICYCHECKERTEST_H

#include <ra/common/Common.h>
#include <ra/common/Log.h>
#include <ra/process/PolicyChecker.h>
#include <ra/fetcher/MetricNode.h>
#include <ra/fetcher/MetricData.h>
#include <cppunit/extensions/HelperMacros.h>

RA_BEGIN_NAMESPACE(process);

class PolicyCheckerTest : public CppUnit::TestFixture {
    CPPUNIT_TEST_SUITE(PolicyCheckerTest);
    CPPUNIT_TEST(testSimpleProcess);
    CPPUNIT_TEST(testGenOneNormalizedData);
    CPPUNIT_TEST(testGenMultiNormalizedData);
    CPPUNIT_TEST(testCheck);
    CPPUNIT_TEST(testCheckSingleHost);
    CPPUNIT_TEST(testHostTotal);
    CPPUNIT_TEST(testMetricTotal);
    CPPUNIT_TEST(testRemoveShieldAbnormalInfo);
    CPPUNIT_TEST(testGenAbnormalStr);
    CPPUNIT_TEST(testGetAlarmMsgContent);
    CPPUNIT_TEST_SUITE_END();
public:
    PolicyCheckerTest();
    ~PolicyCheckerTest();
public:
    void setUp();
    void tearDown();
    void testSimpleProcess();
    void testGenOneNormalizedData();
    void testGenMultiNormalizedData();
    void testCheck();
    void testCheckSingleHost();
    void testHostTotal();
    void testMetricTotal();
    void testRemoveShieldAbnormalInfo();
    void testGenAbnormalStr();
    void testGetAlarmMsgContent();

private:
    static std::string makeMetricPath(const std::string& serviceName,
                                      const std::string& nodePath,
                                      const std::string& metricName);
    fetcher::MetricNodePtr generateMetricData(
        const std::string& serviceName, const std::string& nodePath,
        uint32_t host, const std::string& metric,
        const fetcher::MetricData::TimeVec& timeVec,
        const fetcher::MetricData::ValueVec& valVec) const;
    fetcher::MetricPath generateMetricPath(
        const std::string& serviceName, const std::string& nodePath,
        uint32_t host, const std::string& metric,
        const fetcher::MetricData::TimeVec& timeVec,
        const fetcher::MetricData::ValueVec& valVec) const;

private:
    RA_LOG_DECLARE();
};

RA_END_NAMESPACE(process);

#endif //RA_POLICYCHECKERTEST_H
