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

static uint64_t verifier;

/******************************************************************************
 ******************************************************************************/
static void
worker_thread(void *parms)
{
    size_t i;
    unsigned result = 0;
    
    void (*addx)(volatile unsigned *a, unsigned b) = parms;
    
    for (i=0; i<BENCH_ITERATIONS2; i++) {
        addx(&result, 1);
    }
    
    verifier += result;
}


/******************************************************************************
 ******************************************************************************/
void
bench_funcall(unsigned cpu_count, void (*addx)(volatile unsigned *a, unsigned b))
{
    unsigned i;
    
    for (i=0; i<cpu_count; i++) {
        unsigned j;
        double ellapsed;
        double speed;
        size_t thread_handles[256];
        size_t thread_count = 0;
        uint64_t start, stop;
        
        verifier = 0;
        
        start = pixie_gettime();
        for (j=0; j<=i; j++) {
            thread_handles[thread_count++] = pixie_begin_thread(worker_thread, 0, addx);
        }
        for (j=0; j<thread_count; j++)
            pixie_join(thread_handles[j], 0);
        stop = pixie_gettime();
        
        
        ellapsed = (stop-start)/1000000.0;
        speed = BENCH_ITERATIONS2*1.0/ellapsed;
        
        printf("funcall,     %2u,  %7.3f,  %7.1f\n",
               (unsigned)thread_count,
               speed/1000000.0,
               1000000000.0/speed);
    }

}
