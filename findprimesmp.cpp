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
#include <gmp.h>

struct MPZ {
  MPZ(const mpz_t& X, uint32_t Bits) : MPV() {
    mpz_init2(MPV, Bits);
    mpz_set(MPV, X);
  }

  mpz_t MPV;
};

template<typename T>
struct mpz_less : public std::binary_function<T, T, bool> {
public:
  constexpr inline bool operator()(const T& L, const T& R) const {
    return mpz_cmp(L->MPV, R->MPV) < 0;
  }
};

static std::string RangeStart = "18446744073709551615";
static std::string RangeEnd;
static uint32_t NThreads = 4U;
static uint32_t Bits = 128U;
static std::set<MPZ*, mpz_less<MPZ*>> PrimeStorage;
static bool PrintHeader = false;
static bool PrintTimestamp = false;
static struct timespec ts_begin = { 0, 0 };
static struct timespec ts_end = { 0, 0 };
static pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
static std::vector<pthread_attr_t> tattr;

struct prime_range {
  prime_range() : Start(), End(), TId(0U) {
    mpz_init2(Start, Bits);
    mpz_init2(End, Bits);
  }

  prime_range(const prime_range& rhs)
  : Start(), End(), TId(rhs.TId) {
    mpz_init2(Start, Bits);
    mpz_init2(End, Bits);

    mpz_set(Start, rhs.Start);
    mpz_set(End, rhs.End);
  }

  ~prime_range() = default;

  prime_range& operator=(const prime_range& rhs) {
    if (this != &rhs) {
      mpz_set(Start, rhs.Start);
      mpz_set(End, rhs.End);
      TId = rhs.TId;
    }

    return *this;
  }

  mpz_t Start;
  mpz_t End;
  uint32_t TId;
};

static pthread_t monitor_thread;
static std::vector<pthread_t> threads;
static std::vector<prime_range> ranges;
static uint32_t thread_count = 0U;

#ifdef __cplusplus
extern "C" {
#endif

void (*gmp_free_mem_func)(void*, size_t);

#ifdef __cplusplus
} // extern "C"
#endif

bool IsPrime(const mpz_t& X);
void PrintMPZ(const mpz_t& M, const char* N);

static void PrintHelp() {
  std::cerr << "Usage: findprimesmp -s <range-start> "
    << "(default 18446744073709551615)" << std::endl;
  std::cerr << "                    -e <range-end>" << std::endl;
  std::cerr << "       [ -b <number-of-bits> (default 128)]" << std::endl;
  std::cerr << "       [ -T <number-of-threads> (default 4)]" << std::endl;
  std::cerr << "       [ -f <output-file> (default stdout)]" << std::endl;
  std::cerr << "       [ -p (print header at the top)]" << std::endl;
  std::cerr << "       [ -t (print prime discovery time)]" << std::endl;
}

static void Timestamp(struct timespec* ts) {
  errno = 0;
  if (clock_gettime(CLOCK_PROCESS_CPUTIME_ID, ts) != 0) {
    (void) fprintf(stderr, "Could not get the timestamp: %s\n",
                   std::strerror(errno));
  }
}

