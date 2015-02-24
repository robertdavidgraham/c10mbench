#include "c10mbench.h"
#include "pixie-timer.h"
#include "pixie-threads.h"
#include "rte-ring.h"
#include <stdlib.h>
#include <time.h>

#ifdef WIN32
#include <process.h>
#include <io.h>
#define getpid _getpid
#define read _read 
#else
#include <unistd.h>
#endif

#define BENCH_ITERATIONS2 ((BENCH_ITERATIONS)/10)
static uint64_t verifier;

/******************************************************************************
 ******************************************************************************/
static void
worker_thread(void *parms)
{
    size_t i;
    unsigned result = 0;
    
    for (i=0; i<BENCH_ITERATIONS2; i++) {
#ifdef WIN32
        result += read(0,0,0);
#else
        result += time(0);
#endif
    }
    
    verifier += result;
}

/******************************************************************************
 ******************************************************************************/
void
bench_syscall(unsigned cpu_count)
{
    unsigned i;
    
    for (i=0; i<cpu_count; i++) {
        unsigned j;
        double ellapsed;
        double speed;
        size_t thread_handles[256];
        size_t thread_count = 0;
        uint64_t start, stop;
        
        start = pixie_gettime();
        for (j=0; j<=i; j++) {
            thread_handles[thread_count++] = pixie_begin_thread(worker_thread, 0, 0);
        }
        for (j=0; j<thread_handles[j]; j++) {
            pixie_join(thread_handles[j], 0);
            printf("", j);
        }
        stop = pixie_gettime();
        
        
        ellapsed = (stop-start)/1000000.0;
        speed = BENCH_ITERATIONS2*1.0/ellapsed;
        
        printf("syscall,     %2u,  %7.3f,  %7.1f\n",
               (unsigned)thread_count,
               speed/1000000.0,
               1000000000.0/speed);
    }
}
