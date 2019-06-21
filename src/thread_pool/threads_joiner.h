#pragma once


#include <thread>
#include <vector>


namespace thread_pool {

class threads_joiner {
public:
    explicit threads_joiner(std::vector<std::thread>& threads_) :
            threads(threads_) {}

    ~threads_joiner() {
        for (auto& thread: threads) {
            if (thread.joinable()) {
                thread.join();
            }
        }
    }

private:
    std::vector<std::thread>& threads;
};

} // thread_pool
