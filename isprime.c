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
 * Copyright (C) 2018, 2019 Stefan Teleman.
 */

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>
#include <math.h>

int main(int argc, char* argv[])
{
  if (argc != 2) {
    (void) fprintf(stderr, "Usage: isprime <unsigned-integer>\n");
    return 1;
  }

  bool NP = false;
  uint64_t X = strtoul(argv[1], NULL, 10);

  if (X == 2UL)
    goto shortcut;

  if (!(X & 1) || (X == 0UL)) {
    NP = true;
    goto shortcut;
  }

  uint64_t S = (uint64_t) ceill(sqrtl(X));

  for (uint64_t I = 3; I <= S; ++I) {
    if ((X % I) == 0) {
      NP = true;
      goto shortcut;
    }
  }

shortcut:
  (void) fprintf(stdout, "%lu is %s.\n",
                 X, (NP ? "not prime" : "prime"));
  return 0;
}

