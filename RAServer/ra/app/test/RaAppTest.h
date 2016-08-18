#ifndef RA_RAAPPTEST_H
#define RA_RAAPPTEST_H

#include <ra/common/CommonDefine.h>
#include <ra/common/Log.h>
#include <ra/app/RaApp.h>
#include <cppunit/extensions/HelperMacros.h>

RA_BEGIN_NAMESPACE(app);

class RaAppTest : public CppUnit::TestFixture {
    CPPUNIT_TEST_SUITE(RaAppTest);
    CPPUNIT_TEST(testSimpleProcess);
    CPPUNIT_TEST(testInitSuccess);
    CPPUNIT_TEST(testInitLackOfPort);
    CPPUNIT_TEST(testLoadBootstrapConf);
    CPPUNIT_TEST_SUITE_END();
public:
    RaAppTest();
    ~RaAppTest();
public:
    void setUp();
    void tearDown();
    void testSimpleProcess();
    void testInitSuccess();
    void testInitLackOfPort();
    void testLoadBootstrapConf();
private:
    uint32_t selectPort();
    std::string _bootConfFile;
    std::string _logConfFile;
    uint32_t _port;
    char _portStr[10];
private:
    RA_LOG_DECLARE();
};

RA_END_NAMESPACE(app);

#endif //RA_RAAPPTEST_H
