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

#include <iostream>
#include <vector>
#include <set>
#include <cstdint>
#include <cmath>
#include <ctime>
#include <climits>
#include <cstring>
#include <cerrno>
#include <unistd.h>
#include <pthread.h>
#include <sys/select.h>

static uint64_t range_start = 1UL;
static uint64_t range_end = 0UL;
static uint64_t prime_index = 0UL;
static uint32_t nthreads = 4U;
static std::set<uint64_t> prime_storage;
static bool print_header = false;
static bool print_timestamp = false;
static struct timespec ts_begin = { 0, 0 };
static struct timespec ts_end = { 0, 0 };
static pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
static std::vector<pthread_attr_t> tattr;

struct prime_range {
  prime_range() : start(0UL), end(0UL), tid(0U) { }

  prime_range(const prime_range& rhs)
  : start(rhs.start), end(rhs.end), tid(rhs.tid) { }

  ~prime_range() = default;

  prime_range& operator=(const prime_range& rhs) {
    if (this != &rhs) {
      start = rhs.start;
      end = rhs.end;
      tid = rhs.tid;
    }

    return *this;
  }

  uint64_t start;
  uint64_t end;
  uint32_t tid;
};

static pthread_t monitor_thread;
static std::vector<pthread_t> threads;
static std::vector<prime_range> ranges;
static uint32_t thread_count = 0U;

static void print_help(void)
{
  std::cerr << "Usage: findprimes [-s <range-start> (default 1)]" << std::endl;
  std::cerr << "       [ -e <range-end> default ULONG_MAX | ULLONG_MAX)]"
    << std::endl;
  std::cerr << "       [ -b <number-of-bits> (default 64)]" << std::endl;
  std::cerr << "       [ -T <number-of-threads> (default 4)]" << std::endl;
  std::cerr << "       [ -f <output-file> (default stdout)]" << std::endl;
  std::cerr << "       [ -p <print header at the top>]" << std::endl;
  std::cerr << "       [ -t <print prime discovery time>]" << std::endl;
}

static void timestamp(struct timespec* ts)
{
  errno = 0;
  if (clock_gettime(CLOCK_PROCESS_CPUTIME_ID, ts) != 0) {
    (void) fprintf(stderr, "Could not get the timestamp: %s\n",
                   std::strerror(errno));
  }
}

static void print_time(const char* filename)
{
  FILE* fp = NULL;
  if (filename) {
    errno = 0;
    if ((fp = fopen(filename, "a")) == NULL) {
      std::cerr << "Unable to open file '" << filename << "' for writing: "
                     << std::strerror(errno) << std::endl;
      return;
    }
  } else
    fp = stdout;

  uint64_t sec = (uint64_t) ts_end.tv_sec - ts_begin.tv_sec;
  uint64_t nns = (uint64_t) ts_end.tv_nsec - ts_begin.tv_nsec;

  if (nns >= 1000000000) {
    sec += nns / 1000000000;
    nns = nns % 1000000000;
  }

  (void) std::fprintf(fp, "-----\n");
  (void) std::fprintf(fp, "Discovered %lu prime numbers in %lu.%.16lu seconds.\n",
                      prime_index, sec, nns);
  (void) std::fflush(fp);

  if (filename)
    (void) fclose(fp);
}

