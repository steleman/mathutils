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
#include <math.h>

static inline double geomean(double x, unsigned n)
{
  return exp(log(x) / n);
}

int main(int argc, char* argv[])
{
  if (argc < 3) {
    (void) fprintf(stderr, "Usage: geomean <sequence-of-values>\n");
    return 1;
  }

  double s = 1.0;
  unsigned n = 0;

  while (--argc >= 1) {
    s *= strtod(argv[argc], NULL);
    ++n;
  }

  (void) fprintf(stdout, "%lf\n", geomean(s, n));

  return 0;
}

