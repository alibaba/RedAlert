#include <sqlite3.h>
#include <cppunit/TestAssert.h>
#include <ra/test/test.h>
#include <ra/common/HttpServer.h>
#include <ra/util/test/UtilTest.h>

using namespace std;

RA_BEGIN_NAMESPACE(util);

CPPUNIT_TEST_SUITE_REGISTRATION(UtilTest);

UtilTest::UtilTest() { 
}

UtilTest::~UtilTest() { 
}

void UtilTest::setUp() { 
}

void UtilTest::tearDown() { 
}

void UtilTest::testSimpleProcess() { 
}

void UtilTest::testPatternMatch()
{
    // simple
    CPPUNIT_ASSERT(Util::isPatternMatch("galaxy?index[0-65535]*", "galaxy-index[0-65535]a"));
    CPPUNIT_ASSERT(!Util::isPatternMatch("galaxy?index[0-65535]*", "galaxy-bigindex[0-65535]a"));

    // greedy
    CPPUNIT_ASSERT(Util::isPatternMatch("ab*ba", "abbbb*bbba"));
    CPPUNIT_ASSERT(!Util::isPatternMatch("ab*sa", "abbbbsabba"));

    // multi *
    CPPUNIT_ASSERT(Util::isPatternMatch("****System.10.10.10.10*", "System.10.10.10.10"));

    // ?
    CPPUNIT_ASSERT(Util::isPatternMatch("galaxy?index", "galaxy-index"));
    CPPUNIT_ASSERT(Util::isPatternMatch("galaxy?index", "galaxy_index"));
    CPPUNIT_ASSERT(!Util::isPatternMatch("galaxy?index", "galaxyindex"));

    // escape
    CPPUNIT_ASSERT(Util::isPatternMatch("dd\\dd-*", "dddd-1"));
    //dd\dd
    CPPUNIT_ASSERT(!Util::isPatternMatch("dd\\dd-*", "dd\\dd-1"));
    //dd\\dd
    CPPUNIT_ASSERT(Util::isPatternMatch("dd\\\\dd-*", "dd\\dd-1"));

    // escape *?
    CPPUNIT_ASSERT(Util::isPatternMatch("\\*\\?", "*?")); // \*\?
    CPPUNIT_ASSERT(Util::isPatternMatch("\\*\\??", "*?a"));
    CPPUNIT_ASSERT(Util::isPatternMatch("*\\*\\??", "aadef?*?!"));
    CPPUNIT_ASSERT(Util::isPatternMatch("\\*a*b\\?", "*aaassss.dddb?"));
}

typedef map<string, string> StringMap;

string pairToString(const StringMap::value_type& v)
{
    return v.first + v.second;
}

struct TestFilterCallback
{
    vector<string> strList;
    void operator() (const std::string& s, const StringMap::value_type& v)
    {
        strList.push_back(s);
    }
};

void UtilTest::testIPToString()
{
    string ip;
    CPPUNIT_ASSERT(Util::IPToString(16885952, ip));
    CPPUNIT_ASSERT_EQUAL(string("192.168.1.1"), ip);
}

void UtilTest::testStringToIP()
{
    uint32_t ip;
    CPPUNIT_ASSERT(Util::StringToIP("192.168.1.155", ip));
    CPPUNIT_ASSERT_EQUAL(2600577216U, ip);

    CPPUNIT_ASSERT(!Util::StringToIP("192.168.1.256", ip));
}

void UtilTest::testRtrim() {
    string fileName = "xxxxx/";
    Util::rtrim(fileName, '/');
    CPPUNIT_ASSERT_EQUAL(string("xxxxx"), fileName);

    fileName = "xxxxx///";
    Util::rtrim(fileName,'/');
    CPPUNIT_ASSERT_EQUAL(string("xxxxx"), fileName);

    fileName = "xxxxx";
    Util::rtrim(fileName, '/');
    CPPUNIT_ASSERT_EQUAL(string("xxxxx"), fileName);

    fileName = "/xxxxx";
    Util::rtrim(fileName, '/');
    CPPUNIT_ASSERT_EQUAL(string("/xxxxx"), fileName);
}

void UtilTest::testJoinString() {
    string sep, joined;
    vector<string> tokens;
    tokens.push_back("a");
    tokens.push_back("b");
    tokens.push_back("c");
    sep = ".";
    joined = Util::joinString(tokens, sep);
    CPPUNIT_ASSERT_EQUAL(string("a.b.c"), joined);
}

