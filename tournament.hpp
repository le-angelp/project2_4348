#ifndef TOURNAMENT_TREE_LOCK_HPP
#define TOURNAMENT_TREE_LOCK_HPP

#include "petersonLock.hpp"
#include <cstddef>
#include <memory>
#include <new>
#include <utility>
#include <vector>

class TournamentTreeLock {
public:
    explicit TournamentTreeLock(std::size_t threads)
        : threads_(threads), path_(threads) {
        leaves_ = 1;
        while (leaves_ < threads_) {
            if (leaves_ > static_cast<std::size_t>(-1) / 2) {
                throw std::bad_alloc();
            }
            leaves_ *= 2;
        }
        locks_ = std::make_unique<PetersonLock[]>(leaves_ - 1);
        std::size_t height = 0;
        for (std::size_t count = leaves_; count > 1; count /= 2) ++height;
        for (auto &path : path_) path.reserve(height);
    }

    void acquire(std::size_t id) {
        std::size_t node = leaves_ - 1 + id;
        while (node > 0) {
            std::size_t parent = (node - 1) / 2;
            int side = (node == parent * 2 + 1) ? 0 : 1;
            locks_[parent].acquire(side);
            path_[id].push_back({parent, side});
            node = parent;
        }
    }

    void release(std::size_t id) {
        auto &path = path_[id];
        for (auto it = path.rbegin(); it != path.rend(); ++it) {
            locks_[it->first].release(it->second);
        }
        path.clear();
    }

private:
    std::size_t threads_;
    std::size_t leaves_;
    std::unique_ptr<PetersonLock[]> locks_;
    // Each philosopher owns its own path, so no lock is needed for this list.
    std::vector<std::vector<std::pair<std::size_t, int>>> path_;
};

#endif