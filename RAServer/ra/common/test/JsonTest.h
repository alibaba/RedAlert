#ifndef RA_JSONTEST_H
#define RA_JSONTEST_H

#include <cppunit/extensions/HelperMacros.h>
#include <ra/common/CommonDefine.h>
#include <ra/common/Log.h>

RA_BEGIN_NAMESPACE(common);

class JsonTest: public CppUnit::TestFixture {
    CPPUNIT_TEST_SUITE(JsonTest);
    CPPUNIT_TEST(testLoad);
    CPPUNIT_TEST(testDump);
    CPPUNIT_TEST(testArray);
    CPPUNIT_TEST(testObject);
    CPPUNIT_TEST(testFromJson);
    CPPUNIT_TEST(testToJson);
    CPPUNIT_TEST_SUITE_END();
public:
    JsonTest();
    virtual ~JsonTest();

    void setUp();
    void tearDown();

    void testLoad();
    void testDump();
    void testArray();
    void testObject();
    void testFromJson();
    void testToJson();
};

RA_END_NAMESPACE(common);

#endif /* RA_JSONTEST_H */