static void PrintTime(const char* Filename) {
  FILE* fp = NULL;
  if (Filename) {
    errno = 0;
    if ((fp = fopen(Filename, "a")) == NULL) {
      std::cerr << "Unable to open file '" << Filename << "' for writing: "
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
                      PrimeStorage.size(), sec, nns);
  (void) std::fflush(fp);

  if (Filename)
    (void) fclose(fp);
}

static void Cleanup() {
  for (std::set<MPZ*>::const_iterator PI = PrimeStorage.begin();
       PI != PrimeStorage.end(); ++PI) {
    mpz_clear((*PI)->MPV);
    delete *PI;
  }
}

void PrintMPZ(const mpz_t& M, const char* N) {
  if (char* P = mpz_get_str(NULL, 10, M)) {
    std::cerr << N << ": " << P << std::endl;
    mp_get_memory_functions(NULL, NULL, &gmp_free_mem_func);
    gmp_free_mem_func(P, std::strlen(P) + 1);
  }
}

static int CheckBits(uint32_t bits) {
  if (bits < 128U) {
    std::cerr << "Invalid number of bits. At least 128 bits are required.";
    return -1;
  }

  return 0;
}

void AddPrime(const mpz_t& X) {
  MPZ* M = new MPZ(X, Bits);
  (void) pthread_mutex_lock(&mutex);
  PrimeStorage.insert(M);
  (void) pthread_mutex_unlock(&mutex);
}

static int PrintPrimes(const char* Filename) {
  FILE* fp = NULL;

  if (Filename) {
    errno = 0;
    if ((fp = std::fopen(Filename, "w+")) == NULL) {
      (void) std::fprintf(stderr, "Unable to open file '%s' for writing: %s\n",
                          Filename, std::strerror(errno));
      return -1;
    }
  } else
    fp = stdout;

  if (PrintHeader)
    (void) std::fprintf(fp, "List of prime numbers in the range %s - %s:\n\n",
                        RangeStart.c_str(), RangeEnd.c_str());

  for (std::set<MPZ*>::const_iterator PI = PrimeStorage.begin();
       PI != PrimeStorage.end(); ++PI) {
    char* P = mpz_get_str(NULL, 10, (*PI)->MPV);
    (void) std::fprintf(fp, "%s\n", P);
    mp_get_memory_functions(NULL, NULL, &gmp_free_mem_func);
    gmp_free_mem_func(P, std::strlen(P) + 1);
  }

  (void) std::fflush(fp);

  if (Filename && fp != stdout)
    (void) fclose(fp);

  return 0;
}

extern "C" {
  void* prime_thread_start(void* Arg) {
    prime_range* PR = (prime_range*) Arg;
    uint32_t PC = 0U;

    mpz_t P;
    mpz_init2(P, Bits);
    mpz_set(P, PR->Start);

    while (mpz_cmp(P, PR->End) <= 0) {
      if (IsPrime(P)) {
        AddPrime(P);
        ++PC;
      }

      mpz_add_ui(P, P, 2UL);
    }

    char* PS = mpz_get_str(NULL, 10, PR->Start);
    char* PE = mpz_get_str(NULL, 10, PR->End);
    (void) std::fprintf(stderr, "thread %u [%s -> %s] is done [%u].\n",
                        PR->TId, PS, PE, PC);
    (void) std::fflush(stderr);
    mp_get_memory_functions(NULL, NULL, &gmp_free_mem_func);
    gmp_free_mem_func(PS, std::strlen(PS) + 1);
    gmp_free_mem_func(PE, std::strlen(PE) + 1);

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

    while (tc < NThreads) {
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

bool IsPrime(const mpz_t& X) {
  if (char* P = mpz_get_str(NULL, 10, X)) {
    size_t SL = std::strlen(P);

    if (SL == 0UL)
      return false;
    else if (SL == 1UL && (P[0] == '1' || P[0] == '2'))
      return true;

    switch (P[SL - 1]) {
    case '0':
    case '2':
    case '4':
    case '6':
    case '8':
      mp_get_memory_functions(NULL, NULL, &gmp_free_mem_func);
      gmp_free_mem_func(P, std::strlen(P) + 1);
      return false;
      break;
    case '5':
      return SL == 1UL;
      break;
    default:
      break;
    }

    mp_get_memory_functions(NULL, NULL, &gmp_free_mem_func);
    gmp_free_mem_func(P, std::strlen(P) + 1);
    bool NP = false;

    mpz_t SQRT;
    mpz_t ON;
    mpz_init2(SQRT, Bits);
    mpz_init2(ON, Bits);

    mpz_set_ui(ON, 1UL);
    mpz_sqrt(SQRT, X);
    mpz_add(SQRT, ON, SQRT);

    mpz_t I;
    mpz_t R;
    mpz_t T;
    mpz_t Z;

    mpz_init2(I, Bits);
    mpz_init2(R, Bits);
    mpz_init2(T, Bits);
    mpz_init2(Z, Bits);

    mpz_set_ui(I, 3UL);
    mpz_set_ui(T, 2UL);
    mpz_set_ui(Z, 0UL);

    while (mpz_cmp(SQRT, I) > 0) {
      mpz_mod(R, X, I);

      if (mpz_cmpabs(R, Z) == 0) {
        NP = true;
        break;
      }

      mpz_add(I, T, I);
    }

    mpz_clear(Z);
    mpz_clear(T);
    mpz_clear(R);
    mpz_clear(I);
    mpz_clear(ON);
    mpz_clear(SQRT);

    return !NP;
  }

  return false;
}

void AdjustRanges() {
  size_t SL = RangeStart.length();
  switch (RangeStart[SL - 1]) {
  case '0':
  case '2':
  case '4':
  case '6':
  case '8':
    RangeStart[SL - 1] = RangeStart[SL - 1] + 1;
    std::cerr << "range lower bound adjusted to " << RangeStart
      << '.' << std::endl;
    break;
  default:
    break;
  }

  SL = RangeEnd.length();
  switch (RangeEnd[SL - 1]) {
  case '0':
  case '2':
  case '4':
  case '6':
  case '8':
    RangeEnd[SL - 1] = RangeEnd[SL - 1] + 1;
    std::cerr << "range upper bound adjusted to " << RangeEnd
      << '.' << std::endl;
    break;
  default:
    break;
  }
}

void FindPrimes() {
  AdjustRanges();

  mpz_t RS;
  mpz_t RE;

  mpz_init2(RS, Bits);
  mpz_init2(RE, Bits);

  mpz_set_str(RS, RangeStart.c_str(), 10);
  mpz_set_str(RE, RangeEnd.c_str(), 10);

  mpz_t DF;
  mpz_init2(DF, Bits);
  mpz_sub(DF, RE, RS);

  mpz_t R;
  mpz_t Q;
  mpz_init2(R, Bits);
  mpz_init2(Q, Bits);

  mpz_cdiv_q_ui(Q, DF, NThreads);
  mpz_cdiv_r_ui(R, DF, NThreads);

  ranges.resize(NThreads);

  mpz_t RRS;
  mpz_t RRE;
  mpz_t ON;
  mpz_t T;

  mpz_init2(RRS, Bits);
  mpz_init2(RRE, Bits);
  mpz_init2(ON, Bits);
  mpz_init2(T, Bits);

  mpz_set_ui(ON, 1UL);
  mpz_set_ui(T, 2UL);

  mpz_set(ranges[0].Start, RS);
  mpz_add(RRE, RS, Q);
  mpz_set(ranges[0].End, RRE);

  uint32_t i;
  for (i = 1; i < NThreads - 1; ++i) {
    mpz_set(ranges[i].Start, ranges[i - 1].End);
    mpz_add(ranges[i].Start, ON, ranges[i].Start);
    mpz_set(RRE, ranges[i].Start);
    mpz_add(ranges[i].End, RRE, Q);

    if (mpz_fdiv_ui(ranges[i].End, 2UL) == 0UL)
      mpz_add(ranges[i].End, ON, ranges[i].End);

    if (mpz_fdiv_ui(ranges[i].Start, 2UL) == 0UL)
      mpz_sub(ranges[i].Start, ranges[i].Start, ON);
  }

  mpz_set(ranges[NThreads - 1].Start, ranges[i - 1].End);
  if (mpz_fdiv_ui(ranges[NThreads - 1].Start, 2UL) == 0UL)
    mpz_sub(ranges[NThreads - 1].Start, ranges[i].Start, ON);

  mpz_set(ranges[NThreads - 1].End, RE);

  for (i = 0; i < NThreads; ++i) {
    char* PS = mpz_get_str(NULL, 10, ranges[i].Start);
    char* PE = mpz_get_str(NULL, 10, ranges[i].End);
    (void) std::fprintf(stderr, "range[%u]: %s --> %s\n", i, PS, PE);
    mp_get_memory_functions(NULL, NULL, &gmp_free_mem_func);
    gmp_free_mem_func(PS, std::strlen(PS) + 1);
    gmp_free_mem_func(PE, std::strlen(PE) + 1);
  }

  tattr.resize(NThreads);
  threads.resize(NThreads);

  for (i = 0; i < NThreads; ++i) {
    (void) pthread_attr_init(&tattr[i]);
    (void) pthread_attr_setdetachstate(&tattr[i], PTHREAD_CREATE_DETACHED);
  }

  struct timeval mtv = { 0, 1000 };

  (void) pthread_create(&monitor_thread, NULL, monitor_thread_start, NULL);
  select(0, NULL, NULL, NULL, &mtv);

  Timestamp(&ts_begin);

  for (i = 0; i < NThreads; ++i) {
    std::cerr << "starting thread " << i << " ..." << std::endl;
    ranges[i].TId = i;
    pthread_create(&threads[i], &tattr[i], prime_thread_start, &ranges[i]);
  }

  (void) pthread_join(monitor_thread, NULL);

  Timestamp(&ts_end);

  mpz_clear(T);
  mpz_clear(ON);
  mpz_clear(RRE);
  mpz_clear(RRS);
  mpz_clear(DF);
  mpz_clear(RE);
  mpz_clear(RS);
  mpz_clear(R);
  mpz_clear(Q);
}

int main(int argc, char* argv[])
{
  int opt;
  bool ph = false;
  const char* Filename = NULL;

  if (argc < 4) {
    PrintHelp();
    return 1;
  }

  while ((opt = getopt(argc, argv, "hpts:e:b:f:T:")) != -1) {
    switch (opt) {
    case 'h':
      ph = true;
      break;
    case 'b':
      Bits = (uint32_t) std::strtoul(optarg, NULL, 10);
      break;
    case 's':
      RangeStart = optarg;
      break;
    case 'e':
      RangeEnd = optarg;
      break;
    case 'f':
      Filename = optarg;
      break;
    case 'p':
      PrintHeader = true;
      break;
    case 't':
      PrintTimestamp = true;
      break;
    case 'T':
      NThreads = (uint32_t) std::strtoul(optarg, NULL, 10);
      break;
    default:
      ph = true;
      break;
    }
  }

  if (ph) {
    PrintHelp();
    return 1;
  }

  if (NThreads < 4) {
    (void) std::fprintf(stderr, "At least 4 threads are required.\n");
    return 1;
  }

  if (RangeStart.empty())
    RangeStart = "18446744073709551615";

  if (RangeEnd.empty()) {
    std::cerr << "A range upper bound must be specified." << std::endl;
    return 1;
  }

  if (CheckBits(Bits) != 0)
    return 1;

  FindPrimes();

  if (PrintPrimes(Filename) != 0)
    return 1;

  if (PrintTimestamp)
    PrintTime(Filename);

  Cleanup();

  return 0;
}

