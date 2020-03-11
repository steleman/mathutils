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
#include <string>
#include <set>
#include <cmath>
#include <unistd.h>

#if defined(_OPENMP)
#include <omp.h>
#endif

std::set<uint64_t> Primes;
bool PrintPrimes = false;

bool isprime(uint64_t N) {
  if (N == 2) return true;

  if (!(N & 1) || (N == 0))
    return false;

  uint64_t S = (uint64_t) ceill(sqrtl(N));

  for (uint64_t I = 3UL; I <= S; ++I) {
    if ((N % I) == 0)
      return false;
  }

  return true;
}

inline void addprime(uint64_t N) {
#pragma omp critical
  Primes.insert(N);
}

void findprimes(uint64_t N) {
  Primes.insert(1);
  Primes.insert(2);
  Primes.insert(3);

#if defined(_OPENMP)
#pragma omp parallel for
#endif
  for (uint64_t I = 3; I < N; I += 2) {
    if (isprime(I))
      addprime(I);
  }
}

bool checknumber(uint64_t N) {
  if (N <= 2UL) {
    std::cerr << "N=" << N << " is less than 2!" << std::endl;
    return false;
  } else if (N & 1) {
    std::cerr << "N=" << N << " is not an even number!" << std::endl;
    return false;
  }

  return true;
}

static void printprimes() {
  for (std::set<uint64_t>::iterator I = Primes.begin();
       I != Primes.end(); ++I)
    std::cerr << *I << std::endl;
}

bool goldbach(uint64_t N, std::pair<uint64_t, uint64_t>& R) {
  R.first  = 0UL;
  R.second = 0UL;

  for (std::set<uint64_t>::iterator I = Primes.begin();
       I != Primes.end(); ++I) {
    uint64_t D = N - (*I);

    if (Primes.find(D) != Primes.end()) {
      R.first = *I;
      R.second = D;
      return true;
    }
  }

  return false;
}

void printUsage() {
  std::cerr << "Usage: goldbach -N <even-integer>" << std::endl
    << "             [ -P (print prime numbers up to N) ]" << std::endl
    << "             [ -h (print this help message) ]" << std::endl;
}

int main(int argc, char* argv[])
{
  if (argc < 3) {
    printUsage();
    return 1;
  }

  uint64_t N = 0UL;
  int c;

  while ((c = getopt(argc, argv, "hPN:")) != -1) {
    switch (c) {
    case 'P':
      PrintPrimes = true;
      break;
    case 'N':
      N = std::stoul(optarg);
      if (!checknumber(N))
        return 1;
      break;
    default:
      printUsage();
      return 1;
      break;
    }
  }

  findprimes(N);

  if (PrintPrimes)
    printprimes();

  std::pair<uint64_t, uint64_t> R;

  if (!goldbach(N, R)) {
    std::cerr << "Could not find a pair of prime numbers "
      << "to satisfy Goldbach's Conjecture." << std::endl;
    std::cerr << "This is severely weird!" << std::endl;
    return 1;
  }

  std::cout << "Goldbach's Conjecture for " << N
    << " is satisfied by " << R.first << " + "
    << R.second << "." << std::endl;

  return 0;
}

