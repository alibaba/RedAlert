#ifndef RA_THREAD_LOCK_H
#define RA_THREAD_LOCK_H

#include <assert.h>
#include <pthread.h>
#include <ra/common/Common.h>
#include <ra/common/Log.h>

RA_BEGIN_NAMESPACE(util);

class Mutex {
public:
    Mutex() {
        pthread_mutex_init(&_mutex, NULL);
    }

    ~Mutex() {
        pthread_mutex_destroy(&_mutex);
    }

    int lock() {
        return pthread_mutex_lock(&_mutex);
    }

    int trylock () {
        return pthread_mutex_trylock(&_mutex);
    }

    int unlock() {
        return pthread_mutex_unlock(&_mutex);
    }

private:
    pthread_mutex_t _mutex;
};

class ScopedLock {
public:
    explicit ScopedLock(Mutex& mutex) : _mutex(mutex) {
        int ret = _mutex.lock();
        assert(ret == 0); (void) ret;
    }

    ~ScopedLock() {
        int ret = _mutex.unlock();
        assert(ret == 0); (void) ret;
    }

private:
    ScopedLock(const ScopedLock &);
    ScopedLock& operator= (const ScopedLock &);

private:
    Mutex &_mutex;
};


RA_END_NAMESPACE(util);

#endif /* RA_THREAD_LOCK_H */

