# Signature Sort
Project for CS 166

To compile: g++ -std=c++17 -o [COMPILED FILENAME] [FILENAME]


## Remarks:

1.  **packed_sort.cpp**

 *  I pack k b-bit numbers into a 64 bit number. w(64) = 2*k*(b+1).

 *  You can call this function by this: packed_sort(uint64_t a[],  int low, int cnt, int b)

 *  According to the implementation problem, cnt can only be chosen 2^n. For example:
  packed_sort(a, 0, N, 7);

 * radix_sort is also included in the radix_sort.cpp