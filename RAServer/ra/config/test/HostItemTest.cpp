#include <ra/config/test/HostItemTest.h>
#include <ra/test/test.h>
#include <cppunit/TestAssert.h>

using namespace std;

RA_BEGIN_NAMESPACE(config);
RA_LOG_SETUP(config, HostItemTest);

CPPUNIT_TEST_SUITE_REGISTRATION(HostItemTest);

HostItemTest::HostItemTest() { 
}

HostItemTest::~HostItemTest() { 
}

void HostItemTest::setUp() { 
    RA_LOG(DEBUG, "setUp!");
}

void HostItemTest::tearDown() { 
    RA_LOG(DEBUG, "tearDown!");
}

void HostItemTest::testSimpleProcess() { 
    RA_LOG(DEBUG, "Begin Test!");
}

RA_END_NAMESPACE(config);

