#ifndef THREADPOOL_H
#define THREADPOOL_H

#include <thread>
#include <future>
#include <cassert>
#include <functional>
#include <algorithm>

#include "TSQueue.h"


class Thread_pool {
private:
    using task_function = std::function<void()>;
    std::vector<std::thread> workers;
    Thread_safe_queue<task_function> ts_queue;

public:
    Thread_pool(size_t count_of_workers): ts_queue(count_of_workers) {
        for (size_t worker = 0; worker < count_of_workers; ++worker) {
            workers.emplace_back(std::bind(&Thread_pool::worker_fn, this));
        }
    }

    Thread_pool(): ts_queue(std::max((int)std::thread::hardware_concurrency(), 4)){
        int count_of_workers = std::max((int)std::thread::hardware_concurrency(), 4);
        for (int worker = 0; worker < count_of_workers; ++worker) {
            workers.emplace_back(std::thread(std::bind(&Thread_pool::worker_fn, this)));
        }
    }

    void worker_fn() {
        task_function task;
        while (ts_queue.pop(task)) {
            try {
                task();
            } catch(std::exception& e) {
                std::cerr << "multiprocessing error" << std::endl;
            }

        }
    }

    bool try_submit(std::function<void()> new_task) {
        return ts_queue.try_push(new_task);
    }

    void submit(std::function<void()> new_task) {
        ts_queue.push(new_task);
    }

    ~Thread_pool() {
        ts_queue.shutdown();
        for(auto & i : workers) {
            assert(i.joinable());
            i.join();
        }
    }
};

#endif // THREADPOOL_H
