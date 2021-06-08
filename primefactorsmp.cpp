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
#include <cstring>
#include <cmath>

#include <unistd.h>
#include <gmp.h>

#ifdef __cplusplus
extern "C" {
#endif

static void (*gmp_free_mem_func)(void*, size_t);

#ifdef __cplusplus
} // extern "C"
#endif

class MPZ {
private:
  MPZ() = delete;

public:
  MPZ(const mpz_t& V, unsigned NumBits) : MP(), S() {
    mpz_init2(MP, NumBits);
    mpz_set(MP, V);
  }

  ~MPZ() {
    mpz_clear(MP);
  }

  const std::string& AsString() {
    char* P = mpz_get_str(NULL, 10, MP);
    if (P) {
      S = P;
      mp_get_memory_functions(NULL, NULL, &gmp_free_mem_func);
      gmp_free_mem_func(P, std::strlen(P) + 1);
    } else {
      S = "<null>";
    }

    return S;
  }

private:
  mpz_t MP;
  std::string S;
};

std::set<MPZ*> Factors;
static unsigned NumBits = static_cast<unsigned>(~0x0);

static void PrimeFactors(mpz_t N, unsigned NumBits) {
  mpz_t Q;
  mpz_t R;
  mpz_t D;
  mpz_t Z;
  mpz_t NL;

  mpz_init2(Q, NumBits);
  mpz_init2(R, NumBits);
  mpz_init2(D, NumBits);
  mpz_init2(Z, NumBits);
  mpz_init2(NL, NumBits);

  mpz_set(NL, N);
  mpz_set_ui(D, 2UL);
  mpz_set_ui(Z, 0UL);

  mpz_mod(R, NL, D);

  while (mpz_cmpabs(R, Z) == 0) {
    Factors.insert(new MPZ(D, NumBits));
    mpz_cdiv_q(Q, NL, D);
    mpz_set(NL, Q);
    mpz_mod(R, NL, D);
  }

  mpz_t S;
  mpz_init2(S, NumBits);

  mpz_sqrt(S, NL);

  mpz_t I;
  mpz_init2(I, NumBits);
  mpz_set_ui(I, 3UL);

  int K = mpz_cmp(I, S);

  while (K <= 0) {
    mpz_mod(R, NL, I);

    while (mpz_cmpabs(R, Z) == 0) {
      Factors.insert(new MPZ(I, NumBits));
      mpz_cdiv_q(Q, NL, I);
      mpz_set(NL, Q);
      mpz_mod(R, NL, I);
    }

    mpz_add_ui(I, I, 2UL);
    K = mpz_cmp(I, S);
  }

  K = mpz_cmp(NL, D);

  if (K > 0)
    Factors.insert(new MPZ(NL, NumBits));

  mpz_clear(Q);
  mpz_clear(R);
  mpz_clear(D);
  mpz_clear(I);
  mpz_clear(S);
  mpz_clear(Z);
  mpz_clear(NL);
}

static void Cleanup() {
  for (std::set<MPZ*>::iterator I = Factors.begin();
       I != Factors.end(); ++I)
    delete *I;
}

static void PrintUsage() {
  std::cerr << "Usage: primefactorsmp -b <number-of-bits> <unsigned integer>"
    << std::endl;
}

static void PrintFactors(const mpz_t& N) {
  MPZ* NS = new MPZ(N, NumBits);
  std::cout << "Prime Factors of " << NS->AsString() << ":";

  std::set<std::string> FS;
  for (std::set<MPZ*>::iterator I = Factors.begin();
       I != Factors.end(); ++I)
    (void) FS.insert((*I)->AsString());

  for (std::set<std::string>::iterator I = FS.begin(); I != FS.end(); ++I)
    std::cout << " " << *I;

  std::cout << std::endl;
  delete NS;
}

int main(int argc, char* argv[])
{
  if (argc != 4) {
    PrintUsage();
    return 1;
  }

  int c;

  while ((c = getopt(argc, argv, "hb:")) != -1) {
    switch (c) {
    case 'b':
      NumBits = (unsigned) std::stoul(argv[2]);
      break;
    case 'h':
      PrintUsage();
      return 0;
      break;
    default:
      PrintUsage();
      return 1;
      break;
    }
  }

  if (NumBits == static_cast<unsigned>(~0x0)) {
    std::cerr << "Error: Invalid number of bits!" << std::endl;
    return 1;
  }

  mpz_t N;
  mpz_init2(N, NumBits);

  if (mpz_set_str(N, argv[argc - 1], 10) != 0) {
    std::cerr << "Error: Could not convert " << argv[1]
      << " to a MP Integer Value!" << std::endl;
    return 1;
  }

  PrimeFactors(N, NumBits);
  PrintFactors(N);
  Cleanup();

  return 0;
}

