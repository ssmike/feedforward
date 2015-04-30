#ifndef GUARDS_H
#define GUARDS_H

#include <mutex>
#include <thread>
#include <iostream>


class Thread_guard {
public:
    Thread_guard(std::thread t): in_t(std::move(t)) {
    }
    ~Thread_guard(){
        if (in_t.joinable())
            in_t.join();
    }
    Thread_guard(Thread_guard&& other) {
        in_t = std::move(other.in_t);
    }
private:
    std::thread in_t;
};

class Mutex_lock_guard {
public:
     Mutex_lock_guard(std::mutex& m): in_m(m) {
        in_m.lock();
    }
    ~ Mutex_lock_guard() {
        in_m.unlock();
    }

private:
    std::mutex& in_m;
};

#endif // GUARDS_H
