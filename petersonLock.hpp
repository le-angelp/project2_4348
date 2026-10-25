#ifndef PETERSON_LOCK_HPP
#define PETERSON_LOCK_HPP

#include <atomic>
#include <thread>

class PetersonLock {
public:
    void acquire(int me) {
        int other = 1 - me;
        wants_[me].store(true);
        turn_.store(other);
        while (wants_[other].load() && turn_.load() == other) {
            std::this_thread::yield();
        }
    }

    void release(int me) {
        wants_[me].store(false);
    }

private:
    std::atomic<bool> wants_[2] = {false, false};
    std::atomic<int> turn_{0};
};

#endif