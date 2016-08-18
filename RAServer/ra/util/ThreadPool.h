#ifndef RA_THREAD_POOL_H
#define RA_THREAD_POOL_H

#include <pthread.h>
#include <queue>
#include <ra/common/Common.h>
#include <ra/common/Log.h>

RA_BEGIN_NAMESPACE(util);

class WorkItem
{
public:
    virtual ~WorkItem() { }
    virtual void process() = 0;
};

class ThreadPool {
public:
    ThreadPool(const size_t threadCount = 1U,
               const size_t queueCapacity = 32U);
    virtual ~ThreadPool();

    bool push(WorkItem *item, bool block = true);
    bool start();
    bool stop();

    size_t getQueueSize() {
        pthread_mutex_lock(&_mutex);
        size_t size = _queue.size();
        pthread_mutex_unlock(&_mutex);
        return size;
    }

private:
    ThreadPool(const ThreadPool&);
    ThreadPool& operator=(const ThreadPool&);

    static void *threadEntry(void *arg);

private:
    size_t _threadCount;
    size_t _queueCapacity;
    bool _running;
    std::queue<WorkItem *> _queue;
    std::vector<pthread_t> _threads;
    pthread_mutex_t _mutex;
    pthread_cond_t _empty;
    pthread_cond_t _full;
};

RA_END_NAMESPACE(util);

#endif /* RA_THREAD_POOL_H */

