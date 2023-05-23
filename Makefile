CC = /usr/bin/gcc
CXX = /usr/bin/g++

CFLAGS = -D_GNU_SOURCE -D_XOPEN_SOURCE=700 -D_REENTRANT
CFLAGS += -O3 -std=c99 -mtune=core-avx2 -pthread
CFLAGS += -ftree-vectorize -ftree-slp-vectorize
CFLAGS += -finline-functions -funroll-loops
CFLAGS += -Wall -Wextra -Wpedantic -flto=32

CXXFLAGS = -D_GNU_SOURCE -D_XOPEN_SOURCE=700 -D_REENTRANT
CXXFLAGS += -O3 -std=c++17 -mtune=core-avx2 -pthread
CXXFLAGS += -ftree-vectorize -ftree-slp-vectorize
CXXFLAGS += -finline-functions -funroll-loops
CXXFLAGS += -Wall -Wextra -Wpedantic -flto=32

LDFLAGS = -lm
GNUMP = -lgmp
OPENMP = -fopenmp

PROGRAMS = isprime isprimemp popcnt clz ctz geomean findprimes goldbach
PROGRAMS += primefactors primefactorsmp

all: $(PROGRAMS)

isprime: isprime.o
	$(CC) $(CFLAGS) $(LDFLAGS) $< -o $@

isprimemp: isprimemp.o
	$(CXX) $(CXXFLAGS) $(LDFLAGS) $(GNUMP) $< -o $@

popcnt: popcnt.o
	$(CC) $(CFLAGS) $(LDFLAGS) $< -o $@

findprimes: findprimes.o
	$(CXX) $(CXXFLAGS) $(LDFLAGS) $< -o $@

primefactors: primefactors.o
	$(CXX) $(CXXFLAGS) $(LDFLAGS) $< -o $@

primefactorsmp: primefactorsmp.o
	$(CXX) $(CXXFLAGS) $(LDFLAGS) $(GNUMP) $< -o $@

goldbach: goldbach.o
	$(CXX) $(CXXFLAGS) $(OPENMP) $(LDFLAGS) $< -o $@

geomean: geomean.o
	$(CC) $(CFLAGS) $(LDFLAGS) $< -o $@

clz: clz.o
	$(CC) $(CFLAGS) $(LDFLAGS) $< -o $@

ctz: ctz.o
	$(CC) $(CFLAGS) $(LDFLAGS) $< -o $@

goldbach.o: goldbach.cpp
	$(CXX) $(CXXFLAGS) $(OPENMP) -c $< -o $@

.c.o:
	$(CC) $(CFLAGS) -c $< -o $@

.cpp.o:
	$(CXX) $(CXXFLAGS) -c $< -o $@

clean:
	rm -f *.o $(PROGRAMS)

