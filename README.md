# Mini Memory Allocator
A simple memory allocator in C that mimics malloc and free, using a fixed-size memory pool.
- Allocate memory blocks of arbitrary size
- Free memory blocks and reuse them
- Block splitting and coalescing
No installation required. Just compile the C file:
gcc -o allocator allocator.c
