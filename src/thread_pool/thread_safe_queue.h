#pragma once

#include <condition_variable>
#include <memory>
#include <mutex>


namespace thread_pool {

template<typename T>
class thread_safe_queue {
public:
    thread_safe_queue() :
            head(new node),
            tail(head.get()) {}

    thread_safe_queue(const thread_safe_queue &) = delete;

    thread_safe_queue& operator=(const thread_safe_queue&) = delete;

    bool try_pop(T& value) {
        std::lock_guard<std::mutex> head_lock(head_mutex);
        if (head.get() == get_tail()) {
            return false;
        }
        value = std::move(*head->data);
        pop_head();
        return true;
    }

    void push(T new_value) {
        std::shared_ptr<T> new_data(std::make_shared<T>(std::move(new_value)));
        std::unique_ptr<node> p(new node);
        {
            std::lock_guard<std::mutex> tail_lock(tail_mutex);
            tail->data = new_data;
            node* const new_tail = p.get();
            tail->next = std::move(p);
            tail = new_tail;
        }
    }

private:
    struct node {
        std::shared_ptr<T> data;
        std::unique_ptr<node> next;
    };

    node* get_tail() {
        std::lock_guard<std::mutex> tail_lock(tail_mutex);
        return tail;
    }

    void pop_head() {
        head = std::move(head->next);
    }

    std::mutex head_mutex;
    std::unique_ptr<node> head;

    std::mutex tail_mutex;
    node* tail;
};

} // thread_pool