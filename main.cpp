#include <atomic>
#include <charconv>
#include <chrono>
#include <cstdio>
#include <cstdlib>
#include <exception>
#include <memory>
#include <new>
#include <random>
#include <system_error>
#include <thread>
#include <utility>
#include <vector>

#include "petersonLock.hpp"
#include "tournament.hpp"

enum class Solution { Coarse, Fine };

const char *solutionName(Solution solution) {
    return solution == Solution::Coarse ? "COARSE" : "FINE";
}

void printEvent(Solution solution, std::size_t philosopher,
                int round, const char *event) {
    char line[128];
    std::snprintf(line, sizeof(line), "%s %zu %d %s",
                  solutionName(solution), philosopher, round, event);
    std::printf("%s\n", line);
}

void pauseRandomly(std::mt19937 &random) {
    std::uniform_int_distribution<int> delay(1, 500);
    std::this_thread::sleep_for(std::chrono::milliseconds(delay(random)));
}

std::mt19937 makeRandom(std::size_t id) {
    auto now = static_cast<unsigned int>(
        std::chrono::steady_clock::now().time_since_epoch().count());
    return std::mt19937(now ^ (static_cast<unsigned int>(id) * 2654435761U));
}

void runCoarse(std::size_t n) {
    TournamentTreeLock lock(n);
    std::vector<std::thread> workers;
    workers.reserve(n);

    try {
        for (std::size_t id = 0; id < n; ++id) {
            workers.emplace_back([id, &lock]() {
                auto random = makeRandom(id);
                for (int round = 0; round < 5; ++round) {
                    printEvent(Solution::Coarse, id, round, "THINK_START");
                    pauseRandomly(random);
                    printEvent(Solution::Coarse, id, round, "THINK_END");

                    lock.acquire(id);
                    printEvent(Solution::Coarse, id, round, "EAT_START");
                    pauseRandomly(random);
                    printEvent(Solution::Coarse, id, round, "EAT_END");
                    lock.release(id);
                }
            });
        }
    } catch (...) {
        for (auto &worker : workers) worker.join();
        throw;
    }

    for (auto &worker : workers) worker.join();
}

int philosopherSide(std::size_t stick, std::size_t philosopher) {
    return philosopher == stick ? 0 : 1;
}

void runFine(std::size_t n) {
    auto sticks = std::make_unique<PetersonLock[]>(n);
    std::vector<std::thread> workers;
    workers.reserve(n);

    try {
        for (std::size_t id = 0; id < n; ++id) {
            workers.emplace_back([id, n, &sticks]() {
                auto random = makeRandom(id + n);
                std::size_t left = id;
                std::size_t right = (id + n - 1) % n;

                for (int round = 0; round < 5; ++round) {
                    printEvent(Solution::Fine, id, round, "THINK_START");
                    pauseRandomly(random);
                    printEvent(Solution::Fine, id, round, "THINK_END");

                    // Always take the lower-numbered stick first.
                    std::size_t first = left < right ? left : right;
                    std::size_t second = left < right ? right : left;
                    sticks[first].acquire(philosopherSide(first, id));
                    sticks[second].acquire(philosopherSide(second, id));

                    printEvent(Solution::Fine, id, round, "EAT_START");
                    pauseRandomly(random);
                    printEvent(Solution::Fine, id, round, "EAT_END");

                    sticks[second].release(philosopherSide(second, id));
                    sticks[first].release(philosopherSide(first, id));
                }
            });
        }
    } catch (...) {
        for (auto &worker : workers) worker.join();
        throw;
    }

    for (auto &worker : workers) worker.join();
}

bool parseCount(const char *text, std::size_t &value) {
    if (text == nullptr || *text == '\0') return false;
    std::size_t parsed = 0;
    const char *end = text;
    while (*end != '\0') ++end;
    auto result = std::from_chars(text, end, parsed);
    if (result.ec != std::errc{} || result.ptr != end || parsed < 2) return false;
    value = parsed;
    return true;
}

int main(int argc, char *argv[]) {
    if (argc != 2) {
        std::fprintf(stderr, "Usage: ./my-dining n (n must be at least 2)\n");
        return 1;
    }

    std::size_t n = 0;
    if (!parseCount(argv[1], n)) {
        std::fprintf(stderr, "Error: n must be an integer of at least 2.\n");
        return 1;
    }

    try {
        std::printf("BEGIN COARSE\n");
        runCoarse(n);
        std::printf("END COARSE\n");

        std::printf("BEGIN FINE\n");
        runFine(n);
        std::printf("END FINE\n");
    } catch (const std::bad_alloc &) {
        std::fprintf(stderr, "Error: not enough memory for this value of n.\n");
        return 1;
    } catch (const std::system_error &error) {
        std::fprintf(stderr, "Error: could not create a thread: %s\n", error.what());
        return 1;
    } catch (const std::exception &error) {
        std::fprintf(stderr, "Error: %s\n", error.what());
        return 1;
    }

    return 0;
}
