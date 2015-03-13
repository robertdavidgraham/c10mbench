#include "c10mbench.h"
#include "pixie-timer.h"
#include "pixie-threads.h"
#include "rte-ring.h"
#include <stdlib.h>

#define BENCH_ITERATIONS2 ((BENCH_ITERATIONS)*10)

volatile unsigned result = 0;

void *p;

/******************************************************************************
 ******************************************************************************/
static void
worker_mutex(void *mutex)
{
    size_t i;
    
    for (i=0; i<BENCH_ITERATIONS2; i++) {
        pixie_mutex_lock(mutex);
        result++;
        pixie_mutex_unlock(mutex);
    }
}


/******************************************************************************
 ******************************************************************************/
static void
worker_locked_add(void *parms)
{
    size_t i;
    
    for (i=0; i<BENCH_ITERATIONS2; i++) {
        pixie_locked_add_u32(&result, 1);
    }
}

/******************************************************************************
 ******************************************************************************/
static void
worker_add(void *parms)
{
    size_t i;

    for (i=0; i<BENCH_ITERATIONS2; i++) {
        result++;
    }
}


/******************************************************************************
 ******************************************************************************/
void
bench_cache_bounce(unsigned cpu_count, unsigned which_test)
{
    unsigned i;
    void *mutex;
    
    mutex = pixie_mutex_create();
    

    for (i=0; i<cpu_count; i += 1) {
        unsigned j;
        double ellapsed;
        double speed;
        size_t thread_handles[256];
        size_t thread_count = 0;
        uint64_t start, stop;
        void (*func)(void *);
        const char *test_name;
        
        switch (which_test) {
            case CacheBench_Add:
                func = worker_add;
                test_name = "addnorm";
                break;
            case CacheBench_LockedAdd:
                func = worker_locked_add;
                test_name = "addlocked";
                break;
            case CacheBench_MutexAdd:
                func = worker_mutex;
                test_name = "addmutex";
                break;
            default:
                func = 0;
                test_name = "unknown";
                fprintf(stderr, "cachebounce: unknown test\n");
                exit(1);
                break;
        }
        
        start = pixie_gettime();
        for (j=0; j<=i; j += 1) {
            thread_handles[thread_count++] = pixie_begin_thread(func, 0, mutex);
        }
        for (j=0; j<thread_count; j++)
            pixie_join(thread_handles[j], 0);
        stop = pixie_gettime();


        ellapsed = (stop-start)/1000000.0;
        speed = BENCH_ITERATIONS2*1.0/ellapsed;
        printf("%-12s, %2u,  %7.3f,  %7.3f,  %7.1f\n",
               test_name,
               (unsigned)thread_count,
               speed/1000000.0,
               1.0*thread_count*speed/1000000.0,
               1000000000.0/speed);
        //printf("verifier: %u = %u\n", result, 2*BENCH_ITERATIONS2);
    }
    
    pixie_mutex_destroy(mutex);

}
