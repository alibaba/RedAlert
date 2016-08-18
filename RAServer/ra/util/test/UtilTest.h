#ifndef RA_UTILTEST_H
#define RA_UTILTEST_H

#include <ra/common/CommonDefine.h>
#include <ra/common/Log.h>
#include <ra/util/Util.h>
#include <cppunit/extensions/HelperMacros.h>

RA_BEGIN_NAMESPACE(util);

class UtilTest : public CppUnit::TestFixture {
    CPPUNIT_TEST_SUITE(UtilTest);
    CPPUNIT_TEST(testSimpleProcess);
    CPPUNIT_TEST(testPatternMatch);
    CPPUNIT_TEST(testIPToString);
    CPPUNIT_TEST(testStringToIP);
    CPPUNIT_TEST(testRtrim);
    CPPUNIT_TEST(testJoinString);
    CPPUNIT_TEST(testSplitString);
    CPPUNIT_TEST(testGetLocalAddress);
    CPPUNIT_TEST_SUITE_END();
public:
    UtilTest();
    ~UtilTest();
public:
    void setUp();
    void tearDown();
    void testSimpleProcess();
    void testPatternMatch();
    void testIPToString();
    void testStringToIP();
    void testRtrim();
    void testJoinString();
    void testSplitString();
    void testGetLocalAddress();
private:
    RA_LOG_DECLARE();
};

RA_END_NAMESPACE(util);

#endif //RA_UTILTEST_H
