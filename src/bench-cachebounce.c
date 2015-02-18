#include "c10mbench.h"
#include "pixie-timer.h"
#include "pixie-threads.h"
#include "rte-ring.h"
#include <stdlib.h>

#define BENCH_ITERATIONS2 ((BENCH_ITERATIONS)*100)

unsigned result = 0;

static void
reader(void *parms)
{
    size_t i;
    for (i=0; i<BENCH_ITERATIONS2; i++) {
        pixie_locked_add_u32(&result, 1);
    }
}

static void
writer(void *parms)
{
    size_t i;
    for (i=0; i<BENCH_ITERATIONS2; i++) {
        pixie_locked_add_u32(&result, 1);
    }
}


void
bench_cache_bounce(void)
{
    size_t reader_thread, writer_thread;
    uint64_t start, stop;


    start = pixie_gettime();
    reader_thread = pixie_begin_thread(reader, 0, 0);
    writer_thread = pixie_begin_thread(writer, 0, 0);
    pixie_join(reader_thread, 0);
    pixie_join(writer_thread, 0);
    stop = pixie_gettime();


    {
        double ellapsed = (stop-start)/1000000.0;
        double speed = BENCH_ITERATIONS2*1.0/ellapsed;
        printf("\nbenchmark: cache bounce rate\n");
        printf("verifier: %u = %u\n", result, 2*BENCH_ITERATIONS2);
        printf("rate = %5.2f mega-msgs/sec\n", speed/1000000.0);
        printf("time = %5.3f usec\n", 1000000.0/speed);
    }
}
