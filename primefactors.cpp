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

std::set<uint64_t> Factors;

static void PrimeFactors(uint64_t N) {
  while ((N % 2) == 0) {
    Factors.insert(2);
    N /= 2;
  }

  uint64_t S = (uint64_t) sqrtl(N);

  for (uint64_t I = 3; I <= S; I += 2) {
    while ((N % I) == 0) {
      Factors.insert(I);
      N /= I;
    }
  }

  if (N > 2)
    Factors.insert(N);
}

static void PrintUsage() {
  std::cerr << "Usage: primefactors <integer>" << std::endl;
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
  if (argc != 2) {
    PrintUsage();
    return 1;
  }

  uint64_t N = (uint64_t) std::stoul(argv[1]);
  PrimeFactors(N);
  PrintFactors(N);
  return 0;
}

