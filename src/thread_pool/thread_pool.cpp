#include <chrono>
#include "thread_pool.h"


namespace thread_pool {

thread_pool::thread_pool(size_t num_threads) :
        done(false),
        joiner(threads) {
    try {
        for (size_t i = 0; i < num_threads; ++i) {
            threads.emplace_back(&thread_pool::worker_thread, this);
        }
    } catch (...) {
        done = true;
        throw;
    }
}

thread_pool::~thread_pool() {
    done = true;
}

void thread_pool::worker_thread() {
    while (!done) {
        std::function<void()> task;
        if (work_queue.try_pop(task)) {
            task();
        } else {
            //using namespace std::chrono_literals;
            //std::this_thread::sleep_for(50ms);

            //std::this_thread::yield();

            std::unique_lock<std::mutex> locker(treads_locker);
            threads_cond.wait(locker);
       }
    }
}

} // thread_pool