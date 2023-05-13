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
#include <cstring>

static std::set<uint64_t> Factors;
static bool Check = false;

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
    if ((static_cast<uint64_t>(I % 2ULL)) == 0UL)
      continue;

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
}

static void CheckFactors() {
  std::cout << "----------------------------" << std::endl;
  for (std::set<uint64_t>::const_iterator I = Factors.begin();
       I != Factors.end(); ++I) {
    if (IsPrime(*I))
      std::cout << *I << " is prime." << std::endl;
    else
      std::cout << *I << " is NOT prime." << std::endl;
  }
  std::cout << "----------------------------" << std::endl;
}

static void PrintUsage() {
  std::cerr << "Usage: primefactors <unsigned-integer> [ --check ]" << std::endl;
}

static void PrintFactors(uint64_t N) {
  std::cout << "Prime Factors of " << N << ":";

  for (std::set<uint64_t>::iterator I = Factors.begin();
       I != Factors.end(); ++I) {
    std::cout << " " << (*I);
  }

  std::cout << std::endl;
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
  PrimeFactors(N);
  PrintFactors(N);

  if (Check)
    CheckFactors();

  return 0;
}

