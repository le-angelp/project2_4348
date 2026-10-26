#ifndef PETERSON_LOCK_HPP
#define PETERSON_LOCK_HPP

#include <atomic>
#include <thread>

// Peterson's mutual exclusion lock for two threads.
// The lock uses two atomic flags (wants_) and an atomic variable (turn_) to indicate 
// if they want to enter and whose turn it is to enter the critical section. 
// Each thread has a unique ID (0 or 1) and sets its wants_ flag to enter the critical section.

class PetersonLock {
public:
    // Acquire lock by setting the wants_ flag for the current thread to true and
    // setting the turn_ variable to the other thread's ID.
    void acquire(int me) {
        int other = 1 - me;
        wants_[me].store(true);
        turn_.store(other);
        while (wants_[other].load() && turn_.load() == other) {
            std::this_thread::yield();
        }
    }

    //Release lock by setting the wants_ flag for the current thread to false
    void release(int me) {
        wants_[me].store(false);
    }

private:
    std::atomic<bool> wants_[2] = {false, false};
    std::atomic<int> turn_{0};
};

#endif