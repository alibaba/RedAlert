#include <jansson.h>
#include <ra/common/Json.h>

RA_BEGIN_NAMESPACE(common);
using namespace std;

RA_LOG_SETUP(common, Json);

static JsonPtr toJson(json_t *raw) {
    if (json_is_null(raw)) {
        return JsonPtr(new JsonNull());
    }
    if (json_is_boolean(raw)) {
        return JsonPtr(new JsonBoolean(json_boolean_value(raw)));
    }
    if (json_is_number(raw)) {
        return JsonPtr(new JsonNumber(json_number_value(raw)));
    }
    if (json_is_string(raw)) {
        string value(json_string_value(raw));
        return JsonPtr(new JsonString(value));
    }
    if (json_is_array(raw)) {
        JsonArray *array = new JsonArray();
        for (size_t i = 0; i < json_array_size(raw); i++) {
            json_t *elem = json_array_get(raw, i);
            array->append(toJson(elem));
        }
        return JsonPtr(array);
    }
    if (json_is_object(raw)) {
        JsonObject *object = new JsonObject();
        const char *key;
        json_t *value;
        json_object_foreach(raw, key, value) {
            object->insert(string(key),  toJson(value));
        }
        return JsonPtr(object);
    }
    return JsonPtr();
}

JsonPtr Json::load(const string& input) {
    json_error_t error;
    json_t *root = json_loads(input.c_str(), 0, &error);
    if (root == NULL) {
        LOG(ERROR) << "Json load error: on line " << error.line << ": " << error.text;
        return JsonPtr();
    }
    JsonPtr json = toJson(root);
    if (json == NULL) {
        LOG(ERROR) << "Json load error: result is NULL";
    }
    json_decref(root);
    return json;
}

string JsonArray::dump() const {
    stringstream out;
    out << "[";
    for (const_iterator iter = begin(); iter != end(); iter++) {
        if (iter != begin()) out << ",";
        if (*iter) out << (*iter)->dump();
        else out << "null";
    }
    out << "]";
    return out.str();
}

string JsonObject::dump() const {
    stringstream out;
    out << "{";
    for (const_iterator iter = begin(); iter != end(); iter++) {
        if (iter != begin()) out << ",";
        out << "\"" << iter->first << "\":";
        if (iter->second) out << iter->second->dump();
        else out << "null";
    }
    out << "}";
    return out.str();
}

RA_END_NAMESPACE(common);

