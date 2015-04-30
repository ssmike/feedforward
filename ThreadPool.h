#ifndef THREADPOOL_H
#define THREADPOOL_H

#include <thread>
#include <future>
#include <cassert>
#include <functional>

#include "TSQueue.h"

template<typename R>
class Thread_pool {
private:
    using task_function = std::function<R()>;
    using task_promise = std::promise<R>;
    std::vector<std::thread> workers;
    Thread_safe_queue<std::pair<task_function, task_promise > > ts_queue;

public:
    Thread_pool(size_t count_of_workers): ts_queue(count_of_workers) {
        for (size_t worker = 0; worker < count_of_workers; ++worker) {
            workers.emplace_back(std::bind(&Thread_pool::worker_fn, this));
        }
    }

    Thread_pool() {
        unsigned int count_of_workers = std::thread::hardware_concurrency();
        if(count_of_workers == 0) {
            count_of_workers = 4;
        }
        ts_queue(count_of_workers);
        for (int worker = 0; worker < count_of_workers; ++worker) {
            workers.emplace_back(std::thread(std::bind(&Thread_pool::worker_fn, this)));
        }
    }

    void worker_fn() {
        std::pair<task_function, task_promise > task;
        while (ts_queue.pop(task)) {
            try {
                R result = task.first();
                task.second.set_value(result);
            } catch(std::exception&) {
                task.second.set_exception(std::current_exception());
            }

        }
    }

    std::future<R> try_submit(std::function<R()> new_task) {
        std::promise<R> promise_for_task;
        std::future<R> future_for_task = promise_for_task.get_future();
        if (ts_queue.try_push(make_pair(new_task, std::move(promise_for_task)))) {
            return  future_for_task;
        }
        return std::future<R>();
    }

    std::future<R> submit(std::function<R()> new_task) {
        std::promise<R> promise_for_task;
        std::future<R> future_for_task = promise_for_task.get_future();
        ts_queue.push(std::make_pair(new_task, std::move(promise_for_task)));
        return future_for_task;
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
