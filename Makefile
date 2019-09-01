CC = /usr/bin/gcc

CFLAGS = -D_GNU_SOURCE -D_XOPEN_SOURCE=700
CFLAGS += -O2 -std=c99 -mtune=core-avx2
CFLAGS += -ftree-vectorize -ftree-slp-vectorize
CFLAGS += -Wall -Wextra -flto=32

LDFLAGS = -lm

PROGRAMS = isprime popcnt clz ctz geomean

all: $(PROGRAMS)

isprime: isprime.o
	$(CC) $(CFLAGS) $(LDFLAGS) $< -o $@

popcnt: popcnt.o
	$(CC) $(CFLAGS) $(LDFLAGS) $< -o $@

geomean: geomean.o
	$(CC) $(CFLAGS) $(LDFLAGS) $< -o $@

clz: clz.o
	$(CC) $(CFLAGS) $(LDFLAGS) $< -o $@

ctz: ctz.o
	$(CC) $(CFLAGS) $(LDFLAGS) $< -o $@

.c.o:
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -f *.o $(PROGRAMS)

