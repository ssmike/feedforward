#ifndef TSQUEUE_H
#define TSQUEUE_H

#include "Guards.h"

#include <queue>
#include <mutex>
#include <thread>
#include <iostream>
#include <algorithm>
#include <condition_variable>


template<typename T>
class Thread_safe_queue
{
private:
    std::mutex mut;
    size_t capacity;
    std::queue<T> data_queue;
    std::condition_variable not_empty_cond;
    std::condition_variable not_full_cond;
    bool shutdown_called = false;

    int size() {
        std::unique_lock<std::mutex> lock(mut);
        return data_queue.size();
    }

public:
    Thread_safe_queue(int need_size_of_queue):capacity(need_size_of_queue) {}

    void push(T new_value) {
        std::unique_lock<std::mutex> lock(mut);
        if(shutdown_called) {
            return;
        }
        not_full_cond.wait(lock, [&]{return capacity > data_queue.size();});
        data_queue.push(std::move(new_value));
        not_empty_cond.notify_one();
    }

    bool try_push(T new_value) {
        std::lock_guard<std::mutex> lock(mut);
        if(capacity > data_queue.size()) {
            data_queue.push(std::move(new_value));
            not_empty_cond.notify_one();
            return true;
        }
        return false;
    }

    bool pop(T& new_val) {
        std::unique_lock<std::mutex> lock(mut);
        not_empty_cond.wait(lock, [&]{return shutdown_called || !data_queue.empty();});
        if (data_queue.empty()) return false;
        new_val = std::move(data_queue.front());
        not_full_cond.notify_one();
        data_queue.pop();
        return true;
    }

    void shutdown() {
        std::lock_guard<std::mutex> lock(mut);
        shutdown_called = true;
        not_empty_cond.notify_all();
    }
};

#endif // TSQUEUE_H
