/*
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 *
 * Copyright (C) 2019 Stefan Teleman.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <stdbool.h>
#include <limits.h>
#include <math.h>
#include <time.h>
#include <unistd.h>
#include <errno.h>

static uint64_t range_start = 1UL;
static uint64_t range_end = 0UL;
static uint64_t nprimes = 0UL;
static uint64_t prime_index = 0UL;
static uint64_t* prime_storage = NULL;
static bool print_header = false;
static bool print_timestamp = false;
static struct timespec ts_begin = { 0, 0 };
static struct timespec ts_end = { 0, 0 };
static struct timespec ts_correct = { 0, 0 };

static void print_help(void)
{
  (void) fprintf(stderr, "Usage: findprimes [-s <range-start> (default 1)]\n");
  (void) fprintf(stderr, "       [ -e <range-end> "
                         "(default ULONG_MAX | ULLONG_MAX)]\n");
  (void) fprintf(stderr, "       [ -b <number-of-bits> (default 64)]\n");
  (void) fprintf(stderr, "       [ -f <output-file> (default stdout)]\n");
  (void) fprintf(stderr, "       [ -p <print header at the top>]\n");
  (void) fprintf(stderr, "       [ -t <print prime discovery time>]\n");
}

#if defined(_OPENMP)
static int qsort_compare(const void* l, const void* r)
{
  return *(uint64_t*) l - *(uint64_t*) r;
}
#endif

static void timestamp(struct timespec* ts)
{
  errno = 0;
  if (clock_gettime(CLOCK_PROCESS_CPUTIME_ID, ts) != 0) {
    (void) fprintf(stderr, "Could not get the timestamp: %s\n",
                   strerror(errno));
  }
}

static void print_time(const char* filename)
{
  FILE* fp = NULL;
  if (filename) {
    errno = 0;
    if ((fp = fopen(filename, "a")) == NULL) {
      (void) fprintf(stderr, "Unable to open file '%s' for writing: '%s'\n",
                     filename, strerror(errno));
      return;
    }
  } else
    fp = stdout;

  struct timespec ts_diff = { 0, 0 };
  ts_diff.tv_sec = ts_end.tv_sec - ts_begin.tv_sec;
  ts_diff.tv_nsec = ts_end.tv_nsec - ts_begin.tv_nsec;

  ts_diff.tv_sec -= ts_correct.tv_sec;
  ts_diff.tv_nsec -= ts_correct.tv_nsec;

  double t = (double) ts_diff.tv_nsec / 100000000;
  t += (double) ts_diff.tv_sec;

  (void) fprintf(fp, "-----\n");
  (void) fprintf(fp, "Discovered %lu prime numbers in %lf seconds.\n",
                 prime_index, t);
  (void) fflush(fp);

  if (filename)
    (void) fclose(fp);
}

static bool is_prime(uint64_t x)
{
  if (x == 2)
    return true;

  if (!(x & 1) || (x == 0))
    return false;

  uint64_t s = (uint64_t) ceill(sqrtl(x));

  for (uint64_t i = 3UL; i <= s; ++i) {
    if ((x % i) == 0)
      return false;
  }

  return true;
}

static int check_bits(unsigned bits)
{
  if ((bits == 32) && (range_end > UINT_MAX)) {
    (void) fprintf(stderr, "end-range exceeds 32-bit UINT_MAX.\n");
    return -1;
  }

  return 0;
}

static int allocate_storage(void)
{
  uint64_t span = range_end - range_start;

  if (span == 0) {
    (void) fprintf(stderr, "A search span range of zero makes no sense\n");
    return -1;
  }

  // Crude approximation. This is guaranteed to over-allocate.
  if (span < 10000UL)
    nprimes = span;
  else if (span < 1000000UL)
    nprimes = span / 5UL;
  else
    nprimes = span / 10UL;

  errno = 0;

  if ((prime_storage = malloc(nprimes * sizeof(uint64_t))) == NULL) {
    (void) fprintf(stderr,
                   "Unable to allocate storage for sequence of primes: %s\n",
                   strerror(errno));
    return -1;
  }

  (void) memset(prime_storage, 0, nprimes * sizeof(uint64_t));
  return 0;
}

static void add_prime(uint64_t x)
{
#if defined(_OPENMP)
#pragma omp critical
#endif

  prime_storage[prime_index++] = x;
}

static int print_primes(const char* filename)
{
  FILE* fp = NULL;

  if (filename) {
    errno = 0;
    if ((fp = fopen(filename, "w+")) == NULL) {
      (void) fprintf(stderr, "Unable to open file '%s' for writing: %s\n",
                     filename, strerror(errno));
      return -1;
    }
  } else
    fp = stdout;

  if (print_header)
    (void) fprintf(fp, "List of prime numbers in the range %lu - %lu:\n\n",
                   range_start, range_end);

  for (uint64_t i = 0; i < prime_index; ++i)
    (void) fprintf(fp, "%lu\n", prime_storage[i]);

  (void) fflush(fp);

  if (filename)
    (void) fclose(fp);

  return 0;
}

static void find_primes(void)
{
  uint64_t effective_range_start = range_start;

  if (effective_range_start == 1UL) {
    add_prime(1UL);
    add_prime(2UL);
    effective_range_start = 3UL;
  } else if (effective_range_start == 2UL) {
    add_prime(2UL);
    effective_range_start = 3UL;
  }

  if ((effective_range_start % 2) == 0)
    effective_range_start += 1;

  timestamp(&ts_begin);

  uint64_t i;

#if defined(_OPENMP)
#pragma omp parallel for private (i)
#endif

  for (i = effective_range_start; i <= range_end; i += 2) {
    if (is_prime(i))
      add_prime(i);
  }

  timestamp(&ts_end);
}

static void apply_timestamp_correction(void)
{
  struct timespec tsb = { 0, 0 };
  struct timespec tse = { 0, 0 };

  timestamp(&tsb);
  prime_storage[prime_index] = (uint64_t) -1;
  timestamp(&tse);

  ts_correct.tv_sec = tse.tv_sec - tsb.tv_sec;
  ts_correct.tv_nsec = tse.tv_nsec - tsb.tv_nsec;

  ts_correct.tv_sec *= prime_index - 1;
  ts_correct.tv_nsec *= prime_index - 1;
}

int main(int argc, char* argv[])
{
  int opt;
  unsigned bits = 64;
  bool ph = false;
  const char* filename = NULL;

  while ((opt = getopt(argc, argv, "hpts:e:b:f:")) != -1) {
    switch (opt) {
    case 'h':
      ph = true;
      break;
    case 'b':
      bits = (unsigned) strtoul(optarg, NULL, 10);
      break;
    case 's':
      range_start = (uint64_t) strtoul(optarg, NULL, 10);
      break;
    case 'e':
      range_end = (uint64_t) strtoul(optarg, NULL, 10);
      break;
    case 'f':
      filename = optarg;
      break;
    case 'p':
      print_header = true;
      break;
    case 't':
      print_timestamp = true;
      break;
    default:
      ph = true;
      break;
    }
  }

  if (ph) {
    print_help();
    return 1;
  }

  if (range_start == 0)
    range_start = 1UL;

  if (range_end == 0)
    range_end = bits == 32 ? UINT_MAX : ULLONG_MAX;

  if (check_bits(bits) != 0)
    return 1;

  if (allocate_storage() != 0)
    return -1;

  find_primes();

#if defined(_OPENMP)
  qsort(prime_storage, prime_index - 1, sizeof(uint64_t), qsort_compare);
#endif

  if (print_primes(filename) != 0)
    return 1;

  if (print_timestamp) {
    apply_timestamp_correction();
    print_time(filename);
  }

  return 0;
}

