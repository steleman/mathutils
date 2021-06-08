# mathutils
Some simple but practical math utilities

- findprimes uses OpenMP. See the file .ompenv for the OpenMP environment variables.
- added Makefile.aocc and Makefile.icc for the AOCC (AMD) and ICC (Intel) compilers.
- primefactors can only handle 32-bit and 64-bit unsigned integers. Run as:
  ```%> ./primefactors <unsigned-integer>```
- primefactorsmp uses GNU MP (GMP) and can handle unsigned integers of
  arbitrary bit width. Run as:
  ```%> ./primefactorsmp -b <bit-width> <unsigned-integer>```

