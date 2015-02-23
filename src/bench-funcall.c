#include "c10mbench.h"
#include "pixie-timer.h"
#include "pixie-threads.h"
#include "rte-ring.h"
#include <stdlib.h>

#ifdef WIN32
#include <process.h>
#define getpid _getpid
#else
#include <unistd.h>
#endif

#define BENCH_ITERATIONS2 ((BENCH_ITERATIONS)*1000)


void
bench_funcall(void (*addx)(volatile unsigned *a, unsigned b))
{
    unsigned long long start, stop;
    size_t i;
    volatile unsigned result = 0;


    start = pixie_gettime();
    for (i=0; i<BENCH_ITERATIONS2; i++) {
        addx(&result, 1);
    }
    stop = pixie_gettime();


    {
        double ellapsed = (stop-start)/1000000.0;
        double speed = BENCH_ITERATIONS2*1.0/ellapsed;
        printf("\nbenchmark: function call rate\n");
        printf("stop  = %llu\n", stop);
        printf("start = %llu\n", start);
        //printf("verifier: %u = %u\n", (unsigned)(result/getpid()), BENCH_ITERATIONS2);
        printf("verifier: %u\n", result);
        printf("rate = %5.2f mega-msgs/sec\n", speed/1000000.0);
        printf("time = %5.4f usec\n", 1000000.0/speed);
    }
}
