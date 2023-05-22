#include <iostream>
#include <string>
#include <cstring>
#include <cmath>
#include <ctime>
#include <unistd.h>
#include <gmp.h>

uint32_t Bits = 128;

int32_t NotPrime(const char* argv) {
  std::cerr << argv << " is not prime." << std::endl;
  return 0;
}

int32_t Prime(const char* argv) {
  std::cerr << argv << " is prime." << std::endl;
  return 0;
}

int32_t IsSmallPrime(const char* argv) {
  bool NP = false;
  uint32_t X = (uint32_t) std::stoul(argv);
  uint32_t SQ = (uint32_t) std::ceil(std::sqrt(X));

  if (X == 2UL || !(X & 0x1) || X == 0UL) {
    NP = true;
    goto shortcut;
  }

  for (uint32_t I = 3; I <= SQ; I += 2) {
    if ((X % I) == 0) {
      NP = true;
      goto shortcut;
    }
  }

shortcut:
  std::cerr << argv << " is " << (NP ? "not prime." : "prime.") << std::endl;
  return 0;
}

int32_t IsBigPrime(const char* argv, size_t SL) {
  switch (argv[SL - 1]) {
  case '0':
  case '2':
  case '4':
  case '5':
  case '6':
  case '8':
    return NotPrime(argv);
    break;
  default:
    break;
  }

  bool NP = false;

  mpz_t X;
  mpz_init2(X, Bits);
  mpz_set_str(X, argv, 10);

  mpz_t SQRT;
  mpz_t ON;
  mpz_init2(SQRT, Bits);
  mpz_init2(ON, Bits);

  mpz_set_ui(ON, 1UL);
  mpz_sqrt(SQRT, X);
  mpz_add(SQRT, ON, SQRT);

  mpz_t I;
  mpz_t D;
  mpz_t R;
  mpz_t T;
  mpz_t Z;

  mpz_init2(I, Bits);
  mpz_init2(D, Bits);
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
  mpz_clear(D);
  mpz_clear(I);
  mpz_clear(ON);
  mpz_clear(SQRT);
  mpz_clear(X);

  return NP ? NotPrime(argv) : Prime(argv);
}

static void PrintUsage() {
  std::cerr << "Usage: isprimemp -b <number-of-bits> <unsigned integer>"
    << std::endl;
}

int main(int argc, char* const argv[])
{
  if (argc != 4) {
    PrintUsage();
    return 1;
  }

  int opt;

  while ((opt = getopt(argc, argv, "hb:")) != -1) {
    switch (opt) {
    case 'b':
      Bits = (int32_t) std::stoul(optarg);
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

  if (Bits == static_cast<unsigned>(~0x0) || Bits < 32U) {
    std::cerr << "Error: Invalid number of bits!" << std::endl;
    return 1;
  }

  size_t SL = std::strlen(argv[argc - 1]);

  if (SL == 1)
    return IsSmallPrime(argv[argc - 1]);

  return IsBigPrime(argv[argc - 1], SL);
}

