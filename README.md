# Dining Philosophers Project

Team number: 25
Members: Angel Le, Jiajie Lu


## Compile and run

Run these commands on Linux:

```text
make
./my-dining 5
make clean
```

The number must be 2 or larger. The program runs the coarse solution first and the fine solution second.

## Lock design

The Peterson lock is for two threads. Each thread has a different number, 0 or 1. It sets its `wants` value to true and gives the other thread the turn. The atomic values make the shared lock data safe to read and write.

The Tournament-Tree lock rounds the number of leaves up to a power of two. Philosopher `i` uses leaf `i`. Extra leaves are empty and are never used. At each tree level, the philosopher takes the Peterson lock for that node. It releases the locks from the root back down to its leaf.

## Dining solutions

In the coarse solution, the Tournament-Tree lock covers the full eating time. This lets only one philosopher eat at a time.

In the fine solution, each chopstick has its own Peterson lock. The two philosophers beside that chopstick use participant numbers 0 and 1. Each philosopher always picks up the lower-numbered chopstick first. This order prevents a circular wait, so philosophers cannot deadlock. Two neighboring philosophers cannot eat at the same time because they share a chopstick.

Each philosopher thinks and eats five times. Every think and eat delay is a random number of milliseconds from 1 to 500. Each thread has its own random-number generator.

## Assumptions and limitations

The program uses POSIX threads through `std::thread` and is intended for the course Linux environment with `g++`.
