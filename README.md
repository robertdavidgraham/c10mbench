# c10mbench - raw scalability benchmarks

This project measures raw scalability benchmarks, namely
memory speeds and synchronization speeds. Just type
`make` to compile it, and `make run` to run it.

# Explanation of Benchmarks

## Memory Latency

Memory access is slow. CPUs have small, high-speed caches to store
copies of frequently used day. At scale, there is to much data to 
fit within the cache. Therefore, memory access speed becomes a
limiting factor.

The benchmarks measure latency by randomly accessing memory. Results
vary between about 57 nanosecons and 80 nanoseconds.

## Huge Pages

User-mode uses virtual-memory. Page tables translate virtual addresses
to physical ones. At scale, these pages tables no longer fit in cache.
16-gigabytes worth of data requires 32-megabytes worth of pages tables,
where a cache is around 10-megabytes.

Therefore, every memory access becomes two, doubling memory latency.

One way to mitigate this is through the use of "huge pages" of 
2-megabytes instead of 4-kilobytes. This reduces the page tables
to 64-kilobytes, which can fit with the cache.

This program runs the memory benchmarks twice, once with huge pages
enabled, an once with them disabled.


## Ring Buffer

The ring-buffer is an extremely efficient way to pass messages
between threads. This program benchmarks this. It also benchmarks
the alternative of using pipes, passing messages through the
operating system. It shows that it's 10 to 50 times faster using
the ring-buffer in user-mode using shared-memory than passing
messages through pipes.

## Cache Bouncing

The biggest concern for multicore synchronization is avoiding
data sharing. This benchmark demonstrates this by repeatedly
adding the same memory value from multiple threads.

