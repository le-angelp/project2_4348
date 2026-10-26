CXX = g++
CXXFLAGS = -std=c++17 -pthread -Wall -Wextra

all: my-dining

my-dining: main.cpp petersonLock.hpp tournament.hpp
	$(CXX) $(CXXFLAGS) main.cpp -o my-dining

clean:
	rm -f my-dining

.PHONY: all clean
