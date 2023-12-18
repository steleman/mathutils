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
#include <iomanip>
#include <string>
#include <set>
#include <map>
#include <cmath>
#include <cstring>
#include <ctime>
#include <cerrno>

static std::multiset<uint64_t> Factors;
static std::map<uint64_t, uint32_t> FM;
static bool Check = false;

static struct timespec tp_start;
static struct timespec tp_end;

static int Timestamp(struct timespec* ts) {
  if (clock_gettime(CLOCK_PROCESS_CPUTIME_ID, ts) != 0) {
    std::cerr << "clock_gettime(2) failed: " << strerror(errno)
      << std::endl;
    return -1;
  }

  return 0;
}

static void PrintTimespec(const struct timespec* ts) {
  std::cerr << "sec: " << std::setfill('0') << std::setw(9)
    << ts->tv_sec << std::endl;
  std::cerr << "nns: " << std::setfill('0') << std::setw(12)
    << ts->tv_nsec << std::endl;
}

static void PrintTimediff(const struct timespec* start,
                          const struct timespec* end) {
  uint64_t sec = (uint64_t) end->tv_sec - start->tv_sec;
  uint64_t nns = (uint64_t) end->tv_nsec - start->tv_nsec;

  if (nns >= 1000000000) {
    sec += nns / 1000000000;
    nns = nns % 1000000000;
  }

  std::cerr << "CPU time: " << sec << '.' << std::setfill('0') << std::setw(12)
    << nns << " second(s)." << std::endl;
}

static inline bool IsPrime(uint64_t N)
{
  if (N == 0UL)
    return false;

  if (N <= 2UL)
    return true;

  if ((N & 1ULL) == 0)
    return false;

  uint64_t SQR = static_cast<uint64_t>(std::sqrt(N) + 1ULL);

  for (uint64_t I = 3UL; I <= SQR; I += 2UL) {
    if ((static_cast<uint64_t>(N % I)) == 0UL)
      return false;
  }

  return true;
}

static void PrimeFactors(uint64_t N) {
  uint64_t NX = N;

  while ((NX % 2) == 0) {
    Factors.insert(2);
    NX /= 2;
  }

  uint64_t S = (uint64_t) std::sqrt(N);

  for (uint64_t I = 3; I <= S; I += 2) {
    while ((N % I) == 0) {
      Factors.insert(I);
      N /= I;
    }
  }

  if (N > 2 && IsPrime(N))
    Factors.insert(N);
  if (NX > 2 && IsPrime(NX))
    Factors.insert(NX);
}

static void CheckFactors() {
  std::cout << "----------------------------" << std::endl;
  for (std::map<uint64_t, uint32_t>::const_iterator I = FM.begin();
       I != FM.end(); ++I) {
    if (IsPrime((*I).first))
      std::cout << (*I).first << " is prime." << std::endl;
    else
      std::cout << (*I).first << " is NOT prime." << std::endl;
  }
  std::cout << "----------------------------" << std::endl;
}

static void PrintUsage() {
  std::cerr << "Usage: primefactors <unsigned-integer> [ --check ]" << std::endl;
}

static void PrintFactors(uint64_t N) {
  std::cout << "Prime Factors of " << N << ":";

  FM.clear();

  for (std::multiset<uint64_t>::iterator I = Factors.begin();
       I != Factors.end(); ++I) {
    if (!(FM.insert(std::make_pair(*I, 1U)).second)) {
      std::map<uint64_t, uint32_t>::iterator MI = FM.find(*I);
      ++((*MI).second);
    }
  }

  for (std::map<uint64_t, uint32_t>::iterator I = FM.begin();
       I != FM.end(); ++I) {
    std::cout << " " << (*I).first;
  }

  std::cout << std::endl;

  std::map<uint64_t, uint32_t>::const_iterator MIE;
  std::cout << "Product: [";

  for (std::map<uint64_t, uint32_t>::const_iterator MI = FM.begin();
       MI != FM.end(); ++MI) {
    if ((*MI).second == 1U)
      std::cout << (*MI).first;
    else
      std::cout << '(' << (*MI).first << " ** " << (*MI).second << ')';

    MIE = MI;
    ++MIE;
    if (MIE != FM.end())
      std::cout << " * ";
  }

  std::cout << ']' << std::endl;
}

int main(int argc, char* argv[])
{
  if (argc < 2) {
    PrintUsage();
    return 1;
  }

  if (argc == 3 && strcmp(argv[2], "--check") == 0)
    Check = true;

  uint64_t N = (uint64_t) std::stoul(argv[1]);

  Timestamp(&tp_start);
  PrimeFactors(N);
  Timestamp(&tp_end);
  PrintFactors(N);
  PrintTimediff(&tp_start, &tp_end);


  if (Check) {
    Timestamp(&tp_start);
    CheckFactors();
    Timestamp(&tp_end);
    PrintTimediff(&tp_start, &tp_end);
  }

  return 0;
}