bool is_prime(uint64_t x)
{
  if (x == 2 || !(x & 0x1) || (x == 0))
    return false;

  uint64_t s = (uint64_t) std::ceil(std::sqrt(x));

  for (uint64_t i = 3UL; i <= s; i += 2UL) {
    if ((x % i) == 0) {
      return false;
    }
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

void add_prime(uint64_t x)
{
  (void) pthread_mutex_lock(&mutex);
  prime_storage.insert(x);
  prime_index = prime_storage.size();
  (void) pthread_mutex_unlock(&mutex);
}

static int print_primes(const char* filename)
{
  FILE* fp = NULL;

  if (filename) {
    errno = 0;
    if ((fp = std::fopen(filename, "w+")) == NULL) {
      (void) std::fprintf(stderr, "Unable to open file '%s' for writing: %s\n",
                          filename, strerror(errno));
      return -1;
    }
  } else
    fp = stdout;

  if (print_header)
    (void) std::fprintf(fp, "List of prime numbers in the range %lu - %lu:\n\n",
                        range_start, range_end);

  for (std::set<uint64_t>::const_iterator pi = prime_storage.begin();
       pi != prime_storage.end(); ++pi)
    (void) std::fprintf(fp, "%lu\n", (*pi));

  (void) fflush(fp);

  if (filename && fp != stdout)
    (void) fclose(fp);

  return 0;
}

extern "C" {
  void* prime_thread_start(void* arg) {
    prime_range* pr = (prime_range*) arg;
    uint32_t pc = 0U;

    for (uint64_t p = pr->start; p <= pr->end; p += 2) {
      if (is_prime(p)) {
        add_prime(p);
        ++pc;
      }
    }

    (void) std::fprintf(stderr, "thread %u [%lu -> %lu] is done [%u].\n",
                        pr->tid, pr->start, pr->end, pc);
    (void) std::fflush(stderr);

    pthread_mutex_lock(&mutex);
    thread_count += 1U;
    pthread_mutex_unlock(&mutex);

    return NULL;
  }

  void* monitor_thread_start(void*) {
    struct timeval ttv = { 0, 100 };
    uint32_t tc = 0U;

    pthread_mutex_lock(&mutex);
    tc = thread_count;
    pthread_mutex_unlock(&mutex);

    while (tc < nthreads) {
      select(0, NULL, NULL, NULL, &ttv);
      ttv.tv_sec = 0;
      ttv.tv_usec = 1000;

      pthread_mutex_lock(&mutex);
      tc = thread_count;
      pthread_mutex_unlock(&mutex);
    }

    ttv.tv_sec = 0;
    ttv.tv_usec = 1000;
    select(0, NULL, NULL, NULL, &ttv);

    (void) std::fprintf(stderr, "monitor thread is done.\n");
    return NULL;
  }
}

static void find_primes(void)
{
  uint64_t eff_range_start = range_start;

  if (eff_range_start == 1UL) {
    add_prime(1UL);
    add_prime(2UL);
    eff_range_start = 3UL;
  } else if (eff_range_start == 2UL) {
    add_prime(2UL);
    eff_range_start = 3UL;
  }

  if ((eff_range_start % 2) == 0)
    eff_range_start += 1;

  uint64_t segment = std::ceil((range_end - eff_range_start) / nthreads);

  ranges.resize(nthreads);

  ranges[0].start = eff_range_start;
  ranges[0].end = ranges[0].start + segment;
  if (!(ranges[0].end & 0x1))
    ranges[0].end += 1UL;

  uint32_t i;
  for (i = 1; i < nthreads - 1; ++i) {
    ranges[i].start = ranges[i - 1].end;
    ranges[i].end = ranges[i].start + segment;
    if (!(ranges[i].end & 0x1))
      ranges[i].end += 1UL;
  }

  ranges[nthreads - 1].start = ranges[i - 1].end;
  ranges[nthreads - 1].end = range_end;

  for (i = 0; i < nthreads; ++i) {
    (void) std::fprintf(stderr, "range[%u]: %lu --> %lu\n",
                        i, ranges[i].start, ranges[i].end);
  }

  tattr.resize(nthreads);
  threads.resize(nthreads);

  for (i = 0; i < nthreads; ++i) {
    (void) pthread_attr_init(&tattr[i]);
    (void) pthread_attr_setdetachstate(&tattr[i], PTHREAD_CREATE_DETACHED);
  }

  struct timeval mtv = { 0, 1000 };

  (void) pthread_create(&monitor_thread, NULL, monitor_thread_start, NULL);
  select(0, NULL, NULL, NULL, &mtv);

  timestamp(&ts_begin);

  for (i = 0; i < nthreads; ++i) {
    std::cerr << "starting thread " << i << " ..." << std::endl;
    ranges[i].tid = i;
    pthread_create(&threads[i], &tattr[i], prime_thread_start, &ranges[i]);
  }

  (void) pthread_join(monitor_thread, NULL);

  timestamp(&ts_end);
}

int main(int argc, char* argv[])
{
  int opt;
  unsigned bits = 64;
  bool ph = false;
  const char* filename = NULL;

  while ((opt = getopt(argc, argv, "hpts:e:b:f:T:")) != -1) {
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
    case 'T':
      nthreads = (uint32_t) strtoul(optarg, NULL, 10);
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

  if (nthreads < 4) {
    (void) std::fprintf(stderr, "At least 4 threads are required.\n");
    return 1;
  }

  if (range_start == 0)
    range_start = 1UL;

  if (range_end == 0)
    range_end = bits == 32 ? UINT_MAX : ULLONG_MAX;

  if (check_bits(bits) != 0)
    return 1;

  find_primes();

  if (print_primes(filename) != 0)
    return 1;

  if (print_timestamp) {
    print_time(filename);
  }

  return 0;
}

