#ifndef RA_JSONMEMORYMANAGER_H
#define RA_JSONMEMORYMANAGER_H

#include <ra/common/Common.h>
#include <ra/common/Log.h>
#include <jansson.h>
RA_BEGIN_NAMESPACE(fetcher);

#define _wrap_json_memeory(value) JsonMemoryWrapper __##value##_jsonMemoryWrapper(value);
class JsonMemoryWrapper
{
public:
    JsonMemoryWrapper(json_t *value) {
        _value = value;
    }
    ~JsonMemoryWrapper() {
        if (_value != NULL) {
            json_decref(_value);
        }
    }

private:
    JsonMemoryWrapper(const JsonMemoryWrapper &);
    JsonMemoryWrapper& operator=(const JsonMemoryWrapper &);

private:
    json_t *_value;
private:
    RA_LOG_DECLARE();
};

RA_TYPEDEF_PTR(JsonMemoryWrapper);

RA_END_NAMESPACE(fetcher);

#endif //RA_JSONMEMORYMANAGER_H
