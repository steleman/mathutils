CC = /usr/bin/gcc
CXX = /usr/bin/g++

CFLAGS = -D_GNU_SOURCE -D_XOPEN_SOURCE=700
CFLAGS += -O2 -std=c99 -mtune=corei7-avx
CFLAGS += -ftree-vectorize -ftree-slp-vectorize
CFLAGS += -Wall -Wextra -flto=32

CXXFLAGS = -D_GNU_SOURCE -D_XOPEN_SOURCE=700
CXXFLAGS += -O2 -std=c++11 -mtune=core-avx2
CXXFLAGS += -ftree-vectorize -ftree-slp-vectorize
CXXFLAGS += -Wall -Wextra -flto=32

LDFLAGS = -lm
OPENMP = -fopenmp

PROGRAMS = isprime popcnt clz ctz geomean findprimes goldbach

all: $(PROGRAMS)

isprime: isprime.o
	$(CC) $(CFLAGS) $(LDFLAGS) $< -o $@

popcnt: popcnt.o
	$(CC) $(CFLAGS) $(LDFLAGS) $< -o $@

findprimes: findprimes.o
	$(CC) $(CFLAGS) $(OPENMP) $(LDFLAGS) $< -o $@

goldbach: goldbach.o
	$(CXX) $(CXXFLAGS) $(OPENMP) $(LDFLAGS) $< -o $@

geomean: geomean.o
	$(CC) $(CFLAGS) $(LDFLAGS) $< -o $@

clz: clz.o
	$(CC) $(CFLAGS) $(LDFLAGS) $< -o $@

ctz: ctz.o
	$(CC) $(CFLAGS) $(LDFLAGS) $< -o $@

findprimes.o: findprimes.c
	$(CC) $(CFLAGS) $(OPENMP) -c $< -o $@

goldbach.o: goldbach.cpp
	$(CXX) $(CXXFLAGS) $(OPENMP) -c $< -o $@

.c.o:
	$(CC) $(CFLAGS) -c $< -o $@

.cpp.o:
	$(CXX) $(CXXFLAGS) -c $< -o $@

clean:
	rm -f *.o $(PROGRAMS)