void UtilTest::testSplitString() {
    {
        string sep, joined;
        vector<string> tokens;
        sep = ".";
        tokens = Util::splitString("a.b.c", sep);
        joined = Util::joinString(tokens, sep);
        CPPUNIT_ASSERT_EQUAL(string("a.b.c"), joined);
        sep = "..";
        tokens = Util::splitString("..a..b..c....", sep);
        joined = Util::joinString(tokens, sep);
        CPPUNIT_ASSERT_EQUAL(string("a..b..c"), joined);
        sep = "/";
        tokens = Util::splitString("///////", sep);
        joined = Util::joinString(tokens, sep);
        CPPUNIT_ASSERT_EQUAL(string(), joined);
    }
    {
        string path = "/a/b/c/d";
        vector<string> elems = Util::splitString(path, "/");
        CPPUNIT_ASSERT_EQUAL((size_t)4, elems.size());
        CPPUNIT_ASSERT_EQUAL(string("a"), elems[0]);
        CPPUNIT_ASSERT_EQUAL(string("b"), elems[1]);
        CPPUNIT_ASSERT_EQUAL(string("c"), elems[2]);
        CPPUNIT_ASSERT_EQUAL(string("d"), elems[3]);

        path = "/a/b/c/d/";
        elems.clear();
        elems = Util::splitString(path, "/");
        CPPUNIT_ASSERT_EQUAL((size_t)4, elems.size());
        CPPUNIT_ASSERT_EQUAL(string("a"), elems[0]);
        CPPUNIT_ASSERT_EQUAL(string("b"), elems[1]);
        CPPUNIT_ASSERT_EQUAL(string("c"), elems[2]);
        CPPUNIT_ASSERT_EQUAL(string("d"), elems[3]);

        path = "a/b/c/d/";
        elems.clear();
        elems = Util::splitString(path, "/");
        CPPUNIT_ASSERT_EQUAL((size_t)4, elems.size());
        CPPUNIT_ASSERT_EQUAL(string("a"), elems[0]);
        CPPUNIT_ASSERT_EQUAL(string("b"), elems[1]);
        CPPUNIT_ASSERT_EQUAL(string("c"), elems[2]);
        CPPUNIT_ASSERT_EQUAL(string("d"), elems[3]);

        path = "a/b/c/d";
        elems.clear();
        elems = Util::splitString(path, "/");
        CPPUNIT_ASSERT_EQUAL((size_t)4, elems.size());
        CPPUNIT_ASSERT_EQUAL(string("a"), elems[0]);
        CPPUNIT_ASSERT_EQUAL(string("b"), elems[1]);
        CPPUNIT_ASSERT_EQUAL(string("c"), elems[2]);
        CPPUNIT_ASSERT_EQUAL(string("d"), elems[3]);

        path = "a//b/c/d/";
        elems.clear();
        elems = Util::splitString(path, "/");
        CPPUNIT_ASSERT_EQUAL((size_t)4, elems.size());
        CPPUNIT_ASSERT_EQUAL(string("a"), elems[0]);
        CPPUNIT_ASSERT_EQUAL(string("b"), elems[1]);
        CPPUNIT_ASSERT_EQUAL(string("c"), elems[2]);
        CPPUNIT_ASSERT_EQUAL(string("d"), elems[3]);

        path = "///a/b/c/d/";
        elems.clear();
        elems = Util::splitString(path, "/");
        CPPUNIT_ASSERT_EQUAL((size_t)4, elems.size());
        CPPUNIT_ASSERT_EQUAL(string("a"), elems[0]);
        CPPUNIT_ASSERT_EQUAL(string("b"), elems[1]);
        CPPUNIT_ASSERT_EQUAL(string("c"), elems[2]);
        CPPUNIT_ASSERT_EQUAL(string("d"), elems[3]);
    }
}

void UtilTest::testGetLocalAddress() {
    common::HttpServer server;
    string remoteHost("127.0.0.1");
    uint16_t remotePort = 18081;
    CPPUNIT_ASSERT(server.init(remoteHost, remotePort));
    string localAddr;

    CPPUNIT_ASSERT_FAIL(Util::getLocalAddress(remoteHost, 0, localAddr));
    CPPUNIT_ASSERT_FAIL(Util::getLocalAddress("127.0.0.2", remotePort, localAddr));
    CPPUNIT_ASSERT_FAIL(Util::getLocalAddress("m.sm.cn", remotePort, localAddr));
    CPPUNIT_ASSERT(Util::getLocalAddress(remoteHost, remotePort, localAddr));
    CPPUNIT_ASSERT(Util::getLocalAddress(remoteHost, remotePort, localAddr));
    CPPUNIT_ASSERT(Util::getLocalAddress(remoteHost, remotePort, localAddr));
    LOG(INFO) << "Local address: " << localAddr;
    CPPUNIT_ASSERT_EQUAL(remoteHost, localAddr);
}

RA_END_NAMESPACE(util);

