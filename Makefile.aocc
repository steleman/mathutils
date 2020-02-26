CC = /opt/amd/aocc-compiler-2.0.0/bin/clang

CFLAGS = -D_GNU_SOURCE -D_XOPEN_SOURCE=700
CFLAGS += -O3 -std=c99 -march=znver2
CFLAGS += -ftree-vectorize -ftree-slp-vectorize
CFLAGS += -Wall -Wextra
LDFLAGS = -L/opt/amd/aocc-compiler-2.0.0/bin/lib -lamdlibm
LDFLAGS += -Wl,-rpath -Wl,/opt/amd/aocc-compiler-2.0.0/lib -lm
LDFLAGS += -fuse-ld=gold
LDFLAGS_OPENMP = -L/opt/amd/aocc-compiler-2.0.0/lib -lomp
OPENMP = -fopenmp

PROGRAMS = isprime popcnt clz ctz geomean findprimes

all: $(PROGRAMS)

isprime: isprime.o
	$(CC) $(CFLAGS) $(LDFLAGS) $< -o $@

popcnt: popcnt.o
	$(CC) $(CFLAGS) $(LDFLAGS) $< -o $@

findprimes: findprimes.o
	$(CC) $(CFLAGS) $(OPENMP) $(LDFLAGS) $(LDFLAGS_OPENMP) $< -o $@

geomean: geomean.o
	$(CC) $(CFLAGS) $(LDFLAGS) $< -o $@

clz: clz.o
	$(CC) $(CFLAGS) $(LDFLAGS) $< -o $@

ctz: ctz.o
	$(CC) $(CFLAGS) $(LDFLAGS) $< -o $@

findprimes.o: findprimes.c
	$(CC) $(CFLAGS) $(OPENMP) -c $< -o $@

.c.o:
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -f *.o $(PROGRAMS)

