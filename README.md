# mathutils
Some simple but practical math utilities

- findprimes no longer uses OpenMP. It now uses POSIX threads, minimum of 4.
- added Makefile.aocc and Makefile.icc for the AOCC (AMD) and ICC (Intel) compilers.
- primefactors can only handle 32-bit and 64-bit unsigned integers. Run as:

  ```%> ./primefactors <unsigned-integer>```
  
- primefactorsmp and findprimesmp use GNU MP (GMP) and can handle unsigned integers of
  arbitrary bit width.
- findprimesmp uses POSIX threads, minimum of 4.
- If you run `findprimesmp -h` or `primefactorsmp -h` it will show you all the command
line options:
  
  ```
  %> ./primefactorsmp -h
  Usage: primefactorsmp -b <number-of-bits> <unsigned integer>
  %> ./findprimesmp -h
  Usage: findprimesmp -s <range-start> (default 18446744073709551615)
                      -e <range-end>
       [ -b <number-of-bits> (default 128)]
       [ -T <number-of-threads> (default 4)]
       [ -f <output-file> (default stdout)]
       [ -p (print header at the top)]
       [ -t (print prime discovery time)]
  ```

