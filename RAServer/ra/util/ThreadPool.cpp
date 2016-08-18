#include <algorithm>
#include <ra/util/ThreadPool.h>

RA_BEGIN_NAMESPACE(util);

class DummyWorkItem: public WorkItem {
public:
    DummyWorkItem() { }
    ~DummyWorkItem() { }
    void process() { }
};

ThreadPool::ThreadPool(const size_t threadCount, const size_t queueCapacity)
    : _threadCount(threadCount), _queueCapacity(queueCapacity), _running(false) {
    _queueCapacity = std::max(_queueCapacity, _threadCount);
    pthread_mutex_init(&_mutex, NULL);
    pthread_cond_init(&_empty, NULL);
    pthread_cond_init(&_full, NULL);
}

ThreadPool::~ThreadPool() {
    pthread_mutex_lock(&_mutex);
    while (!_queue.empty()) {
        WorkItem *item = _queue.front();
        _queue.pop();
        delete item;
    }
    pthread_mutex_unlock(&_mutex);

    pthread_mutex_destroy(&_mutex);
    pthread_cond_destroy(&_empty);
    pthread_cond_destroy(&_full);
}

bool ThreadPool::push(WorkItem* item, bool block) {
    if (item == NULL) return false;
    pthread_mutex_lock(&_mutex);
    if (!block && _queue.size() >= _queueCapacity) {
        pthread_mutex_unlock(&_mutex);
        return false;
    }
    while (_queue.size() >= _queueCapacity) {
        pthread_cond_wait(&_full, &_mutex);
    }
    _queue.push(item);
    pthread_cond_broadcast(&_empty);
    pthread_mutex_unlock(&_mutex);
    return true;
}

void* ThreadPool::threadEntry(void* arg) {
    ThreadPool *pool = (ThreadPool *)arg;
    while (pool->_running) {
        pthread_mutex_lock(&pool->_mutex);
        while (pool->_queue.empty()) {
            pthread_cond_wait(&pool->_empty, &pool->_mutex);
        }
        WorkItem *item = pool->_queue.front();
        pool->_queue.pop();
        pthread_cond_broadcast(&pool->_full);
        pthread_mutex_unlock(&pool->_mutex);
        item->process();
        delete item;
    }
    return NULL;
}

bool ThreadPool::start() {
    _threads.resize(_threadCount);
    for (size_t i = 0; i < _threadCount; i++) {
        int err = pthread_create(&_threads[i], NULL, &ThreadPool::threadEntry, this);
        if (err != 0) return false;
    }
    _running = true;
    return true;
}

bool ThreadPool::stop() {
    if (!_running) {
        return true;
    }
    _running = false;
    pthread_mutex_lock(&_mutex);
    for (size_t i = 0; i < _threadCount; i++) {
        _queue.push(new DummyWorkItem());
    }
    pthread_cond_broadcast(&_empty);
    pthread_mutex_unlock(&_mutex);
    for (size_t i = 0; i < _threadCount; i++) {
        pthread_join(_threads[i], NULL);
    }
    _threads.clear();
    return true;
}

RA_END_NAMESPACE(util);
