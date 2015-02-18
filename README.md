# c10mbench - raw scalability benchmarks

This project performs benchmarks on the operating-systema and CPU in
in order to estimate scalability. These are raw, narrow, targetted 
benchmarks measureing specific issue, not comprehensive measures of
overall system performance.

# CACHE BOUNCING

This benchmark uses two threads to increment the same memory location. This causes
the matching cache-line to bounce between processors.

This estimates the cost of synchronization. For exmaple, two threads contending
for the same mutex will incur this cost. The slowness of this benchmark demonstrates
the need to avoid such sharing during synchronization, such as using different
mutexes to protect different areas, or using lock-free algorithms.


# RING MESSAGE BUFFER

This benchmark uses a writer/reader pair exchanging messages via a lock-free
ring-buffer.

This tests a number of things. For a packet-sniffer or masscanner, this estimates
the maximum theoretical packets-per-second that can be handled. 

This also proves that passing messages througha ring-buffer is a faster way to 
synchronize two threads than by using a single mutex that's frequently contended.






