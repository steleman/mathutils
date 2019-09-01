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
 * Copyright (C) 2018 Stefan Teleman.
 */

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>
#include <unistd.h>

void print_help(void)
{
  (void) fprintf(stderr, "Usage: ctz -v <value> [-b <bits> (default 64)]\n"
                 "           [-p (print bitmask)]\n");
}

void print_bitmask(uint64_t x, int bits)
{
  (void) fprintf(stderr, "%lu: [", x);

  for (int i = 0; i < bits; ++i) {
    if (x & (1UL << i))
      (void) fprintf(stdout, "1");
    else
      (void) fprintf(stdout, "0");
  }

  (void) fprintf(stdout, "]\n");
}

uint32_t ctz64(uint64_t x)
{
  for (int i = 63; i >= 0; --i) {
    if (x & (1UL << (uint64_t) i))
      return 63 - i;
  }

  return 64;
}

uint32_t ctz32(uint32_t x)
{
  for (int i = 31; i >= 0; --i) {
    if (x & (1U << (uint32_t) i))
      return 31 - i;
  }

  return 32;
}

int main(int argc, char* argv[])
{
  int opt;
  int bits = 64;
  uint64_t x = 0;
  bool pb = false;
  bool ph = false;

  while ((opt = getopt(argc, argv, "pb:v:")) != -1) {
    switch (opt) {
    case 'p':
      pb = true;
      break;
    case 'b':
      bits = (unsigned) strtoul(optarg, NULL, 10);
      break;
    case 'v':
      x = strtoul(optarg, NULL, 10);
      break;
    default:
      ph = true;
      break;
    }
  }

  if (ph || argc < 3) {
    print_help();
    return 1;
  }

  if (pb)
    print_bitmask(x, bits);

  if (bits == 32)
    (void) fprintf(stdout, "%u\n", ctz32((uint32_t) x));
  else
    (void) fprintf(stdout, "%u\n", ctz64(x));

  return 0;
}


