#include "ThreadPool.h"


ThreadPool::ThreadPool(const size_t thread_count) : _thread_count(thread_count), _stop(false)
{
    for (size_t i = 0; i< thread_count; i++)
    {
        _threads.emplace_back([this]{
            while(true)
        {
            std::unique_lock<std::mutex> lock(_mutex);
                _cv.wait(lock,[this] { return !_tasks.empty() || _stop; });
                if (_stop && _tasks.empty()) break;
                auto task = _tasks.front();
                _tasks.pop();
                lock.unlock();
                task();
        }});
    }
}

void ThreadPool::submit(std::function<void()> lambda)
{
    std::unique_lock<std::mutex> lock(_mutex);
    _tasks.push(lambda);
    _cv.notify_one();
}

ThreadPool::~ThreadPool()
{
    _stop = true;
    _cv.notify_all();
    for (auto& t : _threads) t.join();
}
