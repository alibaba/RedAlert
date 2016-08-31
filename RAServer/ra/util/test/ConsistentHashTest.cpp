#include <ra/util/test/ConsistentHashTest.h>
#include <ra/test/test.h>
#include <cppunit/TestAssert.h>

using namespace std;

RA_BEGIN_NAMESPACE(util);

CPPUNIT_TEST_SUITE_REGISTRATION(ConsistentHashTest);

ConsistentHashTest::ConsistentHashTest() { 
}

ConsistentHashTest::~ConsistentHashTest() { 
}

void ConsistentHashTest::setUp() { 
}

void ConsistentHashTest::tearDown() { 
}

void ConsistentHashTest::testSimpleProcess() { 
    ConsistentHash hash;

    const string* name = hash.getService(1);
    CPPUNIT_ASSERT(NULL == name);
    
    hash.addNode("dev1", 10);
    hash.addNode("dev2", 10);
    CPPUNIT_ASSERT_EQUAL((uint32_t)20, hash.size());

    name = hash.getService(1);
    CPPUNIT_ASSERT(NULL != name);
}

void ConsistentHashTest::testGetService()
{
    ConsistentHash hash;
    hash.addVirtualNode("s1", 1000);
    hash.addVirtualNode("s2", 5000);
    hash.addVirtualNode("s3", 6000);

    const string* name;
    name = hash.getService(0);
    CPPUNIT_ASSERT_EQUAL(*name, string("s1"));
    name = hash.getService(999);
    CPPUNIT_ASSERT_EQUAL(*name, string("s1"));
    name = hash.getService(1000);
    CPPUNIT_ASSERT_EQUAL(*name, string("s2"));
    name = hash.getService(5500);
    CPPUNIT_ASSERT_EQUAL(*name, string("s3"));
    name = hash.getService(9000);
    CPPUNIT_ASSERT_EQUAL(*name, string("s1"));
}

RA_END_NAMESPACE(util);

