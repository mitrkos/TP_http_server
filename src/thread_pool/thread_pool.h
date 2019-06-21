#pragma once

#include <atomic>
#include <condition_variable>
#include <functional>
#include <thread>
#include <vector>

#include "threads_joiner.h"
#include "thread_safe_queue.h"


namespace thread_pool {

class thread_pool {
public:
    explicit thread_pool(size_t num_threads);

    ~thread_pool();

    template<typename FunctionType>
    void submit(FunctionType f) {
        work_queue.push(std::function<void()>(f));

        std::unique_lock<std::mutex> locker(treads_locker);
        threads_cond.notify_one();
    }


private:
    void worker_thread();

    std::mutex treads_locker;
    std::condition_variable threads_cond;
    std::atomic_bool done;
    thread_safe_queue<std::function<void()>> work_queue;
    std::vector<std::thread> threads;
    threads_joiner joiner;
};

} // thread_pool
