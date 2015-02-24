#include "c10mbench.h"
#include "pixie-timer.h"
#include "pixie-threads.h"
#include "rte-ring.h"
#include <stdlib.h>

#define BENCH_ITERATIONS2 ((BENCH_ITERATIONS)*100)

unsigned result = 0;

/******************************************************************************
 ******************************************************************************/
static void
worker_thread(void *parms)
{
    size_t i;
    for (i=0; i<BENCH_ITERATIONS2; i++) {
        pixie_locked_add_u32(&result, 1);
    }
}

/******************************************************************************
 ******************************************************************************/
void
bench_cache_bounce(unsigned cpu_count)
{
    unsigned i;

    for (i=0; i<cpu_count; i += 1) {
        unsigned j;
        double ellapsed;
        double speed;
        size_t thread_handles[256];
        size_t thread_count = 0;
        uint64_t start, stop;
        
        start = pixie_gettime();
        for (j=0; j<=i; j += 1) {
            thread_handles[thread_count++] = pixie_begin_thread(worker_thread, 0, 0);
        }
        for (j=0; j<thread_count; j++)
            pixie_join(thread_handles[j], 0);
        stop = pixie_gettime();


        ellapsed = (stop-start)/1000000.0;
        speed = BENCH_ITERATIONS2*1.0/ellapsed;
        printf("cachebounce, %2u,  %7.3f,  %7.1f\n",
               (unsigned)thread_count,
               speed/1000000.0,
               1000000000.0/speed);
        //printf("verifier: %u = %u\n", result, 2*BENCH_ITERATIONS2);
    }
}
