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
#include <math.h>
#include <unistd.h>
#include <getopt.h>
#include <gmp.h>

uint32_t Bits = 128U;

void (*gmp_free_mem_func)(void*, size_t);

void print_usage(const char* prog) {
  (void) fprintf(stderr, "Usage: %s -b <bits> <base-large-integer> "
                 "<arithmetic operator> <operand-integer>\n", basename(prog));
  (void) fprintf(stderr, "Example: %s -b 256 "
                 "18446744073709551615 + 18446744073709551615\n", basename(prog));
}

void invalid_operator(const char* prog, char operator) {
  (void) fprintf(stderr, "%s: error: invalid arithmetic operator '%c'\n",
                 basename(prog), operator);
}

static struct option long_options[] = {
  { "bits", required_argument, 0, 'b' }
};

int addmpz(const char* base, const char* operand) {
  mpz_t B;
  mpz_t O;
  mpz_t S;

  mpz_init2(B, Bits);
  mpz_init2(O, Bits);
  mpz_init2(S, Bits);

  mpz_set_str(B, base, 10);
  mpz_set_str(O, operand, 10);
  mpz_add(S, B, O);

  char* P = mpz_get_str(NULL, 10, S);
  (void) fprintf(stderr, "[%s + %s]: %s\n", base, operand, P);
  mp_get_memory_functions(NULL, NULL, &gmp_free_mem_func);
  gmp_free_mem_func(P, strlen(P) + 1);

  mpz_clear(S);
  mpz_clear(O);
  mpz_clear(B);

  return 0;
}

int submpz(const char* base, const char* operand) {
  mpz_t B;
  mpz_t O;
  mpz_t S;

  mpz_init2(B, Bits);
  mpz_init2(O, Bits);
  mpz_init2(S, Bits);

  mpz_set_str(B, base, 10);
  mpz_set_str(O, operand, 10);
  mpz_sub(S, B, O);

  char* P = mpz_get_str(NULL, 10, S);
  (void) fprintf(stderr, "[%s - %s]: %s\n", base, operand, P);
  mp_get_memory_functions(NULL, NULL, &gmp_free_mem_func);
  gmp_free_mem_func(P, strlen(P) + 1);

  mpz_clear(S);
  mpz_clear(O);
  mpz_clear(B);

  return 0;
}

int mulmpz(const char* base, const char* operand) {
  mpz_t B;
  mpz_t O;
  mpz_t S;

  mpz_init2(B, Bits);
  mpz_init2(O, Bits);
  mpz_init2(S, Bits);

  mpz_set_str(B, base, 10);
  mpz_set_str(O, operand, 10);
  mpz_mul(S, B, O);

  char* P = mpz_get_str(NULL, 10, S);
  (void) fprintf(stderr, "[%s * %s]: %s\n", base, operand, P);
  mp_get_memory_functions(NULL, NULL, &gmp_free_mem_func);
  gmp_free_mem_func(P, strlen(P) + 1);

  mpz_clear(S);
  mpz_clear(O);
  mpz_clear(B);

  return 0;
}

int divmpz(const char* base, const char* operand,
           char operator) {
  mpz_t B;
  mpz_t O;
  mpz_t R;
  mpz_t Q;

  mpz_init2(B, Bits);
  mpz_init2(O, Bits);
  mpz_init2(R, Bits);
  mpz_init2(Q, Bits);

  mpz_set_str(B, base, 10);
  mpz_set_str(O, operand, 10);
  mpz_fdiv_qr(Q, R, B, O);

  char* RP = mpz_get_str(NULL, 10, R);
  char* QP = mpz_get_str(NULL, 10, Q);
  if (operator == '%')
    (void) fprintf(stderr, "[%s %c %s]: %s\n", base, operator, operand, RP);
  else
    (void) fprintf(stderr, "[%s %c %s]: %s (%s)\n", base, operator, operand,
                   QP, RP);
  mp_get_memory_functions(NULL, NULL, &gmp_free_mem_func);
  gmp_free_mem_func(RP, strlen(RP) + 1);
  gmp_free_mem_func(QP, strlen(QP) + 1);

  mpz_clear(Q);
  mpz_clear(R);
  mpz_clear(O);
  mpz_clear(B);

  return 0;
}

int main(int argc, char* const argv[])
{
  if (argc != 6) {
    print_usage(argv[0]);
    return 1;
  }

  const char* base = NULL;
  const char* operand = NULL;
  char operator = '\0';
  int c;

  while (1) {
    c = getopt_long(argc, argv, "hb:", long_options, &optind);
    if (c == -1)
      break;

    switch (c) {
    case 'b':
      Bits = (uint32_t) strtoul(optarg, NULL, 10);
      break;
    case 'h':
      print_usage(argv[0]);
      return 0;
      break;
    default:
      print_usage(argv[0]);
      return 1;
      break;
    }
  }

  if (optind < argc) {
    base = argv[optind++];
    operator = argv[optind++][0];
    operand = argv[optind];
  }

  if (!base || !operand) {
    print_usage(argv[0]);
    return 1;
  }

  switch (operator) {
  case '+':
    return addmpz(base, operand);
    break;
  case '-':
    return submpz(base, operand);
    break;
  case '*':
    return mulmpz(base, operand);
    break;
  case '/':
  case '%':
    return divmpz(base, operand, operator);
    break;
  default:
    break;
  }

  invalid_operator(argv[0], operator);
  return 1;
}

