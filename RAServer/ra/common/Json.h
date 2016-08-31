#ifndef RA_JSON_H
#define RA_JSON_H

#include <sstream>
#include <tr1/memory>
#include <ra/common/Common.h>
#include <ra/common/Log.h>

RA_BEGIN_NAMESPACE(common);

class Json;
class JsonNull;
class JsonBoolean;
class JsonNumber;
class JsonString;
class JsonArray;
class JsonObject;

RA_TYPEDEF_PTR(Json);
RA_TYPEDEF_PTR(JsonNull);
RA_TYPEDEF_PTR(JsonBoolean);
RA_TYPEDEF_PTR(JsonNumber);
RA_TYPEDEF_PTR(JsonString);
RA_TYPEDEF_PTR(JsonArray);
RA_TYPEDEF_PTR(JsonObject);

/// Josn objects

class Json {
public:
    Json() { }
    virtual ~Json() { }

    static JsonPtr load(const std::string &input);
    virtual std::string dump() const { return std::string(); }

    virtual bool isNull() const { return false; }
    virtual bool isBoolean() const { return false; }
    virtual bool isNumber() const { return false; }
    virtual bool isString() const { return false; }
    virtual bool isArray() const { return false; }
    virtual bool isObject() const { return false; }

protected:
    RA_LOG_DECLARE();
};

class JsonNull: public Json {
public:
    JsonNull() { }
    ~JsonNull() { }
    virtual bool isNull() const { return true; }
    virtual std::string dump() const { return "null"; }
};

class JsonBoolean: public Json {
public:
    JsonBoolean(bool value): _value(value) { }
    ~JsonBoolean() { }
    virtual bool isBoolean() const { return true; }
    virtual std::string dump() const { return _value ? "true" : "false"; }
    operator bool() const { return _value; }
private:
    bool _value;
};

class JsonNumber: public Json {
public:
    JsonNumber(double value): _value(value) { }
    ~JsonNumber() { }
    virtual bool isNumber() const { return true; }
    virtual std::string dump() const { std::stringstream out; out << _value; return out.str(); }
    operator double() const { return _value; }
private:
    double _value;
};

class JsonString: public Json {
public:
    JsonString(const std::string &value): _value(value) { }
    ~JsonString() { }
    virtual bool isString() const { return true; }
    virtual std::string dump() const { return '"' + _value + '"'; }
    operator const std::string&() const { return _value; }
private:
    std::string _value;
};

class JsonArray: public Json {
public:
    typedef std::vector<JsonPtr>::iterator iterator;
    typedef std::vector<JsonPtr>::const_iterator const_iterator;
public:
    JsonArray() { }
    ~JsonArray() { }
    virtual bool isArray() const { return true; }
    virtual std::string dump() const;
    void append(const JsonPtr &json) { _array.push_back(json); }
    void erase(iterator iter) { _array.erase(iter); }
    JsonPtr& operator[](size_t i) { return _array[i]; }
    const_iterator begin() const { return _array.begin(); }
    iterator begin() { return _array.begin(); }
    const_iterator end() const { return _array.end(); }
    iterator end() { return _array.end(); }
    size_t size() const { return _array.size(); }
private:
    std::vector<JsonPtr> _array;
};

class JsonObject: public Json {
public:
    typedef std::map<std::string, JsonPtr>::iterator iterator;
    typedef std::map<std::string, JsonPtr>::const_iterator const_iterator;
public:
    JsonObject() { }
    ~JsonObject() { }
    virtual bool isObject() const { return true; }
    virtual std::string dump() const;
    const_iterator find(const std::string &key) const { return _object.find(key); }
    iterator find(const std::string &key) { return _object.find(key); }
    template <typename T>
    void get(const std::string &key, T &value, const T &defaultValue) const;
    template <typename T>
    bool get(const std::string &key, T &value) const;
    std::pair<iterator, bool> insert(const std::string &key, const JsonPtr &value) {
        return _object.insert(std::pair<std::string, JsonPtr>(key, value));
    }
    void erase(const std::string &key) { _object.erase(key); }
    void erase(iterator iter) { _object.erase(iter); }
    JsonPtr& operator[](const std::string& key) { return _object[key]; }
    const_iterator begin() const { return _object.begin(); }
    iterator begin() { return _object.begin(); }
    const_iterator end() const { return _object.end(); }
    iterator end() { return _object.end(); }
    size_t size() const { return _object.size(); }
private:
    std::map<std::string, JsonPtr> _object;
};

template <typename T>
void JsonObject::get(const std::string& key, T& value, const T& defaultValue) const {
    const_iterator iter = find(key);
    if (iter != end() && fromJson(iter->second, value)) {
        return;
    }
    LOG(INFO) << "Return default value for key " << key;
    value = defaultValue;
}

