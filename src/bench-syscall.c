#include "c10mbench.h"
#include "pixie-timer.h"
#include "pixie-threads.h"
#include "rte-ring.h"
#include <stdlib.h>

#ifdef WIN32
#include <process.h>
#define getpid _getpid
#define read _read 
#else
#include <unistd.h>
#endif

#define BENCH_ITERATIONS2 ((BENCH_ITERATIONS)*1)


void
bench_syscall(void)
{
    uint64_t start, stop;
    size_t i;
    uint64_t result = 0;


    start = pixie_gettime();
    for (i=0; i<BENCH_ITERATIONS2; i++) {
#ifdef WIN32
        result += read(0,0,0);
#else
        result += time(0);
#endif
    }
    stop = pixie_gettime();


    {
        double ellapsed = (stop-start)/1000000.0;
        double speed = BENCH_ITERATIONS2*1.0/ellapsed;
        printf("\nbenchmark: syscall rate\n");
        //printf("verifier: %u = %u\n", (unsigned)(result/getpid()), BENCH_ITERATIONS2);
        printf("verifier: none\n");
        printf("rate = %5.2f mega-msgs/sec\n", speed/1000000.0);
        printf("time = %5.3f usec\n", 1000000.0/speed);
    }
}
