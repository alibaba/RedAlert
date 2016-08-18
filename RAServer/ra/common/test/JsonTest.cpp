#include <sstream>
#include <tr1/memory>
#include <ra/test/test.h>
#include <ra/common/Json.h>
#include <ra/common/test/JsonTest.h>

RA_BEGIN_NAMESPACE(common);
using namespace std;
using namespace std::tr1;

RA_LOG_SETUP(common, JsonTest);
CPPUNIT_TEST_SUITE_REGISTRATION(JsonTest);

JsonTest::JsonTest() {
}

JsonTest::~JsonTest() {
}

void JsonTest::setUp() {
    RA_LOG(INFO, "setUp!");
}

void JsonTest::tearDown() {
    RA_LOG(INFO, "tearDown!");
}

void JsonTest::testLoad() {
    JsonPtr json;
    json = Json::load("1");
    CPPUNIT_ASSERT_FAIL(json);
    json = Json::load("\"str\"");
    CPPUNIT_ASSERT_FAIL(json);
    json = Json::load("null");
    CPPUNIT_ASSERT_FAIL(json);
    json = Json::load("true");
    CPPUNIT_ASSERT_FAIL(json);

    json = Json::load("[1,2,3]");
    CPPUNIT_ASSERT(json);
    CPPUNIT_ASSERT(json->isArray());
    RA_LOG(INFO, "Array: %s", json->dump().c_str());
    json = Json::load("{\"key\":\"value\"}");
    CPPUNIT_ASSERT(json);
    CPPUNIT_ASSERT(json->isObject());
    RA_LOG(INFO, "Object: %s", json->dump().c_str());
}

JsonPtr genNestedJson(size_t depth, size_t &count) {
    if (depth == 0) {
        stringstream id;
        switch (++count % 4) {
            case 0:
                return JsonPtr(new JsonNull());
            case 1:
                return JsonPtr(new JsonBoolean(count % 2 == 1));
            case 2:
                return JsonPtr(new JsonNumber(count));
            case 3:
                id << count;
                return JsonPtr(new JsonString("val" + id.str()));
        }
    }
    if (depth % 2) {
        JsonArray *array = new JsonArray();
        for (size_t i = 0; i < depth; i++) {
            array->append(genNestedJson(depth - 1, count));
        }
        return JsonPtr(array);
    }
    JsonObject *object = new JsonObject();
    for (size_t i = 0; i < depth; i++) {
        stringstream id;
        id << ++count;
        object->insert("key" + id.str(), genNestedJson(depth - 1, count));
    }
    return JsonPtr(object);
}

void JsonTest::testDump() {
    for (size_t depth = 1; depth < 6; depth++) {
        size_t count = 0;
        JsonPtr root = genNestedJson(depth, count);
        string serialized = root->dump();
        RA_LOG(INFO, "Json (depth %zu): %s", depth, serialized.c_str());
        CPPUNIT_ASSERT(Json::load(serialized));
    }
}

void JsonTest::testArray() {
    JsonPtr json = Json::load("[0,1,2]");
    CPPUNIT_ASSERT(json);
    CPPUNIT_ASSERT(json->isArray());
    JsonArrayPtr array = dynamic_pointer_cast<JsonArray>(json);
    CPPUNIT_ASSERT(array);
    JsonNumberPtr number;
    for (JsonArray::iterator iter = array->begin(); iter != array->end(); iter++) {
        CPPUNIT_ASSERT((*iter)->isNumber());
        number = dynamic_pointer_cast<JsonNumber>(*iter);
        double value = *number;
        double count = iter - array->begin();
        CPPUNIT_ASSERT_EQUAL(count, value);
    }
}

void JsonTest::testObject() {
    JsonPtr json = Json::load("{\"key0\":0,\"key1\":1,\"key2\":2}");
    CPPUNIT_ASSERT(json);
    CPPUNIT_ASSERT(json->isObject());
    JsonObjectPtr object = dynamic_pointer_cast<JsonObject>(json);
    CPPUNIT_ASSERT(object);
    JsonNumberPtr number;
    for (JsonObject::iterator iter = object->begin(); iter != object->end(); iter++) {
        CPPUNIT_ASSERT(iter->second->isNumber());
        number = dynamic_pointer_cast<JsonNumber>(iter->second);
        double value = *number;
        CPPUNIT_ASSERT(value < 3);
    }
    for (size_t i = 0; i < 3; i++) {
        stringstream ss;
        ss << i;
        string key = "key" + ss.str();
        number = dynamic_pointer_cast<JsonNumber>((*object)[key]);
        double value = *number;
        CPPUNIT_ASSERT_EQUAL((double)i, value);
    }
}

void JsonTest::testFromJson() {
    JsonPtr json = Json::load("[\"a\",\"b\",\"c\"]");
    CPPUNIT_ASSERT(json);
    vector<string> strings;
    CPPUNIT_ASSERT(fromJson(json, strings));
    CPPUNIT_ASSERT_EQUAL((size_t)3U, strings.size());

    json = Json::load("{\"key0\":0,\"key1\":1,\"key2\":2}");
    CPPUNIT_ASSERT(json);
    map<string, double> numbers;
    CPPUNIT_ASSERT(fromJson(json, numbers));
    CPPUNIT_ASSERT_EQUAL((size_t)3U, numbers.size());

    json = Json::load("{\"key0\":[0],\"key1\":[1],\"key2\":[2]}");
    CPPUNIT_ASSERT(json);
    map<string, vector<double> > arrays;
    CPPUNIT_ASSERT(fromJson(json, arrays));
    CPPUNIT_ASSERT_EQUAL((size_t)3U, arrays.size());
}

void JsonTest::testToJson() {
    vector<string> strings;
    strings.push_back("a");
    strings.push_back("b");
    strings.push_back("c");
    JsonPtr json = toJson(strings);
    CPPUNIT_ASSERT(json);

    map<string, double> numbers;
    numbers["key0"] = 0;
    numbers["key1"] = 1;
    numbers["key2"] = 2;
    json = toJson(numbers);
    CPPUNIT_ASSERT(json);

    map<string, vector<double> > arrays;
    arrays["key0"] = vector<double>(1, 0);
    arrays["key1"] = vector<double>(1, 1);
    arrays["key2"] = vector<double>(1, 2);
    json = toJson(arrays);
    CPPUNIT_ASSERT(json);
}

RA_END_NAMESPACE(common);