template <typename T>
bool JsonObject::get(const std::string& key, T& value) const {
    const_iterator iter = find(key);
    if (iter != end() && fromJson(iter->second, value)) {
        return true;
    }
    return false;
}

/// Json utilities

inline bool fromJson(const JsonPtr &json, bool &value) {
    if (json == NULL) return false;
    if (!json->isBoolean()) return false;
    JsonBooleanPtr boolean = std::tr1::dynamic_pointer_cast<JsonBoolean>(json);
    if (boolean == NULL) return false;
    value = *boolean;
    return true;
}

inline bool fromJson(const JsonPtr &json, double &value) {
    if (json == NULL) return false;
    if (!json->isNumber()) return false;
    JsonNumberPtr number = std::tr1::dynamic_pointer_cast<JsonNumber>(json);
    if (number == NULL) return false;
    value = *number;
    return true;
}

inline bool fromJson(const JsonPtr &json, uint32_t &value) {
    double doubleVal = 0.0;
    if (!fromJson(json, doubleVal)) {
        return false;
    }
    value = (uint32_t)doubleVal;
    return true;
}

inline bool fromJson(const JsonPtr &json, int32_t &value) {
    double doubleVal = 0.0;
    if (!fromJson(json, doubleVal)) {
        return false;
    }
    value = (int32_t)doubleVal;
    return true;
}

inline bool fromJson(const JsonPtr &json, uint64_t &value) {
    double doubleVal = 0.0;
    if (!fromJson(json, doubleVal)) {
        return false;
    }
    value = (uint64_t)doubleVal;
    return true;
}

inline bool fromJson(const JsonPtr &json, int64_t &value) {
    double doubleVal = 0.0;
    if (!fromJson(json, doubleVal)) {
        return false;
    }
    value = (int64_t)doubleVal;
    return true;
}

inline bool fromJson(const JsonPtr &json, std::string &value) {
    if (json == NULL) return false;
    if (!json->isString()) return false;
    JsonStringPtr str = std::tr1::dynamic_pointer_cast<JsonString>(json);
    if (str == NULL) return false;
    value = *str;
    return true;
}

template <typename T>
bool fromJson(const JsonPtr &json, std::vector<T> &value) {
    if (json == NULL) return false;
    if (!json->isArray()) return false;
    JsonArrayPtr array = std::tr1::dynamic_pointer_cast<JsonArray>(json);
    if (array == NULL) return false;
    std::vector<T> tmp(array->size());
    for (size_t i = 0; i < array->size(); i++) {
        if (!fromJson((*array)[i], tmp[i])) return false;
    }
    value.swap(tmp);
    return true;
}

template <typename T>
bool fromJson(const JsonPtr &json, std::map<std::string, T> &value) {
    if (json == NULL) return false;
    if (!json->isObject()) return false;
    JsonObjectPtr object = std::tr1::dynamic_pointer_cast<JsonObject>(json);
    if (object == NULL) return false;
    std::map<std::string, T> tmp;
    JsonObject::const_iterator iter;
    for (iter = object->begin(); iter != object->end(); iter++) {
        const std::string& key = iter->first;
        if (!fromJson(iter->second, tmp[key])) return false;
    }
    value.swap(tmp);
    return true;
}

template <typename T>
bool fromJson(const JsonPtr &json, T& value) {
    if (json == NULL) return false;
    if (!json->isObject()) return false;
    JsonObjectPtr object = std::tr1::dynamic_pointer_cast<JsonObject>(json);
    if (object == NULL) return false;
    return value.fromJson(object);
}

inline JsonPtr toJson(bool value) {
    return JsonPtr(new JsonBoolean(value));
}

inline JsonPtr toJson(double value) {
    return JsonPtr(new JsonNumber(value));
}

inline JsonPtr toJson(const std::string &value) {
    return JsonPtr(new JsonString(value));
}

template <typename T>
JsonPtr toJson(const std::vector<T> &value) {
    JsonArrayPtr array(new JsonArray());
    typename std::vector<T>::const_iterator iter;
    for (iter = value.begin(); iter != value.end(); iter++) {
        JsonPtr elem = toJson(*iter);
        if (elem == NULL) return JsonPtr();
        array->append(elem);
    }
    return array;
}

template <typename T>
JsonPtr toJson(const std::map<std::string, T> &value) {
    JsonObjectPtr object(new JsonObject());
    typename std::map<std::string, T>::const_iterator iter;
    for (iter = value.begin(); iter != value.end(); iter++) {
        JsonPtr elem = toJson(iter->second);
        if (elem == NULL) return JsonPtr();
        object->insert(iter->first, elem);
    }
    return object;
}

RA_END_NAMESPACE(common);

#endif /* RA_JSON_H */

