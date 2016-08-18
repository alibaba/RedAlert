#ifndef RA_TIMERANGETEST_H
#define RA_TIMERANGETEST_H

#include <ra/common/CommonDefine.h>
#include <ra/common/Log.h>
#include <ra/config/TimeRange.h>
#include <cppunit/extensions/HelperMacros.h>

RA_BEGIN_NAMESPACE(config);

class TimeRangeTest : public CppUnit::TestFixture {
    CPPUNIT_TEST_SUITE(TimeRangeTest);
    CPPUNIT_TEST(testSimpleProcess);
    CPPUNIT_TEST(testFormatTime);
    CPPUNIT_TEST(testIsInRangeWithFullDay);
    CPPUNIT_TEST(testIsInRangeWithOneMin);
    CPPUNIT_TEST_SUITE_END();
public:
    TimeRangeTest();
    ~TimeRangeTest();
public:
    void setUp();
    void tearDown();
    void testSimpleProcess();
    void testFormatTime();
    void testIsInRangeWithFullDay();
    void testIsInRangeWithOneMin();
private:
    RA_LOG_DECLARE();
};

RA_END_NAMESPACE(config);

#endif //RA_TIMERANGETEST_H
